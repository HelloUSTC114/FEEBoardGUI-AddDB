#include <QFileDialog>
#include <QDebug>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>

#include "ui_DBWindow.h"
#include "DBWindow.h"
#include "FEEControlWidget.h"
#include "DBManager.h"

#include <iostream>
DBWindow::DBWindow(QWidget *parent) : QMainWindow(parent),
                                      ui(new Ui::DBWindow)
{
    ui->setupUi(this);
    //    setWindowFlag()
    std::cout << "DEBUG JOHN--------------------" << std::endl;

    sipmRes = new SiPMTestResult();
    feeRes = new BoardTestResult();

    // Build Channels info
    auto lbl0 = new QLabel(ui->grpChs);
    lbl0->setText("channels");
    ui->gridChs->addWidget(lbl0, 0, 0, Qt::AlignCenter);
    // ui->gridChs->addWidget(lbl0, 0, 0, 1, 1);

    auto lbl1 = new QLabel(ui->grpChs);
    lbl1->setText("Amp Choosing");
    ui->gridChs->addWidget(lbl1, 0, 1, Qt::AlignCenter);

    auto lbl2 = new QLabel(ui->grpChs);
    lbl2->setText("Amp Value");
    ui->gridChs->addWidget(lbl2, 0, 2, Qt::AlignCenter);

    auto lbl3 = new QLabel(ui->grpChs);
    lbl3->setText("Bias Choosing");
    ui->gridChs->addWidget(lbl3, 0, 3, Qt::AlignCenter);

    auto lbl4 = new QLabel(ui->grpChs);
    lbl4->setText("Bias Value");
    ui->gridChs->addWidget(lbl4, 0, 4, Qt::AlignCenter);

    auto lbl5 = new QLabel(ui->grpChs);
    lbl5->setText("SiPM T Factor");
    ui->gridChs->addWidget(lbl5, 0, 5, Qt::AlignCenter);

    auto lbl6 = new QLabel(ui->grpChs);
    lbl6->setText("SiPM BD");
    ui->gridChs->addWidget(lbl6, 0, 6, Qt::AlignCenter);

    auto lbl7 = new QLabel(ui->grpChs);
    lbl7->setText("SiPM OV");
    ui->gridChs->addWidget(lbl7, 0, 7, Qt::AlignCenter);

    for (int ch = 0; ch < 32; ch++)
    {
        auto lblCh = new QLabel(ui->grpChs);
        lblCh->setText(QString::number(ch));
        ui->gridChs->addWidget(lblCh, ch + 1, 0, Qt::AlignCenter);

        fcombAmp[ch] = new QSpinBox(ui->grpChs);
        ui->gridChs->addWidget(fcombAmp[ch], ch + 1, 1, Qt::AlignCenter);
        fcombAmp[ch]->setRange(0, 63);
        connect(fcombAmp[ch], SIGNAL(valueChanged(int)), this, SLOT(on_btnShow_clicked()));

        flblAmp[ch] = new QLabel(ui->grpChs);
        flblAmp[ch]->setText(QString::number(0));
        ui->gridChs->addWidget(flblAmp[ch], ch + 1, 2, Qt::AlignCenter);

        fcombBias[ch] = new QSpinBox(ui->grpChs);
        ui->gridChs->addWidget(fcombBias[ch], ch + 1, 3, Qt::AlignCenter);
        fcombBias[ch]->setRange(0, 255);
        connect(fcombBias[ch], SIGNAL(valueChanged(int)), this, SLOT(on_btnShow_clicked()));

        flblBias[ch] = new QLabel(ui->grpChs);
        flblBias[ch]->setText(QString::number(0));
        ui->gridChs->addWidget(flblBias[ch], ch + 1, 4, Qt::AlignCenter);

        flblSiPMTCom[ch] = new QLabel(ui->grpChs);
        flblSiPMTCom[ch]->setText(QString::number(0));
        ui->gridChs->addWidget(flblSiPMTCom[ch], ch + 1, 5, Qt::AlignCenter);

        flblSiPMBD[ch] = new QLabel(ui->grpChs);
        flblSiPMBD[ch]->setText(QString::number(0));
        ui->gridChs->addWidget(flblSiPMBD[ch], ch + 1, 6, Qt::AlignCenter);

        flblSiPMOV[ch] = new QLabel(ui->grpChs);
        flblSiPMOV[ch]->setText(QString::number(0));
        ui->gridChs->addWidget(flblSiPMOV[ch], ch + 1, 7, Qt::AlignCenter);
    }
}

