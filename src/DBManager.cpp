#include <QDebug>

#include "DBManager.h"
#include <iostream>

DBManager *DBManager::Instance()
{
    static auto instance = new DBManager();
    return instance;
}

bool DBManager::OpenDB(QString sDBName)
{
    if (fInitiated)
        CloseDB();
    Init(sDBName);
    return fInitiated;
}

void DBManager::CloseDB()
{
    if (fInitiated)
    {
        fDataBase.close();
        fInitiated = 0;
    }
}

DBManager::DBManager()
{
}

DBManager::~DBManager()
{
    CloseDB();
}

void DBManager::Init(QString sDBName)
{
    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        fDataBase = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        fDataBase = QSqlDatabase::addDatabase("QSQLITE");
        fDataBase.setDatabaseName(sDBName);
        // fDataBase.setUserName("John");
        // fDataBase.setPassword("123456");
    }

    if (fDataBase.open())
    {
        qInfo() << "Successfully open Database: " << sDBName;
        fInitiated = 1;
    }
    else
    {
        qInfo() << "Falied to open Database: " << sDBName;
        fInitiated = 0;
    }
}

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <TCanvas.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TFile.h>

/// @brief Process amp test csv file
/// @param sFileName [IN]
/// @param vampSet [OUT] row of ampSet
/// @param vchvalue [OUT] value[ch][ampset]
/// @param NRow [OUT] how many rows
/// @return
bool ProcessAmpTestFile(std::string sFileName, double *vampSet, double **vchvalue, int &NRow)
{
    std::ifstream fin;
    fin.open(sFileName);
    if (!fin.is_open())
        return false;

    std::string sLine;
    std::stringstream ss;

    std::getline(fin, sLine);
    std::getline(fin, sLine);
    // for (; !fin.eof() && fin.good() && fin.is_open();)
    int rowCount = 0;
    for (; !fin.eof() && fin.good() && fin.is_open();)
    {
        std::getline(fin, sLine);
        ss.clear();
        ss.str(sLine);
        int ampSet;
        double value;
        double MultiplyFactor;
        char c;

        // Judge whether the first character is ','
        ss >> c;
        if (c == ',')
            break;
        ss.clear();
        ss.str(sLine);

        ss >> ampSet;
        // std::cout << ampSet << std::endl;
        // std::cout << sLine << std::endl;
        ss >> c;
        if (c != ',')
        {
            std::cout << "Error." << std::endl;
            break;
        }

        ss >> MultiplyFactor;
        ss >> c;
        if (c != ',')
        {
            std::cout << "Error." << std::endl;
            break;
        }
        for (int ch = 0; ch < 32; ch++)
        {
            ss >> value;
            ss >> c;
            if (c != ',')
            {
                std::cout << "Error." << std::endl;
                break;
            }
            // tg[ch]->SetPoint(pointCounter[ch]++, ampSet, value / MultiplyFactor);
            vchvalue[ch][rowCount] = value;
        }
        vampSet[rowCount] = ampSet;
        rowCount++;
    }
    NRow = rowCount;
    fin.close();

    return true;
}

double BoardTestResult::CalcAmpMultiFactor(int ampDAC, GAINTYPE hl)
{
    double lgFactor = 60.0 / (63 - ampDAC);
    double hgFactor = lgFactor * 10;
    // std::cout << lgFactor << '\t' << hgFactor << std::endl;
    return (hl == hg) ? (hgFactor) : (lgFactor);
}

