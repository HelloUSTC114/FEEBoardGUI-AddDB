#include <iostream>

#include "MultiBoard.h"
#include "ui_MultiBoard.h"
#include "feecontrol.h"

#include <QtConcurrent/QtConcurrent>
#include <QDateTime>

void MultiBoardJob::ScanBoards()
{
    for (int boardNo = 0; boardNo < 12; boardNo++)
    {
        gBoard->InitPort(boardNo);
        bool flag = 0;

        // Test connection
        auto future = QtConcurrent::run(gBoard, &FEEControl::TestConnect);
        QDateTime timeNow = QDateTime::currentDateTime();
        while (future.isRunning())
        {
            // std::cout << "Waiting for board " << boardNo << " to connect..." << std::endl;
            // std::cout << timeNow.secsTo(QDateTime::currentDateTime()) << std::endl;
            if (timeNow.msecsTo(QDateTime::currentDateTime()) > 500)
            {
                future.cancel();
                break;
            }
        }
        if (future.isCanceled())
            flag = 0;
        else
            flag = future.result();

        std::cout << "Board No: " << boardNo << "\tSuccess: " << flag << std::endl;
        emit UpdateBoardStatus(boardNo, flag);
    }
    emit BoardsScanned();
}

MultiBoard::MultiBoard(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MultiBoard)
{
    ui->setupUi(this);

    // Create work thread
    fMultiBoardJob = new MultiBoardJob();
    fMultiBoardJob->moveToThread(&fMultiBoardThread);
    connect(&fMultiBoardThread, &QThread::finished, fMultiBoardJob, &QObject::deleteLater);
    fMultiBoardThread.start();

    // Connect signals and slots
    connect(this, &MultiBoard::StartScanBoards, fMultiBoardJob, &MultiBoardJob::ScanBoards);
    connect(fMultiBoardJob, &MultiBoardJob::UpdateBoardStatus, this, &MultiBoard::handle_BoardStatus);
    connect(fMultiBoardJob, &MultiBoardJob::BoardsScanned, this, &MultiBoard::handle_BoardsScanned);
}

MultiBoard::~MultiBoard()
{
    delete ui;
    fMultiBoardThread.quit();
    fMultiBoardThread.wait();

    if (fMultiBoardJob)
    {
        delete fMultiBoardJob;
        fMultiBoardJob = nullptr;
    }
}

void MultiBoard::ScanBoards()
{
    ClearLists();
    emit StartScanBoards();
    ui->btnScanBoards->setEnabled(false);
}

void MultiBoard::UpdateLists()
{
    ui->listBoards->clear();
    for (auto &board : fBoardStatus)
    {
        QString status = board.second ? "Connected" : "Not connected";
        ui->listBoards->addItem(QString("Board No: %1\tStatus: %2").arg(board.first).arg(status));
    }
}

void MultiBoard::ClearLists()
{
    ui->listBoards->clear();
    fBoardStatus.clear();
}

void MultiBoard::handle_BoardStatus(int boardNo, bool status)
{
    fBoardStatus[boardNo] = status;
    UpdateLists();
}

void MultiBoard::handle_BoardsScanned()
{
    ui->btnScanBoards->setEnabled(true);
    UpdateLists();
}

void MultiBoard::on_btnScanBoards_clicked()
{
    ScanBoards();
}

#include "feecontrol.h"
#include "datamanager.h"

BoardConnection::BoardConnection()
{
    fBoard = new FEEControl();
    fDataManager = new DataManager();
    fParser = new ConfigFileParser();
    fBoardNo = -1;

    fSingleBoardJob = new SingleBoardJob(this);
    fSingleBoardJob->moveToThread(&fSingleBoardThread);
    QObject::connect(&fSingleBoardThread, &QThread::finished, fSingleBoardJob, &QObject::deleteLater);
    fSingleBoardThread.start();

    QObject::connect(this, &BoardConnection::StartDAQ, fSingleBoardJob, &SingleBoardJob::handle_StartDAQ);
    QObject::connect(fSingleBoardJob, &SingleBoardJob::DAQFinished, this, &BoardConnection::handle_DAQFinished);

    QObject::connect(&fBoardMonitorTimer, &QTimer::timeout, this, &BoardConnection::RetrieveCount);

    QObject::connect(&fMonitorTimer, &QTimer::timeout, this, &BoardConnection::MonitorDAQ);
}

BoardConnection::~BoardConnection()
{
    if (fBoard)
    {
        delete fBoard;
        fBoard = nullptr;
    }
    if (fDataManager)
    {
        delete fDataManager;
        fDataManager = nullptr;
    }
    if (fParser)
    {
        delete fParser;
        fParser = nullptr;
    }
    if (fout.is_open())
    {
        fout.close();
    }
    fBoardNo = -1;

    fSingleBoardThread.quit();
    fSingleBoardThread.wait();
    if (fSingleBoardJob)
    {
        delete fSingleBoardJob;
        fSingleBoardJob = nullptr;
    }

    fBoardMonitorTimer.stop();
    fMonitorTimer.stop();
    fDAQRuningFlag = 0;
    fConnectionStatus = 0;
}