void DBWindow::GetBoardListFromDB()
{
    ClearList();
    gDBManager->ReadFromDB(fsFileName);
    auto feeList = gDBManager->GetFEEBoardLists();
    for (auto iter = feeList.begin(); iter != feeList.end(); iter++)
    {
        vFEEBoardList.push_back(*iter);
        ui->listFEE->addItem(ConvertToString(*iter));
    }
    auto sipmList = gDBManager->GetSiPMBoardLists();
    int counter = 0;
    for (auto iter = sipmList.begin(); iter != sipmList.end(); iter++)
    {
        SiPMBoardInfo temp(iter->first, iter->second);
        vSiPMBoardList.push_back(temp);
        ui->listSiPM->addItem(ConvertToString(temp));
    }
}

void DBWindow::ClearList()
{
    vFEEBoardList.clear();
    vSiPMBoardList.clear();
    vPairedInfo.clear();
    ui->listFEE->clear();
    ui->listSiPM->clear();
    ui->listPair->clear();
    ui->cbbPair->clear();
}

void DBWindow::UpdateListUI()
{
    ui->listFEE->clear();
    ui->listSiPM->clear();
    ui->listPair->clear();
    ui->cbbPair->clear();
    for (auto iter = vFEEBoardList.begin(); iter != vFEEBoardList.end(); iter++)
    {
        ui->listFEE->addItem(ConvertToString(*iter));
    }
    for (auto iter = vSiPMBoardList.begin(); iter != vSiPMBoardList.end(); iter++)
    {
        ui->listSiPM->addItem(ConvertToString(*iter));
    }
    for (auto iter = vPairedInfo.begin(); iter != vPairedInfo.end(); iter++)
    {
        ui->listPair->addItem(ConvertToString(*iter));
        ui->cbbPair->addItem(ConvertToString(*iter));
    }
}

DBWindow::~DBWindow()
{
    delete sipmRes;
    delete feeRes;
    delete ui;
}

DBWindow *DBWindow::Instance()
{
    static auto ins = new DBWindow(gFEEControlWin);
    return ins;
}

std::vector<std::pair<int, int>> DBWindow::GetCurrentCompBias(double temp0, double temp1, double temp2, double temp3)
{
    // TODO: insert return statement here
    ui->lineTemp0->setText(QString::number(temp0));
    ui->lineTemp1->setText(QString::number(temp1));
    ui->lineTemp2->setText(QString::number(temp2));
    ui->lineTemp3->setText(QString::number(temp3));

    GetAllCompBias(temp0, temp1, temp2, temp3);
    std::vector<std::pair<int, int>> rtnArray;
    for (int ch = 0; ch < 32; ch++)
    {
        rtnArray.push_back(std::pair<int, int>(ch, fCurrentBias[ch]));
    }
    return rtnArray;
}

std::vector<std::pair<int, int>> DBWindow::GetCompBias(int feeBoardNo, double temp0, double temp1, double temp2, double temp3)
{
    std::vector<std::pair<int, int>> rtnResult;
    for (int ch = 0; ch < 32; ch++)
        rtnResult.push_back(std::pair<int, int>(ch, fDefaultBias));
    if (!gDBManager->IsInitiated())
        return rtnResult;

    int counter = 0;
    for (auto iter = vPairedInfo.begin(); iter != vPairedInfo.end(); iter++, counter++)
    {
        if (iter->first == feeBoardNo)
        {
            ui->cbbPair->setCurrentIndex(counter);
            on_btnShow_clicked();
            break;
        }
    }
    if (counter == vPairedInfo.size())
        return rtnResult;

    return GetCurrentCompBias(temp0, temp1, temp2, temp3);
}

bool DBWindow::IsValid()
{
    return gDBManager->IsInitiated();
}

void DBWindow::on_btnDBFile_clicked()
{
    QString temp;
    if (fsFileName != "")
        temp = QFileDialog::getOpenFileName(this, "Choose Database", fsFileName, "*.db");
    else
        temp = QFileDialog::getOpenFileName(this, "Choose Database", "F:\\Projects\\FEEDistri\\DataBase\\", "*.db");

    if (temp == "")
    {
        fFileNameIsInput = 0;
        return;
    }

    fsFileName = temp;
    fFileNameIsInput = 1;
    ui->lineDBPath->setText(fsFileName);
}

void DBWindow::on_btnOpenDB_clicked()
{
    if (!gDBManager->OpenDB(fsFileName))
    {
        ui->lblDBLED->setStyleSheet("background-color:rgb(255,0,0)");
        return;
    }
    ui->lblDBLED->setStyleSheet("background-color:rgb(0,255,0)");
    ui->lineDBPath->setText(fsFileName);
    GetBoardListFromDB();
}