bool BoardTestResult::ReadAmpCaliFile(int board, GAINTYPE hl)
{
    // Open Data file
    std::string sPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\~FEE~AmpCali\\";
    std::string sFileName = "board" + std::to_string(board) + "AMPCALI";
    std::string sHL;
    if (hl == hg)
        sHL = "hg";
    else
        sHL = "lg";
    sFileName += sHL + ".csv";
    std::ifstream fin(sPath + sFileName);
    // std::cout << "File Is Open: " << sPath + sFileName << '\t' << fin.is_open() << std::endl;
    // if (!fin.is_open())
    //     return false;

    // Create write root file
    std::string sWName = "AmpCali-board" + std::to_string(board) + "-" + sHL + ".root";
    auto file = new TFile(sWName.c_str(), "recreate");
    TGraph *tg[32];
    TMultiGraph mg;
    int pointCounter[32];
    mg.SetTitle(Form("board%d-%s;Amp Set DAC;#Delta_{V}/A", board, sHL.c_str()));
    for (int ch = 0; ch < 32; ch++)
    {
        tg[ch] = new TGraph();
        tg[ch]->SetTitle(Form("board%d-%s-ch%d;Amp Set DAC;#Delta_{V}/A", board, sHL.c_str(), ch));
        pointCounter[ch] = 0;
        mg.Add(tg[ch]);
    }

    double ampSet[50];
    double *vAmpValue[32];
    for (int ch = 0; ch < 32; ch++)
    {
        vAmpValue[ch] = new double[50];
    }
    int NRow;

    ProcessAmpTestFile(sPath + sFileName, ampSet, vAmpValue, NRow);

    for (int row = 0; row < NRow; row++)
    {
        for (int ch = 0; ch < 32; ch++)
        {

            tg[ch]->SetPoint(row, ampSet[row], vAmpValue[ch][row] / BoardTestResult::CalcAmpMultiFactor(ampSet[row], hl));
        }
    }

    // std::string sLine;
    // std::stringstream ss;

    // std::getline(fin, sLine);
    // std::getline(fin, sLine);
    // // for (; !fin.eof() && fin.good() && fin.is_open();)
    // for (; !fin.eof() && fin.good() && fin.is_open();)
    // {
    //     std::getline(fin, sLine);
    //     ss.clear();
    //     ss.str(sLine);
    //     int ampSet;
    //     double amp;
    //     double MultiplyFactor;
    //     char c;

    //     // Judge whether the first character is ','
    //     ss >> c;
    //     if (c == ',')
    //         break;
    //     ss.clear();
    //     ss.str(sLine);

    //     ss >> ampSet;
    //     std::cout << ampSet << std::endl;
    //     // std::cout << sLine << std::endl;
    //     ss >> c;
    //     if (c != ',')
    //         std::cout << "Error." << std::endl;
    //     ss >> MultiplyFactor;
    //     ss >> c;
    //     if (c != ',')
    //         std::cout << "Error." << std::endl;
    //     for (int ch = 0; ch < 32; ch++)
    //     {
    //         ss >> amp;
    //         ss >> c;
    //         if (c != ',')
    //             std::cout << "Error." << std::endl;
    //         tg[ch]->SetPoint(pointCounter[ch]++, ampSet, amp / MultiplyFactor);
    //     }
    // }
    // fin.close();

    file->cd();
    mg.SetName(Form("mgAMP%d_%s", board, sHL.c_str()));
    mg.Write(Form("mgAMP%d_%s", board, sHL.c_str()));
    for (int ch = 0; ch < 32; ch++)
    {
        tg[ch]->SetName(Form("tgAMP%d_%s_ch%d", board, sHL.c_str(), ch));
        // tg[ch]->Write(Form("tgAMP%d_%s_ch%d", board, sHL.c_str(), ch));
        // delete tg[ch];
        tg[ch] = NULL;
    }
    delete file;
    for (int ch = 0; ch < 32; ch++)
    {
        delete[] vAmpValue[ch];
    }

    return true;
}

