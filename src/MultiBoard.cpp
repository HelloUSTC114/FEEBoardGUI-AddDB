#include <iostream>

#include "MultiBoard.h"
#include "ui_MultiBoard.h"
#include "feecontrol.h"

#include <QtConcurrent/QtConcurrent>
#include <QDateTime>
const std::vector<int> gBoardScanList = {0, 1, 2, 3, 4, 5, 6, 7};

void MultiBoardJob::ScanBoards()
{
    for (auto boardNo : gBoardScanList)
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

    // ui settings
    for (int idx = 0; idx < gBoardScanList.size(); idx++)
    {
        auto boardNo = gBoardScanList[idx];
        flblBoardNo[boardNo] = new QLabel(ui->grpStatus);
        flblBoardNo[boardNo]->setText(QString::number(boardNo));
        ui->gridStatus->addWidget(flblBoardNo[boardNo], idx, 0, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});

        flblConnectionStatus[boardNo] = new QLabel(ui->grpStatus);
        flblConnectionStatus[boardNo]->setText("Not connected");
        flblConnectionStatus[boardNo]->setStyleSheet("QLabel { background-color : red; color : black; }");
        ui->gridStatus->addWidget(flblConnectionStatus[boardNo], idx, 1, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});

        flblRealCount[boardNo] = new QLabel(ui->grpStatus);
        flblRealCount[boardNo]->setText("0");
        ui->gridStatus->addWidget(flblRealCount[boardNo], idx, 2, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});

        flblLiveCount[boardNo] = new QLabel(ui->grpStatus);
        flblLiveCount[boardNo]->setText("0");
        ui->gridStatus->addWidget(flblLiveCount[boardNo], idx, 3, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});

        flblT0TS[boardNo] = new QLabel(ui->grpStatus);
        flblT0TS[boardNo]->setText("0");
        ui->gridStatus->addWidget(flblT0TS[boardNo], idx, 4, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});

        flblDAQCount[boardNo] = new QLabel(ui->grpStatus);
        flblDAQCount[boardNo]->setText("0");
        ui->gridStatus->addWidget(flblDAQCount[boardNo], idx, 5, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});

        flblHGCount[boardNo] = new QLabel(ui->grpStatus);
        flblHGCount[boardNo]->setText("0");
        ui->gridStatus->addWidget(flblHGCount[boardNo], idx, 6, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});

        flblLGCount[boardNo] = new QLabel(ui->grpStatus);
        flblLGCount[boardNo]->setText("0");
        ui->gridStatus->addWidget(flblLGCount[boardNo], idx, 7, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});

        flblTDCCount[boardNo] = new QLabel(ui->grpStatus);
        flblTDCCount[boardNo]->setText("0");
        ui->gridStatus->addWidget(flblTDCCount[boardNo], idx, 8, 1, 1, {Qt::AlignCenter, Qt::AlignCenter});
    }

    ui->grpDAQStart->setEnabled(false);
    ui->btnDAQStop->setEnabled(false);
    ui->listBoards->setSelectionMode(QAbstractItemView::SingleSelection);

    // Create work thread
    fMultiBoardJob = new MultiBoardJob();
    fMultiBoardJob->moveToThread(&fMultiBoardThread);
    connect(&fMultiBoardThread, &QThread::finished, fMultiBoardJob, &QObject::deleteLater);
    fMultiBoardThread.start();

    // Connect signals and slots
    connect(this, &MultiBoard::StartScanBoards, fMultiBoardJob, &MultiBoardJob::ScanBoards);
    connect(fMultiBoardJob, &MultiBoardJob::UpdateBoardStatus, this, &MultiBoard::handle_BoardStatus);
    connect(fMultiBoardJob, &MultiBoardJob::BoardsScanned, this, &MultiBoard::handle_BoardsScanned);

    connect(&fBoardStatusTimer, &QTimer::timeout, this, &MultiBoard::UpdateStatus);
    fBoardStatusTimer.start(1000);

    // Process Board Connection
    for (auto boardNo : gBoardScanList)
    {
        auto boardConnection = new BoardConnection();
        fBoardConnections[boardNo] = boardConnection;
    }
}

MultiBoard::~MultiBoard()
{
    fMultiBoardThread.quit();
    fMultiBoardThread.wait();

    fBoardStatusTimer.stop();

    delete ui;

    for (auto &board : fBoardConnections)
    {
        delete board.second;
    }
}

void MultiBoard::ScanBoards()
{
    ClearLists();
    emit StartScanBoards();
    ui->btnScanBoards->setEnabled(false);
}