bool BoardConnection::InitBoard(int boardNo)
{
    fBoardNo = boardNo;
    fBoard->InitPort(boardNo);

    if (fBoard->TestConnect())
        fConnectionStatus = true;
    else
        fConnectionStatus = false;

    if (fConnectionStatus)
        ProcessConnection();
    else
        ProcessDisconnection();
    return fConnectionStatus;
}

void BoardConnection::ProcessConnection()
{
    SetHV(57);
    SetBoardConfig();
    fBoardMonitorTimer.start(1000);
    fConnectionStatus = true;
}

void BoardConnection::ProcessDisconnection()
{
    fDataManager->Close();
    StopDAQ();
    fDAQRuningFlag = 0;
    fBoardMonitorTimer.stop();
    fConnectionStatus = false;
    fout.close();
    fMonitorTimer.stop();
}

bool BoardConnection::SetHV(double hv)
{
    auto rtn = fBoard->HVSet(hv);
    return rtn > 0;
}

#include "configfileparser.h"
bool BoardConnection::SetBoardConfig(std::string sRepoFolder)
{
    fBoard->logic_select(3);
    fBoard->send_ch_masks(0xffffffff);
    fBoard->HVSet(57.0);

    if (fBoardNo % 2 == 0)
    {
        fParser->Init(sRepoFolder + "Configuration/CRTest/TopLayer/sc_register_pd.txt", sRepoFolder + "Configuration/CRTest/TopLayer/probe_register.txt");
    }
    else
    {
        fParser->Init(sRepoFolder + "Configuration/CRTest/BottomLayer/sc_register_pd.txt", sRepoFolder + "Configuration/CRTest/BottomLayer/probe_register.txt");
    }
    fBoard->SendConfig(fParser);

    return true;
}

bool BoardConnection::InitDataFile(std::string sDataFolder)
{
    fsFilePath = sDataFolder;
    fsFileName = "Board" + std::to_string(fBoardNo);
    auto fFileTimeStamp = QDateTime::currentDateTime();
    auto sFileNameTotal = fsFileName + fFileTimeStamp.toString("-yyyy-MM-dd-hh-mm-ss").toStdString() + ".root";
    fDataManager->Init(fsFilePath + sFileNameTotal);

    fDataManager->SetBoardNo(fBoardNo);
    fDataManager->SetCITIROCConfig(fParser->GetString());
    fDataManager->SetDAQDatime(fFileTimeStamp);
    fDataManager->SetSelectedLogic(fBoard->GetLastLogic());

    fBoard->ReadTemp();
    double temp[4];
    fBoard->GetTemp(temp);
    fDataManager->SetDAQTemp(temp);

    fout.open(fsFilePath + "Board" + std::to_string(fBoardNo) + "TS.txt", std::ios::app);

    return true;
}

#include <QThread>
bool BoardConnection::StartDAQ()
{
    auto rtn = fBoard->TestConnect();
    if (!rtn)
        return false;
    InitDataFile();

    fDAQRuningFlag = true;
    fBoard->HVON();
    fMonitorTimer.start(1000);
    QThread::msleep(1000);

    emit RequestStartDAQ();

    return true;
}

bool BoardConnection::StopDAQ()
{
    if (fDAQRuningFlag)
    {
        fDAQRuningFlag = 0;
        fBoard->SetFifoReadBreak();
    }
    return true;
}

void BoardConnection::handle_DAQFinished(int nDAQLoop)
{
    fDAQRuningFlag = 0;
    fMonitorTimer.stop();

    fDataManager->Close();
    fout.close();
    fBoard->HVOFF();
}

bool BoardConnection::MonitorDAQ()
{
    ReadTimeStamp();
    UpdateDAQInfo();

    return true;
}

bool BoardConnection::ReadTimeStamp()
{
    uint64_t timeStampTemp[5];
    int T0IDdev = fBoard->ReadTimeStamp(timeStampTemp, fCurrentT0ID);
    auto flag = T0IDdev > 0;

    for (int i = 0; i < T0IDdev; i++)
    {
        fTimeStampArray[i] = DataManager::ConvertTDC2Time(timeStampTemp[i]);
    }

    // The loop aims at exchange number restore inside
    for (int j = 0; j < 5 - T0IDdev; j++)
    {
        int idxPre = 4 - j - T0IDdev;
        int idxPost = 4 - j;

        fTimeStampArrayStore[idxPost] = fTimeStampArray[idxPre];
    }

    // Restore data
    for (int i = 0; i < T0IDdev; i++)
    {
        fTimeStampArrayStore[i] = fTimeStampArray[i];
    }

    // output into file
    std::ofstream fout;
    std::string outFileName = (std::string) "Board" + std::to_string(fBoardNo) + "TS.txt";
    for (int i = 0; i < T0IDdev; i++)
    {
        char out_char[100];
        sprintf(out_char, "%1.3f", fTimeStampArray[T0IDdev - 1 - i]);
        fout << fCurrentT0ID << '\t' << T0IDdev << '\t' << out_char << std::endl;
    }
    fout.close();

    fPreviousT0ID = fCurrentT0ID;

    return flag;
}