bool BoardTestResult::ReadPedMeanFile(int board, GAINTYPE hl)
{
    // Open Data file
    std::string sPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\~FEE~PedTest\\";
    std::string sFileName = "board" + std::to_string(board) + "PedCALI";
    std::string sHL;
    if (hl == hg)
        sHL = "hg";
    else
        sHL = "lg";
    sFileName += sHL + "_mean.csv";
    std::ifstream fin(sPath + sFileName);
    std::cout << "Opening file: " << sPath + sFileName << '\t' << fin.is_open() << std::endl;
    if (!fin.is_open())
        return false;

    // Create write root file
    std::string sWName = "FEEPed-board" + std::to_string(board) + "-" + sHL + ".root";
    auto file = new TFile(sWName.c_str(), "recreate");
    TGraph *tg[32];
    TMultiGraph mg;
    int pointCounter[32];
    mg.SetTitle(Form("board%d-%s;Amp Set DAC;Pedestal value", board, sHL.c_str()));

    for (int ch = 0; ch < 32; ch++)
    {
        tg[ch] = new TGraph();
        tg[ch]->SetTitle(Form("board%d-%s-ch%d;Amp Set DAC;Pedestal value", board, sHL.c_str(), ch));
        mg.Add(tg[ch]);
    }

    // Read from source file
    double vAmpSet[50];
    double *vPedMean[32];
    int NRow;
    for (int ch = 0; ch < 32; ch++)
    {
        vPedMean[ch] = new double[50];
    }
    ProcessAmpTestFile(sPath + sFileName, vAmpSet, vPedMean, NRow);

    for (int row = 0; row < NRow; row++)
    {
        for (int ch = 0; ch < 32; ch++)
        {
            tg[ch]->SetPoint(row, vAmpSet[row], vPedMean[ch][row]);
        }
    }

    file->cd();
    mg.SetName(Form("mgPED%d_%s", board, sHL.c_str()));
    mg.Write(Form("mgPED%d_%s", board, sHL.c_str()));

    for (int ch = 0; ch < 32; ch++)
    {
        tg[ch]->SetName(Form("tgPED%d_%s_ch%d", board, sHL.c_str(), ch));
        delete[] vPedMean[ch];
    }

    delete file;
    return true;
}

bool ReadBoard0BiasDACTestFile(double *vBiasSet, double **vchValue, int &NRow)
{
    std::ifstream fin;
    fin.open("E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\~FEE~BiasDACTest\\DACV_test_num_0\\board0_inputdac.txt");
    if (!fin.is_open())
        return false;

    std::stringstream ss;
    int rowCount = 0;
    for (; fin.is_open() && fin.good() && !fin.eof();)
    {
        int biasSet;
        double value;
        char c;

        std::string sLine;
        std::getline(fin, sLine);
        ss.clear();
        ss.str(sLine);

        // std::cout << sLine << std::endl;
        // std::cout << "ss.good: " << ss.good() << std::endl;
        ss >> biasSet;
        // std::cout << biasSet << '\t' << ss.good() << std::endl;
        if (!ss.good() && !fin.eof())
        {
            std::cout << "Error." << std::endl;
            NRow = 0;
            return false;
        }
        if (!ss.good() && fin.eof())
        {
            break;
        }
        vBiasSet[rowCount] = biasSet;
        // std::cout << biasSet << std::endl;

        for (int ch = 0; ch < 32; ch++)
        {
            ss >> value;
            if (!ss.good() && !ss.eof() && (ss.eof() && ch < 31))
            {
                std::cout << "Error." << ch << std::endl;
                NRow = 0;
                return false;
            }
            vchValue[ch][rowCount] = value;
        }
        rowCount++;
    }
    NRow = rowCount;
    fin.close();

    return true;
}

// Constrain in value and bias set array order: [255,1,0], descending from 255 to 0, step is 1
bool ReadBiasTestDACFileOtherBoard(std::string sPath, int ch, double *value)
{
    std::string sFileName = "Channel-" + std::to_string(ch) + ".txt";
    std::ifstream fin;
    fin.open(sPath + sFileName);
    std::cout << "Opening: " << sPath + sFileName << '\t' << fin.is_open() << std::endl;
    if (!fin.is_open())
        return false;

    std::stringstream ss;
    int rowCount = 0;
    for (; fin.is_open() && fin.good() && !fin.eof();)
    {
        std::string sLine;
        int biasSet;
        double valueTemp[100];

        std::getline(fin, sLine);
        ss.clear();
        ss.str(sLine);

        ss >> biasSet;
        int testCount;
        ss >> testCount;
        double sum = 0;
        if (!ss.good() && fin.eof())
            break;
        for (int count = 0; count < testCount; count++)
        {
            ss >> valueTemp[count];
            // std::cout << valueTemp[count] << std::endl;
            sum += valueTemp[count];
        }
        // std::cout << biasSet << std::endl;
        value[255 - biasSet] = sum / testCount;
    }
    fin.close();
    return true;
}