void DBWindow::on_btnCloseDB_clicked()
{
    gDBManager->CloseDB();
    ui->lblDBLED->setStyleSheet("background-color:rgb(190,190,190)");
}

void DBWindow::Group2Boards(const int &fee, const SiPMBoardInfo &sipm)
{
    vPairedInfo.push_back(std::pair<int, SiPMBoardInfo>(fee, sipm));
    // // vPairedInfo.sort();
    // vPairedInfo.sort([](const std::pair<int, SiPMBoardInfo> &a, const std::pair<int, SiPMBoardInfo> &b)
    //                  { return a < b; });
    vFEEBoardList.remove(fee);
    vSiPMBoardList.remove(sipm);
    UpdateListUI();
}

void DBWindow::Ungroup2Boards(const std::pair<int, SiPMBoardInfo> &pair)
{
    vPairedInfo.remove_if([pair](std::pair<int, SiPMBoardInfo> iter)
                          {
        if((iter.first == pair.first)&&(iter.second==pair.second))return true;else return false; });

    vFEEBoardList.push_back(pair.first);
    vSiPMBoardList.push_back(pair.second);
    vFEEBoardList.sort();
    vSiPMBoardList.sort();
    UpdateListUI();
}

QString DBWindow::ConvertToString(std::pair<int, SiPMBoardInfo> pair)
{
    return ConvertToString(pair.first) + "," + ConvertToString(pair.second);
}

QString DBWindow::ConvertToString(SiPMBoardInfo boardinfo)
{
    QString sBT;
    if (boardinfo.fBT == bottom)
        sBT = "b";
    else
        sBT = "t";
    return QString("SiPM-%1:%2").arg(sBT).arg(boardinfo.fBoardNo);
}

QString DBWindow::ConvertToString(int feeboard)
{
    return QString("FEE:%1").arg(feeboard);
}

std::pair<int, SiPMBoardInfo> DBWindow::ConvertStringToPair(QString sPair)
{
    auto list = sPair.split(',');
    int feeboard = ConvertStringToFEE(list[0]);
    auto sipmboard = ConvertStringToSiPM(list[1]);
    std::pair<int, SiPMBoardInfo> pair(feeboard, sipmboard);
    return pair;
}

SiPMBoardInfo DBWindow::ConvertStringToSiPM(QString sSiPM)
{
    auto list1 = sSiPM.split(':');
    auto sBoardNo = list1[1];
    int boardNo = sBoardNo.toInt();

    auto list2 = list1[0].split('-');
    auto sBoardType = list2[1];
    SIPMBOARDTYPE type;
    if (sBoardType == 'b')
        type = bottom;
    else
        type = top;
    return SiPMBoardInfo(boardNo, type);
}

int DBWindow::ConvertStringToFEE(QString sFEE)
{
    auto list = sFEE.split(':');
    auto sBoardNo = list[1];
    return sBoardNo.toInt();
}

void DBWindow::on_btnBind_clicked()
{
    if (ui->listFEE->currentRow() < 0 || ui->listSiPM->currentRow() < 0)
        return;
    auto s1 = ui->listFEE->currentItem()->text();
    auto s2 = ui->listSiPM->currentItem()->text();
    Group2Boards(ConvertStringToFEE(s1), ConvertStringToSiPM(s2));
    return;
}

void DBWindow::on_btnUnbind_clicked()
{
    if (ui->listPair->currentRow() < 0)
        return;
    auto s = ui->listPair->currentItem()->text();
    Ungroup2Boards(ConvertStringToPair(s));
    return;
}

void DBWindow::ReadChanInfo(int ch)
{
    if (!fIsRead)
        return;
    if (!feeRes->IsValid() || !sipmRes->IsValid())
        return;
    int amp = fcombAmp[ch]->value();
    int bias = fcombBias[ch]->value();
    double ampCali = feeRes->GetCaliResult(ch, amp, hg);
    double biasValue = feeRes->GetBias(ch, bias);
    double sipmTCom = sipmRes->GetRealTCompFactor(ch);
    double sipmBD = sipmRes->GetRealBDVoltageV(ch);
    double sipmOV = fCurrentHV - biasValue - sipmBD;

    fCurrentBias[ch] = bias;
    fCurrentTComp[ch] = sipmTCom;
    fCurrentBD[ch] = sipmBD;
    // qDebug() << ch << '\t' << bias << '\t' << sipmTCom << '\t' << sipmBD;

    flblAmp[ch]->setText(QString::number(ampCali));
    flblBias[ch]->setText(QString::number(biasValue));
    flblSiPMTCom[ch]->setText(QString::number(sipmTCom));
    flblSiPMBD[ch]->setText(QString::number(sipmBD));
    flblSiPMOV[ch]->setText(QString::number(sipmOV));
}

