#include <QFileDialog>
#include <QDebug>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>

#include "ui_DBWindow.h"
#include "DBWindow.h"
#include "FEEControlWidget.h"
#include "DBManager.h"

DBWindow::DBWindow(QWidget *parent) : QMainWindow(parent),
                                      ui(new Ui::DBWindow)
{
    ui->setupUi(this);

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
    gDBManager->ReadFromDB();
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

void DBWindow::on_btnDBFile_clicked()
{
    QString temp;
    if (fsFileName != "")
        temp = QFileDialog::getOpenFileName(this, "Choose Database", fsFileName, "*.db");
    else
        temp = QFileDialog::getOpenFileName(this, "Choose Database", "F:\\Projects\\FEEDistri\\DataBase\\", "*.db");

    if (temp == "")
        return;

    fsFileName = temp;
    fFileNameIsInput = 1;
    ui->lineDBPath->setText(fsFileName);
}

void DBWindow::on_btnOpenDB_clicked()
{
    if (!gDBManager->OpenDB(fsFileName))
        ui->lblDBLED->setStyleSheet("background-color:rgb(255,0,0)");
    ui->lblDBLED->setStyleSheet("background-color:rgb(0,255,0)");
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
    double sipmOV = 56 - biasValue - sipmBD;

    flblAmp[ch]->setText(QString::number(ampCali));
    flblBias[ch]->setText(QString::number(biasValue));
    flblSiPMTCom[ch]->setText(QString::number(sipmTCom));
    flblSiPMBD[ch]->setText(QString::number(sipmBD));
    flblSiPMOV[ch]->setText(QString::number(sipmOV));
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
    auto pair = ConvertStringToPair(ui->cbbPair->currentText());
    feeRes->ReadFromDB(pair.first);
    sipmRes->ReadFromDB(pair.second.fBoardNo, pair.second.fBT);
    fIsRead = 1;

    for (int ch = 0; ch < 32; ch++)
    {
        ReadChanInfo(ch);
    }
}