#include <QLabel>
void MultiBoard::UpdateLists()
{
    ui->listBoards->clear();
    for (auto &board : fBoardStatus)
    {
        QString status = board.second ? "Connected" : "Not connected";
        // ui->listBoards->addItem(QString("Board No: %1\tStatus: %2").arg(board.first).arg(status));
        auto label = new QLabel(QString("Board No:\t%1\tStatus:%2").arg(board.first).arg(status));
        if (board.second)
            label->setStyleSheet("QLabel { background-color : green; color : black; }");
        else
            label->setStyleSheet("QLabel { background-color : red; color : black; }");
        auto item = new QListWidgetItem();
        ui->listBoards->addItem(item);
        ui->listBoards->setItemWidget(item, label);
    }
}

void MultiBoard::ClearLists()
{
    ui->listBoards->clear();
    fBoardStatus.clear();
}

void MultiBoard::ProcessConnection()
{
    for (auto &board : fBoardStatus)
    {
        if (board.second)
            if (!fBoardConnections[board.first]->IsInitialized())
                fBoardConnections[board.first]->InitBoard(board.first);
    }
}

void MultiBoard::UpdateStatus()
{
    // Update connection status from fBoardConnections
    bool updateStatusFlag = 0;
    for (auto &board : fBoardStatus)
    {
        if (board.second != fBoardConnections[board.first]->IsConnected())
        {
            updateStatusFlag = 1;
            board.second = fBoardConnections[board.first]->IsConnected();
        }
    }
    if (updateStatusFlag)
        UpdateLists();

    // DAQ status, update status inside fBoardConnections
    for (auto &board : fBoardConnections)
    {
        if (updateStatusFlag)
            if (board.second->IsConnected())
            {
                flblConnectionStatus[board.first]->setText("Connected");
                flblConnectionStatus[board.first]->setStyleSheet("QLabel { background-color : green; color : black; }");
            }
            else
            {
                flblConnectionStatus[board.first]->setText("Not connected");
                flblConnectionStatus[board.first]->setStyleSheet("QLabel { background-color : red; color : black; }");
            }

        flblRealCount[board.first]->setText(QString("%1/%2").arg(board.second->GetRealCount()).arg(board.second->GetRealCR()));
        flblLiveCount[board.first]->setText(QString("%1/%2").arg(board.second->GetLiveCount()).arg(board.second->GetLiveCR()));
        flblT0TS[board.first]->setText(QString("%1").arg(board.second->GetTimeStampArrayStore()[0]));

        if (board.second->GetDAQRuningFlag())
        {

            flblDAQCount[board.first]->setText(QString("%1/%2").arg(board.second->GetDAQRealCount()).arg(board.second->GetDAQRealCR()));
            flblHGCount[board.first]->setText(QString("%1").arg(board.second->GetHGCount()));
            flblLGCount[board.first]->setText(QString("%1").arg(board.second->GetLGCount()));
            flblTDCCount[board.first]->setText(QString("%1").arg(board.second->GetTDCCount()));
        }
    }
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
    ProcessConnection();

    int nConnected = 0;
    int masterBoard = -1;
    std::for_each(fBoardStatus.begin(), fBoardStatus.end(), [&nConnected, &masterBoard](auto &board)
                  { nConnected += board.second; if(masterBoard<0)masterBoard = board.first; });

    if (nConnected > 0)
    {
        ui->grpDAQStart->setEnabled(true);
        ui->btnDAQStart->setEnabled(true);
        SetMasterBoard(masterBoard);
    }
    else
    {
        ui->grpDAQStart->setEnabled(false);
        ui->btnDAQStart->setEnabled(false);
    }
}

void MultiBoard::on_btnScanBoards_clicked()
{
    ScanBoards();
}

bool MultiBoard::SetMasterBoard(int board)
{
    if (std::find(gBoardScanList.begin(), gBoardScanList.end(), board) == gBoardScanList.end())
        return false;
    bool rtn = fBoardConnections[board]->SetMasterBoard(1);
    flblBoardNo[board]->setStyleSheet("QLabel { background-color : blue; color : black; }");
    for (auto &board : fBoardConnections)
        if (board.second->IsConnected())
            board.second->SetMasterBoard(0);
    return rtn;
}

bool MultiBoard::EnableTDC(int board)
{
    if (std::find(gBoardScanList.begin(), gBoardScanList.end(), board) == gBoardScanList.end())
        return false;
    return fBoardConnections[board]->EnableTDC(1);
}