bool BoardConnection::UpdateDAQInfo()
{
    // Update Transist count rate
    int nCount = fDataManager->GetHGTotalCount();
    transCount = nCount - sLastCount;
    // if ((sTransTimeInteval = sLastTime.msecsTo(QDateTime::currentDateTime())) > 100 && transCount > 0)
    if ((sTransTimeInteval = sLastTime.msecsTo(QDateTime::currentDateTime())) >= 1)
    {
        // std::cout << "Count: " << nCount << '\t' << transCount << '\t' << sTransTimeInteval << std::endl;
        sTransCR = (double)transCount / sTransTimeInteval * 1000;
        sLastCount = nCount;
        sTransTimeInteval = 0;
        sLastTime = QDateTime::currentDateTime();
    }

    // Update real count and time monitor first
    fDAQRealCount = nCount;
    fDAQRealTime = fDAQStartTime.msecsTo(QDateTime::currentDateTime());

    auto sCount = QString::number(nCount);

    double countRate = nCount / (double)fDAQRealTime * 1000; // Count rate in unit 1/s

    double percentCount = nCount / (double)fDAQSettingCount * 100;
    double percentTime = fDAQRealTime / (double)fDAQSettingTime.msecsSinceStartOfDay() * 100;

    if (fDAQSettingCount < 0)
    {
        if (fDAQSettingTime == QTime(0, 0, 0, 0))
            // ui->pbarDAQ->setValue(0);
            ;
        else
            // ui->pbarDAQ->setValue(percentTime > 100 ? 100 : percentTime);
            ;
    }
    else
    {
        if (fDAQSettingTime == QTime(0, 0, 0, 0))
            // ui->pbarDAQ->setValue(percentCount > 100 ? 100 : percentCount);
            ;
        else
            // ui->pbarDAQ->setValue(percentCount > percentTime ? percentCount : percentTime);
            ;
    }
    return true;
}

bool BoardConnection::RetrieveCount()
{
    // Update real count and live count
    sLastTestTime = QDateTime::currentDateTime();
    sLastLiveCount = -1;
    sLastRealCount = -1;
    testTime = QDateTime::currentDateTime();
    msToLastTest = (double)sLastTestTime.msecsTo(testTime);

    realCount = 0;
    liveCount = 0;
    fBoard->get_real_counter(realCount);
    fBoard->get_live_counter(liveCount);

    // ui->lineRealCount->setText(QString::number(realCount));
    // ui->lineLiveCount->setText(QString::number(liveCount));
    if (sLastRealCount > 0)
    {
        // ui->lineRealCR->setText(QString::number((realCount - sLastRealCount) * 1000.0 / msToLastTest));
    }
    if (sLastLiveCount > 0)
    {
        // ui->lineLiveCR->setText(QString::number((liveCount - sLastLiveCount) * 1000.0 / msToLastTest));
    }
    sLastRealCount = realCount;
    sLastLiveCount = liveCount;
    sLastTestTime = testTime;
    return false;
}

SingleBoardJob::SingleBoardJob(BoardConnection *conn) : fConnection(conn), fBoard(fConnection->GetBoard()), fDataManager(fConnection->GetDataManager())
{
}

void SingleBoardJob::handle_StartDAQ()
{
    int nDAQLoop = 0;
    int nDAQEventCount = 0;
    fConnection->fDAQStartTime = QDateTime::currentDateTime();

    // Clear queue before DAQ Start
    if (fConnection->fFlagClearQueue)
    {
        fBoard->clean_queue(0);
        fBoard->clean_queue(1);
        fBoard->clean_queue(2);
        fBoard->clean_queue(3);
    }

    bool loopFlag = JudgeLoopFlag(0);
    for (nDAQLoop = 0; loopFlag; nDAQLoop++)
    {
        auto rtnRead = (fBoard)->ReadFifo(fConnection->fDAQBufferSleepms, fConnection->fDAQBufferLeastEvents);
        if (!rtnRead)
            break;
        nDAQEventCount += fDataManager->ProcessFEEData((fBoard));
        loopFlag = JudgeLoopFlag(nDAQEventCount);
    }
    emit DAQFinished(nDAQLoop);
}

bool SingleBoardJob::JudgeLoopFlag(int nEventCount)
{
    bool timeFlag = (fConnection->fDAQSettingTime == QTime(0, 0, 0, 0)) || (fConnection->fDAQStartTime.addMSecs(fConnection->fDAQSettingTime.msecsSinceStartOfDay()) >= QDateTime::currentDateTime());

    bool nEventFlag = (fConnection->fDAQSettingCount < 0 || nEventCount < fConnection->fDAQSettingCount);
    bool connectionFlag = fConnection->IsConnected();
    bool loopFlag = fConnection->fDAQRuningFlag && nEventFlag && timeFlag && connectionFlag;
    return loopFlag;
}