bool BoardTestResult::ReadBiasDACTestFile(int board)
{
    // Open Data file
    std::string sPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\~FEE~BiasDACTest\\DACV_test_num_";
    sPath += std::to_string(board) + "\\";

    // Create write root file
    std::string sWName = "FEEBias-board" + std::to_string(board) + ".root";
    auto file = new TFile(sWName.c_str(), "recreate");
    TGraph *tg[32];
    TMultiGraph mg;
    mg.SetTitle(Form("board%d;Amp Set DAC;Pedestal value", board));
    for (int ch = 0; ch < 32; ch++)
    {
        tg[ch] = new TGraph();
        tg[ch]->SetTitle(Form("board%d;Amp Set DAC;Pedestal value", board));
        mg.Add(tg[ch]);
    }

    int NRow;
    double biasSet[256];
    double *vValue[32];
    for (int ch = 0; ch < 32; ch++)
    {
        vValue[ch] = new double[256];
    }

    if (board == 0)
        ReadBoard0BiasDACTestFile(biasSet, vValue, NRow);
    else
    {
        for (int row = 0; row < 256; row++)
        {
            biasSet[row] = 255 - row;
        }
        for (int ch = 0; ch < 32; ch++)
        {
            ReadBiasTestDACFileOtherBoard(sPath, ch, vValue[ch]);
            NRow = 256;
        }
    }

    for (int ch = 0; ch < 32; ch++)
    {
        for (int row = 0; row < NRow; row++)
        {
            tg[ch]->SetPoint(row, biasSet[row], vValue[ch][row]);
        }
    }

    file->cd();
    mg.SetName(Form("mgBias%d", board));
    mg.Write(Form("mgBias%d", board));

    for (int ch = 0; ch < 32; ch++)
    {
        delete[] vValue[ch];
    }
    delete file;

    return true;
}

void BoardTestResult::GenerateAMPCali()
{
    std::cout << "Generating: AMP cali " << std::endl;
    fsCaliPath = fsDepoPath + "\\~FEE~AmpCali\\";
    std::string sPath = fsCaliPath;
    std::string sFileNameHG = "board" + std::to_string(fBoardNo) + "AMPCALIhg.csv";
    std::string sFileNameLG = "board" + std::to_string(fBoardNo) + "AMPCALIlg.csv";
    std::string sHL;

    double ampSet[50];
    double *vAmpValue[32];
    for (int ch = 0; ch < 32; ch++)
    {
        vAmpValue[ch] = new double[50];
    }
    int NRow;

    // Process HG
    ProcessAmpTestFile(sPath + sFileNameHG, ampSet, vAmpValue, NRow);
    for (int row = 0; row < NRow; row++)
    {
        fAMPTable[ampSet[row]].ampSet = ampSet[row];
        for (int ch = 0; ch < 32; ch++)
        {
            fAMPTable[ampSet[row]].hgCali[ch] = vAmpValue[ch][row];
        }
    }

    // Process LG
    ProcessAmpTestFile(sPath + sFileNameLG, ampSet, vAmpValue, NRow);
    for (int row = 0; row < NRow; row++)
    {
        fAMPTable[ampSet[row]].ampSet = ampSet[row];
        for (int ch = 0; ch < 32; ch++)
        {
            fAMPTable[ampSet[row]].lgCali[ch] = vAmpValue[ch][row];
        }
    }

    for (int ch = 0; ch < 32; ch++)
    {
        delete[] vAmpValue[ch];
    }
}