bool MultiBoard::EnableAllTDC()
{
    std::vector<QFuture<bool>> futures;

    for (auto &board : fBoardConnections)
        if (board.second->IsConnected())
        {
            auto future = QtConcurrent::run(board.second, &BoardConnection::EnableTDC, 1);
            futures.push_back(std::move(future));
        }
    for (auto &future : futures)
        future.waitForFinished();
    return true;
}

bool MultiBoard::DisableAllTDC()
{
    std::vector<QFuture<bool>> futures;

    for (auto &board : fBoardConnections)
        if (board.second->IsConnected())
        {
            auto future = QtConcurrent::run(board.second, &BoardConnection::EnableTDC, 0);
            futures.push_back(std::move(future));
        }
    for (auto &future : futures)
        future.waitForFinished();
    return true;
}

void MultiBoard::on_btnDAQStart_clicked()
{
    ui->btnScanBoards->setEnabled(false);
    ui->btnDAQStart->setEnabled(false);
    ui->btnDAQStop->setEnabled(true);

    auto fileName = ui->lblFileName->text();
    if (fileName == "")
        fsFileName = "Board";
    else
        fsFileName = fileName.toStdString();
    ui->lblFileName->setText(QString::fromStdString(fsFileName));

    for (auto &board : fBoardConnections)
    {
        if (board.second->IsConnected())
        {
            board.second->SetDAQConfig(ui->boxDAQEvent->value(), ui->timeDAQSetting->time(), ui->boxBufferWait->value(), ui->boxLeastEvents->value(), ui->boxClearQueue->isChecked());
            board.second->SetFilePathName(fsFilePath, fsFileName + std::to_string(board.first));
        }
    }

    std::vector<QFuture<bool>> futures;
    for (auto &board : fBoardConnections)
        if (board.second->IsConnected())
        {
            auto future = QtConcurrent::run(board.second, &BoardConnection::StartDAQ);
            futures.push_back(std::move(future));
        }
    for (auto &future : futures)
        future.waitForFinished();

    // Process the result
    bool flagSuccess = 1;
    for (auto &future : futures)
        if (!future.result())
            flagSuccess = 0;
    if (flagSuccess)
    {
        EnableAllTDC();
        std::cout << "DAQ started successfully!" << std::endl;
    }
    else
    {
        std::cout << "DAQ started failed!" << std::endl;
        on_btnDAQStop_clicked();
    }
}

void MultiBoard::on_btnDAQStop_clicked()
{
    ui->btnScanBoards->setEnabled(true);
    ui->btnDAQStart->setEnabled(true);
    ui->btnDAQStop->setEnabled(false);

    std::vector<QFuture<bool>> futures;

    for (auto &board : fBoardConnections)
    {
        if (board.second->IsConnected())
        {
            auto future = QtConcurrent::run(board.second, &BoardConnection::StopDAQ);
            futures.push_back(std::move(future));
        }
    }
    for (auto &future : futures)
        future.waitForFinished();

    DisableAllTDC();
}

#include <QFileDialog>
void MultiBoard::on_btnPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select Directory", "../Data/");
    if (!path.isEmpty())
        fsFilePath = path.toStdString() + "/";
    else if (fsFilePath == "")
        fsFilePath = "../Data/";

    ui->linePath->setText(QString::fromStdString(fsFilePath));
}

#include "feecontrol.h"
#include "datamanager.h"
#include "configfileparser.h"

BoardConnection::BoardConnection()
{
    fBoard = new FEEControl();
    fDataManager = new DataManager();
    fParser = new ConfigFileParser();
    fBoardNo = -1;

    fSingleBoardJob = new SingleBoardJob(this);
    fSingleBoardJob->moveToThread(&fSingleBoardThread);
    connect(&fSingleBoardThread, &QThread::finished, fSingleBoardJob, &QObject::deleteLater);
    fSingleBoardThread.start();

    connect(this, &BoardConnection::RequestStartDAQ, fSingleBoardJob, &SingleBoardJob::handle_StartDAQ);
    connect(fSingleBoardJob, &SingleBoardJob::DAQFinished, this, &BoardConnection::handle_DAQFinished);
    connect(fSingleBoardJob, &SingleBoardJob::DAQDisconnected, this, &BoardConnection::handle_DAQDisconnected);

    connect(&fBoardMonitorTimer, &QTimer::timeout, this, &BoardConnection::RetrieveCount);

    connect(&fMonitorTimer, &QTimer::timeout, this, &BoardConnection::MonitorDAQ);
}

void BoardConnection::handle_DAQDisconnected()
{
    ProcessDisconnection();
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

    fBoardMonitorTimer.stop();
    fMonitorTimer.stop();
    fDAQRuningFlag = 0;
    fConnectionStatus = 0;
}