#include <fstream>
#include <sstream>
#include <iostream>
SiPMBoardInfo ParseSiPMBoardName(std::string sBoardName)
{
    SiPMBoardInfo rtn;
    int idx = 0;
    if ((idx = sBoardName.find("board")) != std::string::npos)
    {
        rtn.fBT = bottom;
        rtn.fBoardNo = std::stoi(sBoardName.substr(idx + 5));
        return rtn;
    }
    else if ((idx = sBoardName.find("top")) != std::string::npos)
    {
        rtn.fBT = top;
        rtn.fBoardNo = std::stoi(sBoardName.substr(idx + 3));
        return rtn;
    }
    return rtn;
}
bool DBWindow::ReadTCompFile(std::string sInputFile)
{
    std::ifstream fin(sInputFile);
    if (!fin.is_open())
    {
        qDebug() << "Error: Cannot open file: " << QString::fromStdString(sInputFile);
        return false;
    }
    std::string sLine;
    std::getline(fin, sLine);

    QString qsTemp;
    int lineCount = 0;
    for (lineCount = 0; fin.good() && !fin.eof() && fin.is_open(); lineCount++)
    {
        std::getline(fin, sLine);
        qsTemp = QString::fromStdString(sLine);
        std::string sBoardName;
        int ch;
        double Vslope, Vbd, Tslope, Tfactor;

        auto list = qsTemp.split(",");
        if (list.size() < 6)
            break;
        sBoardName = list[0].toStdString();
        ch = list[1].toInt();
        Vslope = list[2].toDouble();
        Vbd = list[3].toDouble();
        Tslope = list[4].toDouble();
        Tfactor = list[6].toDouble();
        // qDebug() << list.size() << '\t' << Tfactor;
        // ss >> sBoardName >> c >> ch >> c >> Vslope >> c >> Vbd >> c >> Tslope >> c >> Tfactor;
        // qDebug() << QString::fromStdString(sBoardName) << '\t' << ch << '\t' << Vslope << '\t' << Vbd << '\t' << Tslope << '\t' << Tfactor;
        if (fin.eof() || !fin.good())
            break;
        auto sipminfo = ParseSiPMBoardName(sBoardName);
        fTCompMap[sipminfo][ch] = Tfactor;
        fBDMap[sipminfo][ch] = Vbd;
    }
    fin.close();
    if (lineCount < 31)
        return false;
    return true;
}

void DBWindow::ProcessTComp()
{
    if (!fOtherFileFlag)
        return;
    auto iter = fTCompMap.find(fCurrentPair.second);
    if (iter == fTCompMap.end())
    {
        fCurrentTComp = fTCompMap.begin()->second;
        fCurrentBD = fBDMap.begin()->second;
    }
    else
    {
        fCurrentTComp = iter->second;
        fCurrentBD = fBDMap.find(fCurrentPair.second)->second;
    }

    for (int ch = 0; ch < 32; ch++)
    {
        int bias = fcombBias[ch]->value();
        double biasValue = feeRes->GetBias(ch, bias);
        double sipmTCom = fCurrentTComp[ch];
        double sipmBD = fCurrentBD[ch];
        double sipmOV = fCurrentHV - biasValue - sipmBD;

        // qDebug() << sipmBD << '\t' << fCurrentBD[ch] << '\t' << sipmTCom << '\t' << fCurrentTComp[ch];
        flblSiPMTCom[ch]->setText(QString::number(sipmTCom));
        flblSiPMBD[ch]->setText(QString::number(sipmBD));
        flblSiPMOV[ch]->setText(QString::number(sipmOV));
    }
}