void BoardTestResult::GeneratePed()
{
    std::string sPath = fsDepoPath + "\\~FEE~PedTest\\";
    std::string sFileNameHG = "board" + std::to_string(fBoardNo) + "PedCALIhg_mean.csv";
    std::string sFileNameLG = "board" + std::to_string(fBoardNo) + "PedCALIlg_mean.csv";
    std::cout << "Generating: Pedestal " << std::endl;

    double ampSet[50];
    double *vAmpValue[32];
    for (int ch = 0; ch < 32; ch++)
    {
        vAmpValue[ch] = new double[50];
    }
    int NRow;

    // Process HG
    ProcessAmpTestFile(sPath + sFileNameHG, ampSet, vAmpValue, NRow);
    for (int row = 0; row < NRow; row++)
    {
        fAMPTable[ampSet[row]].ampSet = ampSet[row];
        for (int ch = 0; ch < 32; ch++)
        {
            fAMPTable[ampSet[row]].hgPedMean[ch] = vAmpValue[ch][row];
        }
    }

    // Process LG
    ProcessAmpTestFile(sPath + sFileNameLG, ampSet, vAmpValue, NRow);
    for (int row = 0; row < NRow; row++)
    {
        fAMPTable[ampSet[row]].ampSet = ampSet[row];
        for (int ch = 0; ch < 32; ch++)
        {
            fAMPTable[ampSet[row]].lgPedMean[ch] = vAmpValue[ch][row];
        }
    }

    for (int ch = 0; ch < 32; ch++)
    {
        delete[] vAmpValue[ch];
    }
}

void BoardTestResult::GeneratePedDev()
{
    std::string sPath = fsDepoPath + "\\~FEE~PedTest\\";
    std::string sFileNameHG = "board" + std::to_string(fBoardNo) + "PedCALIhg_std.csv";
    std::string sFileNameLG = "board" + std::to_string(fBoardNo) + "PedCALIlg_std.csv";

    double ampSet[50];
    double *vAmpValue[32];
    for (int ch = 0; ch < 32; ch++)
    {
        vAmpValue[ch] = new double[50];
    }
    int NRow;

    // Process HG
    ProcessAmpTestFile(sPath + sFileNameHG, ampSet, vAmpValue, NRow);
    for (int row = 0; row < NRow; row++)
    {
        fAMPTable[ampSet[row]].ampSet = ampSet[row];
        for (int ch = 0; ch < 32; ch++)
        {
            fAMPTable[ampSet[row]].hgPedStdDev[ch] = vAmpValue[ch][row];
        }
    }

    // Process LG
    ProcessAmpTestFile(sPath + sFileNameLG, ampSet, vAmpValue, NRow);
    for (int row = 0; row < NRow; row++)
    {
        fAMPTable[ampSet[row]].ampSet = ampSet[row];
        for (int ch = 0; ch < 32; ch++)
        {
            fAMPTable[ampSet[row]].lgPedStdDev[ch] = vAmpValue[ch][row];
        }
    }

    for (int ch = 0; ch < 32; ch++)
    {
        delete[] vAmpValue[ch];
    }
}

void BoardTestResult::GenerateBias()
{
    std::string sPath = fsDepoPath + "\\~FEE~BiasDACTest\\DACV_test_num_";
    sPath += std::to_string(fBoardNo) + "\\";

    int NRow;
    double biasSet[256];
    double *vValue[32];
    for (int ch = 0; ch < 32; ch++)
    {
        vValue[ch] = new double[256];
    }

    if (fBoardNo == 0)
        ReadBoard0BiasDACTestFile(biasSet, vValue, NRow);
    else
    {
        for (int row = 0; row < 256; row++)
        {
            biasSet[row] = 255 - row;
        }
        for (int ch = 0; ch < 32; ch++)
        {
            ReadBiasTestDACFileOtherBoard(sPath, ch, vValue[ch]);
            NRow = 256;
        }
    }

    for (int row = 0; row < NRow; row++)
    {
        fBiasTable[biasSet[row]].biasSet = biasSet[row];
        for (int ch = 0; ch < 32; ch++)
        {
            fBiasTable[biasSet[row]].vBiasValue[ch] = vValue[ch][row];
        }
    }

    for (int ch = 0; ch < 32; ch++)
    {
        delete[] vValue[ch];
    }
}

bool BoardTestResult::GenerateFromSource(int board, std::string sDepoPath)
{
    if (fValid)
        return false;
    if (board < 0 || board > 10)
        return false;
    fsDepoPath = sDepoPath;
    fBoardNo = board;

    GenerateAMPCali();
    GeneratePed();
    GeneratePedDev();
    GenerateBias();
    fValid = 1;
    return true;
}