bool BoardConnection::InitBoard(int boardNo)
{
    if (fInitilized)
        return false;

    fBoardNo = boardNo;
    fBoard->InitPort(boardNo);
    fInitilized = true;

    if (fBoard->TestConnect())
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

bool BoardConnection::CloseBoard()
{
    fBoardNo = -1;
    fInitilized = false;

    ProcessDisconnection();
    return true;
}

void BoardConnection::ProcessDisconnection()
{
    fDataManager->Close();
    fDAQRuningFlag = 0;
    fBoardMonitorTimer.stop();
    fConnectionStatus = false;
    fout.close();
    fMonitorTimer.stop();
}

bool BoardConnection::SetMasterBoard(bool bMaster)
{
    if (fConnectionStatus == 0)
        return false;
    return fBoard->write_reg_test(55, bMaster);
}

bool BoardConnection::SetHV(double hv)
{
    if (fConnectionStatus == 0)
        return false;
    auto rtn = fBoard->HVSet(hv);
    return rtn > 0;
}

bool BoardConnection::EnableTDC(bool bEnable)
{
    if (fConnectionStatus == 0)
        return false;
    return fBoard->enable_tdc(bEnable);
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

bool BoardConnection::InitDataFile()
{
    if (fsFilePath == "")
        fsFilePath = "../Data/";
    if (fsFileName == "")
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

void BoardConnection::SetDAQConfig(int nCount, QTime time, int nBufferSleepms, int nBufferLeastEvents, bool bClearQueue)
{
    fDAQSettingCount = nCount;
    fDAQSettingTime = time;
    fDAQBufferSleepms = nBufferSleepms;
    fDAQBufferLeastEvents = nBufferLeastEvents;
    fFlagClearQueue = bClearQueue;
}

void BoardConnection::SetFilePathName(std::string sFilePath, std::string sFileName)
{
    fsFilePath = sFilePath;
    fsFileName = sFileName;
    if (fsFilePath == "")
        fsFilePath = "../Data/";
    if (fsFileName == "")
        fsFileName = "Board" + std::to_string(fBoardNo);
}

double BoardConnection::GetHGCount()
{
    if (fDataManager)
        return fDataManager->GetHGTotalCount();
    return 0;
}

double BoardConnection::GetLGCount()
{
    if (fDataManager)
        return fDataManager->GetLGTotalCount();
    return 0;
}

double BoardConnection::GetTDCCount()
{
    if (fDataManager)
        return fDataManager->GetTDCTotalCount();
    return 0;
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

    fDAQRealCR = nCount / (double)fDAQRealTime * 1000; // Count rate in unit 1/s

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
    testTime = QDateTime::currentDateTime();
    msToLastTest = (double)sLastTestTime.msecsTo(testTime);

    fBoard->get_real_counter(realCount);
    fBoard->get_live_counter(liveCount);

    // ui->lineRealCount->setText(QString::number(realCount));
    // ui->lineLiveCount->setText(QString::number(liveCount));
    if (sLastRealCount > 0)
    {
        // ui->lineRealCR->setText(QString::number((realCount - sLastRealCount) * 1000.0 / msToLastTest));
        sRealCR = (realCount - sLastRealCount) * 1000.0 / msToLastTest;
    }
    if (sLastLiveCount > 0)
    {
        // ui->lineLiveCR->setText(QString::number((liveCount - sLastLiveCount) * 1000.0 / msToLastTest));
        sLiveCR = (liveCount - sLastLiveCount) * 1000.0 / msToLastTest;
    }
    sLastRealCount = realCount;
    sLastLiveCount = liveCount;
    sLastTestTime = testTime;
    return false;
}

SingleBoardJob::SingleBoardJob(BoardConnection *conn)
{
    fConnection = conn;
    fBoard = fConnection->GetBoard();
    fDataManager = fConnection->GetDataManager();
}

void SingleBoardJob::handle_StartDAQ()
{
    int nDAQLoop = 0;
    int nDAQEventCount = 0;
    QThread::msleep(1000); // Wait for 1s to make sure the board is ready (for the first time only
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
    bool rtnRead = 1;
    for (nDAQLoop = 0; loopFlag; nDAQLoop++)
    {
        rtnRead = (fBoard)->ReadFifo(fConnection->fDAQBufferSleepms, fConnection->fDAQBufferLeastEvents);
        if (!rtnRead)
        {
            emit DAQDisconnected();
            break;
        }
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

void MultiBoard::on_btnMaster_clicked()
{
    auto board = ui->listBoards->selectedItems().at(0)->text().split("\t")[1].toInt();
    SetMasterBoard(board);
}