int DBWindow::GetCompBias(double Temp, int ch)
{
    double tslope = fCurrentTComp[ch];
    if (tslope < 10 || tslope > 100)
    {
        qDebug() << "Error: TSlope: " << tslope << "mV/C";
        qDebug() << "Setting at default compensation factor: 52mV/C";
        tslope = 52;
        // return fDefaultBias;
    }
    double tdevi = (Temp - 25);
    double biasDevi = tdevi * tslope / 1000.0;
    double voltage0 = feeRes->GetBias(ch, fDefaultBias);

    int biasRes = fDefaultBias;
    if (biasDevi < 0)
        for (int bias = fDefaultBias; bias >= 0; bias--)
        {
            double voltage = feeRes->GetBias(ch, bias);
            // qDebug() << ch << '\t' << Temp << '\t' << bias << '\t' << (voltage - voltage0) * 1000 << '\t' << biasDevi;
            if (voltage0 - voltage < biasDevi)
            {
                biasRes = bias;
                break;
            }
        }
    else
        for (int bias = fDefaultBias; bias < 256; bias++)
        {
            double voltage = feeRes->GetBias(ch, bias);
            // qDebug() << ch << '\t' << Temp << '\t' << bias << '\t' << (voltage - voltage0) * 1000 << '\t' << biasDevi;
            if (voltage0 - voltage > biasDevi)
            {
                biasRes = bias;
                break;
            }
        }
    qDebug() << Temp << '\t' << ch << '\t' << biasRes;
    return biasRes;
}

std::array<double, 32> &DBWindow::GetAllCompBias(const std::array<double, 32> &temp)
{
    // TODO: insert return statement here
    for (int ch = 0; ch < 32; ch++)
    {
        double Temp = temp[ch];
        fCurrentBias[ch] = GetCompBias(Temp, ch);
        fcombBias[ch]->setValue((int)fCurrentBias[ch]);
    }

    return fCurrentBias;
}

std::array<double, 32> &DBWindow::GetAllCompBias(double temp0, double temp1, double temp2, double temp3)
{
    // TODO: insert return statement here
    std::array<double, 32> temp;
    for (int ch = 0; ch < 8; ch++)
    {
        temp[ch] = temp0;
        temp[ch + 8] = temp1;
        temp[ch + 16] = temp2;
        temp[ch + 24] = temp3;
    }
    return GetAllCompBias(temp);
}

bool operator==(const SiPMBoardInfo &a, const SiPMBoardInfo &b)
{
    return (a.fBoardNo == b.fBoardNo) && (a.fBT == b.fBT);
}

bool operator<(const SiPMBoardInfo &a, const SiPMBoardInfo &b)
{
    if (a.fBoardNo < b.fBoardNo)
        return true;
    if (a.fBoardNo > b.fBoardNo)
        return false;
    if (a.fBT < b.fBT)
        return true;
    return false;
}

void DBWindow::on_btnShow_clicked()
{
    // qDebug() << ui->cbbPair->currentText();
    fCurrentPair = ConvertStringToPair(ui->cbbPair->currentText());
    // qDebug() << fCurrentPair.first << '\t' << fCurrentPair.second.fBoardNo << '\t' << fCurrentPair.second.fBT;
    feeRes->ReadFromDB(fCurrentPair.first);
    sipmRes->ReadFromDB(fCurrentPair.second.fBoardNo, fCurrentPair.second.fBT);
    fIsRead = 1;

    for (int ch = 0; ch < 32; ch++)
    {
        ReadChanInfo(ch);
    }
    ProcessTComp();
}

void DBWindow::on_btnNewTComp_clicked()
{
    static QString qsCurrent = "";
    if (qsCurrent == "")
        qsCurrent = "E:\\Data\\~Cali~SiPMTestAnalyze\\SiPMTFactor.csv";
    qsCurrent = QFileDialog::getOpenFileName(this, "Choose Another SiPM Compensation File", qsCurrent, "*.csv");
    ui->lineNewTComp->setText(qsCurrent);

    auto rtn = ReadTCompFile(qsCurrent.toStdString());

    if (rtn)
    {
        fOtherFileFlag = 1;
        ui->btnNewTComp->setEnabled(false);
        ui->btnCloseTComp->setEnabled(true);
    }
    else
    {
        fOtherFileFlag = 0;
        ui->btnNewTComp->setEnabled(true);
        ui->btnCloseTComp->setEnabled(false);
    }
    ProcessTComp();
}

void DBWindow::on_btnCloseTComp_clicked()
{
    fOtherFileFlag = 0;
    ui->btnNewTComp->setEnabled(true);
    ui->btnCloseTComp->setEnabled(false);
    on_btnShow_clicked();
}

#include "FEEControlWidget.h"
void DBWindow::on_btnTempComp_clicked()
{
    double temp0 = ui->lineTemp0->text().toDouble();
    double temp1 = ui->lineTemp1->text().toDouble();
    double temp2 = ui->lineTemp2->text().toDouble();
    double temp3 = ui->lineTemp3->text().toDouble();
    gFEEControlWin->Modify_SP_CITIROC_BiasDAC(GetCompBias(gFEEControlWin->GetBoardNo(), temp0, temp1, temp2, temp3));
}