void BoardTestResult::Dump(std::ostream &os)
{
    os << "Board: " << fBoardNo << std::endl;
    os << "Is valid: " << fValid << std::endl;
    os << "Amp entries: " << fAMPTable.size() << std::endl;
    std::for_each(fAMPTable.begin(), fAMPTable.end(), [&os](std::pair<int, AMPInfo> a)
                  { os << a.second.ampSet << '\t' << a.second.hgCali[31] << '\t' << a.second.lgCali[31] << '\t' << a.second.hgPedMean[31] << '\t' << a.second.hgPedStdDev[31] << '\t' << a.second.lgPedMean[31] << '\t' << a.second.lgPedStdDev[31] << std::endl; });
    os << "Bias entries: " << fBiasTable.size() << std::endl;
    std::for_each(fBiasTable.begin(), fBiasTable.end(), [&os](std::pair<int, BiasInfo> a)
                  { os << a.second.biasSet << '\t' << a.second.vBiasValue[31] << std::endl; });
}

std::stringstream SiPMTestResult::gss;
bool SiPMTestResult::GenerateFromSiPMTestFile(int board, SIPMBOARDTYPE bt)
{
    if (fIsValid)
        return false;

    fBoardNo = board;
    fBT = bt;

    fsPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\~SiPM~SiPMTest\\2\\";
    if (bt == bottom)
    {
        fsBT = "bottom";
        fChannelOffset = 0;
    }
    else
    {
        fsBT = "top";
        fChannelOffset = 8;
    }
    fsFileName = fsBT + std::to_string(board) + "SiPMTest.csv";

    std::ifstream fin(fsPath + fsFileName);
    std::cout << "Opening: " << fsPath + fsFileName << '\t' << fin.is_open() << std::endl;
    if (!fin.is_open())
        return false;

    fNTMeasPoints = 0;
    fNbiasSetPoints = 0;
    std::string sLine;
    char c;

    int lineCount = 0;

    // Get Temperature Result
    std::getline(fin, sLine);
    lineCount++;
    int TemperatureEndLine = 8;
    if (board == 0 && bt == bottom)
        TemperatureEndLine = 9;
    for (; fin.good() && fin.is_open() && !fin.eof() && lineCount < TemperatureEndLine;)
    {
        std::getline(fin, sLine);
        lineCount++;
        std::string stemp;
        stemp = sLine.substr(0, 11);
        // std::cout << stemp << std::endl;
        if (stemp == "temperature")
            break;
    }
    for (; fin.good() && fin.is_open() && !fin.eof() && lineCount < TemperatureEndLine;)
    {
        std::getline(fin, sLine);
        lineCount++;
        gss.clear();
        gss.str(sLine);

        char c;
        double tempMeas;
        double gainResult;

        gss >> tempMeas;
        if (gss.bad())
            continue;
        fTMeas[0][fNTMeasPoints] = tempMeas;
        gss >> c;
        for (int ch = 0; ch < 8; ch++)
        {
            gss >> gainResult;
            fTMeasResult[0][ch][fNTMeasPoints] = gainResult;
            gss >> c;
        }

        gss >> tempMeas;
        fTMeas[1][fNTMeasPoints] = tempMeas;
        gss >> c;
        for (int ch = 0; ch < 8; ch++)
        {
            gss >> gainResult;
            fTMeasResult[1][ch][fNTMeasPoints] = gainResult;
            gss >> c;
        }

        gss >> tempMeas;
        fTMeas[2][fNTMeasPoints] = tempMeas;
        gss >> c;
        for (int ch = 0; ch < 8; ch++)
        {
            gss >> gainResult;
            fTMeasResult[2][ch][fNTMeasPoints] = gainResult;
            gss >> c;
        }

        fNTMeasPoints++;
    }

    std::string sTemp;

    // Get Line TSlope
    std::getline(fin, sLine);
    lineCount++;
    gss.clear();
    gss.str(sLine);
    for (; gss.good();)
    {
        gss >> c;
        if (c == ',')
            break;
    }
    for (int ch = 0; ch < 24; ch++)
    {
        double temp;
        gss >> temp;
        gss >> c;
        fTSlope[ch] = temp;
    }

    // Line T tranc
    std::getline(fin, sLine);
    lineCount++;

    // Line Temperature compensation factor
    std::getline(fin, sLine);
    lineCount++;

    gss.clear();
    gss.str(sLine);
    for (; gss.good();)
    {
        gss >> c;
        if (c == ',')
            break;
    }
    for (int ch = 0; ch < 24; ch++)
    {
        double temp;
        gss >> temp;
        gss >> c;
        fTCompFactor[ch] = temp;
    }

    // Line Break down voltage
    std::getline(fin, sLine);
    std::cout << sLine << std::endl;
    lineCount++;
    gss.clear();
    gss.str(sLine);
    for (; gss.good();)
    {
        gss >> c;
        if (c == ',')
            break;
    }
    for (int ch = 0; ch < 24; ch++)
    {
        double temp;
        gss >> temp;
        gss >> c;
        fBDVoltage[ch] = temp;
        fBDTemperature[ch] = fTMeas[ch / 8][0];
    }

    // Line Bias Voltage Slope
    std::getline(fin, sLine);
    lineCount++;

    gss.clear();
    gss.str(sLine);
    for (; gss.good();)
    {
        gss >> c;
        if (c == ',')
            break;
    }
    for (int ch = 0; ch < 24; ch++)
    {
        double temp;
        gss >> temp;
        gss >> c;
        fBiasSlope[ch] = temp;
    }

    // Line Bias Tranc
    std::getline(fin, sLine);
    lineCount++;

    int OtherEndLine = 9;
    if (board == 0 && bt == bottom)
        OtherEndLine = 10;

    for (; lineCount < OtherEndLine;)
    {
        std::getline(fin, sLine);
        lineCount++;
    }

    for (; fin.good() && fin.is_open() && !fin.eof();)
    {
        std::getline(fin, sLine);
        // std::cout << sLine << std::endl;
        if (fin.eof())
            break;
        lineCount++;

        int biasSetNow;
        double gainNow;
        char c;

        gss.clear();
        gss.str(sLine);

        gss >> biasSetNow;
        gss >> c;
        fBiasSet[fNbiasSetPoints] = biasSetNow;
        for (int ch = 0; ch < 24; ch++)
        {
            gss >> gainNow;
            gss >> c;
            fValue[ch][fNbiasSetPoints] = gainNow;
        }
        fNbiasSetPoints++;
    }
    fIsValid = 1;
    fin.close();

    return true;
}

void SiPMTestResult::Dump(std::ostream &os)
{
    os << "File name: " << fsPath + fsFileName << std::endl;
    os << "Board: " << fBoardNo << '\t' << fsBT << std::endl;

    os << "Temperature measurement points: " << fNTMeasPoints << std::endl;
    for (int i = 0; i < fNTMeasPoints; i++)
    {
        for (int group = 0; group < 3; group++)
        {
            os << "Group " << group << '\t' << "T: " << fTMeas[group][i] << "C" << '\t';
        }
        os << std::endl;
    }
    for (int i = 0; i < fNbiasSetPoints; i++)
    {
        if (i % 10 != 0)
            continue;
        os << fBiasSet[i] << '\t';
        for (int ch = 0; ch < 24; ch++)
        {
            if (ch % 6 != 0)
                continue;
            os << "Ch: " << ch + fChannelOffset << '\t' << fValue[ch][i] << '\t';
        }
        os << std::endl;
    }
}

void GenerateDBFromSource()
{
    SiPMTestResult *vSiPMTop[6];
    SiPMTestResult *vSiPMBottom[6];
    BoardTestResult *vFEE[11];

    for (int i = 0; i < 6; i++)
    {
        vSiPMBottom[i] = new SiPMTestResult;
        vSiPMBottom[i]->GenerateFromSiPMTestFile(i, bottom);

        vSiPMTop[i] = new SiPMTestResult;
        vSiPMTop[i]->GenerateFromSiPMTestFile(i, top);
    }

    for (int i = 0; i < 11; i++)
    {
        vFEE[i] = new BoardTestResult;
        vFEE[i]->GenerateFromSource(i);
    }
}