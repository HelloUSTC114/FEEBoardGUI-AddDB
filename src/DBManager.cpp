#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <QDebug>
#include <QString>
#include <QVariant>
#include <QSqlError>

#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TF1.h>
#include <TStyle.h>

#include "DBManager.h"

#define DATABASE_VERBOSE 0
// DATABASE_VERBOSE means whether to Generate full database or not, verbose = 0 means generate brief database, while verbose = 1 means generate detail database

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

bool DBManager::DeleteFromTable(QString sTableName, int ID, int boardNo)
{
    QString sSelect = "SELECT ID,BoardNo FROM ";
    sSelect += sTableName + " WHERE ID = ";
    sSelect += QString::number(ID);
    sSelect += ";";
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        // qDebug() << fDBQuery.lastError();
        return false;
    }
    if (fDBQuery.value(1).toInt() != boardNo)
    {
        qDebug() << "Table: " << sTableName << " boardNo do not match. ID: "
                 << ID << "BoardNo: " << boardNo;
        return false;
    }

    QString sDelete = "DELETE FROM ";
    sDelete += sTableName + " WHERE ID = ";
    sDelete += QString::number(ID);
    sDelete += ";";
    fDBQuery.exec(sDelete);
    return true;
}

int DBManager::InsertChannelInfo(int boardNo, double *values, bool *valids)
{
    // Generate Key
    static int gPrimaryKey = 1;
    int currentKey = gPrimaryKey;
    gPrimaryKey++;
    // Only for debug, delete the entry which has the same key
    DeleteChannelInfo(currentKey, boardNo);
    // QVariant
    QString sInsert = "INSERT INTO CHANNELSINFO VALUES (?,?,";
    for (int ch = 0; ch < 32; ch++)
    {
        sInsert += "?";
        if (ch != 31)
            sInsert += ",";
    }
    sInsert += ");";
    fDBQuery.prepare(sInsert);
    fDBQuery.bindValue(0, currentKey);
    fDBQuery.bindValue(1, boardNo);
    for (int ch = 0; ch < 32; ch++)
    {
        if (valids[ch])
            fDBQuery.bindValue(ch + 2, values[ch]);
        else
            fDBQuery.bindValue(ch + 2, QVariant::QVariant());
    }
    if (!fDBQuery.exec())
    {
        qDebug() << fDBQuery.lastError();
    }
    // std::cout << sInsert.toStdString() << std::endl;
    // fDBQuery.prepare()
    return currentKey;
}

int DBManager::ReadChannelInfo(int chID, int &boardNo, double *values, bool *valids)
{
    QString sSelect = "SELECT * FROM CHANNELSINFO WHERE ID=%1;";
    sSelect = sSelect.arg(chID);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    boardNo = fDBQuery.value(1).toInt();
    for (int ch = 0; ch < 32; ch++)
    {
        if (fDBQuery.isNull(ch + 2))
        {
            valids[ch] = 0;
            continue;
        }
        valids[ch] = 1;
        values[ch] = fDBQuery.value(ch + 2).toDouble();
    }
    return chID;
}

bool DBManager::DeleteChannelInfo(int chID, int boardNo)
{
    return DeleteFromTable("CHANNELSINFO", chID, boardNo);
}

int DBManager::InsertTemperatureTestInfo(int boardNo, SIPMBOARDTYPE bt, int nTemperatureTestPoints, int *tempEntry, int *valueEntry)
{
    // Generate Key
    static int gPrimaryKey = 1;
    int currentKey = gPrimaryKey;
    gPrimaryKey++;
    // Only for debug, delete the entry which has the same key
    DeleteTemperatureTestInfo(currentKey, boardNo);
    // QVariant
    QString sInsert = "INSERT INTO TEMPERATURETESTTABLE VALUES (?,?,?,?,";
    for (int ch = 0; ch < 14; ch++)
    {
        sInsert += "?";
        if (ch != 13)
            sInsert += ",";
    }
    sInsert += ");";
    fDBQuery.prepare(sInsert);
    fDBQuery.bindValue(0, currentKey);
    fDBQuery.bindValue(1, boardNo);
    fDBQuery.bindValue(2, int(bt));
    fDBQuery.bindValue(3, nTemperatureTestPoints);
    for (int point = 0; point < nTemperatureTestPoints; point++)
    {
        fDBQuery.bindValue(point * 2 + 4, tempEntry[point]);
        fDBQuery.bindValue(point * 2 + 5, valueEntry[point]);
    }
    for (int point = nTemperatureTestPoints; point < 7; point++)
    {
        fDBQuery.bindValue(point * 2 + 4, QVariant::QVariant());
        fDBQuery.bindValue(point * 2 + 5, QVariant::QVariant());
    }
    if (!fDBQuery.exec())
    {
        qDebug() << fDBQuery.lastError();
    }
    // std::cout << sInsert.toStdString() << std::endl;
    // fDBQuery.prepare()
    return currentKey;
}

int DBManager::ReadTemperatureTestInfo(int tempID, int &boardNo, SIPMBOARDTYPE &bt, int &nTemperatureTestPoints, int *tempEntry, int *valueEntry)
{
    QString sSelect = "SELECT * FROM TEMPERATURETESTTABLE WHERE ID=%1;";
    sSelect = sSelect.arg(tempID);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    boardNo = fDBQuery.value(1).toInt();
    bt = (SIPMBOARDTYPE)fDBQuery.value(2).toInt();
    nTemperatureTestPoints = fDBQuery.value(3).toInt();
    for (int point = 0; point < nTemperatureTestPoints; point++)
    {
        tempEntry[point] = fDBQuery.value(point * 2 + 4).toDouble();
        valueEntry[point] = fDBQuery.value(point * 2 + 5).toDouble();
    }
    return tempID;
}

bool DBManager::DeleteTemperatureTestInfo(int tempID, int boardNo)
{
    return DeleteFromTable("TEMPERATURETESTTABLE", tempID, boardNo);
}

int DBManager::InsertSiPMBoardTestInfo(int boardNo, SIPMBOARDTYPE bt, int tempTableEntry, int biasTableEntry, int TSlopeEntry, int BiasSlopeEntry, int BDVEntry, int BDTEntry, int TCompFactorEntry)
{
    // Generate Key
    static int gPrimaryKey = 1;
    int currentKey = gPrimaryKey;
    gPrimaryKey++;
    // Only for debug, delete the entry which has the same key
    DeleteSiPMBoardTestInfo(currentKey, boardNo);
    // QVariant
    QString sInsert = "INSERT INTO SIPMBOARD VALUES (?,?,?,?,?,?,?,?,?,?)";

    fDBQuery.prepare(sInsert);
    fDBQuery.bindValue(0, currentKey);
    fDBQuery.bindValue(1, boardNo);
    fDBQuery.bindValue(2, int(bt));
    fDBQuery.bindValue(3, tempTableEntry);
    fDBQuery.bindValue(4, biasTableEntry);
    fDBQuery.bindValue(5, TSlopeEntry);
    fDBQuery.bindValue(6, BiasSlopeEntry);
    fDBQuery.bindValue(7, BDVEntry);
    fDBQuery.bindValue(8, BDTEntry);
    fDBQuery.bindValue(9, TCompFactorEntry);

#if (DATABASE_VERBOSE == 0)
    fDBQuery.bindValue(3, QVariant::QVariant());
    fDBQuery.bindValue(4, QVariant::QVariant());
#endif

    if (!fDBQuery.exec())
    {
        qDebug() << fDBQuery.lastError();
    }
    // std::cout << sInsert.toStdString() << std::endl;
    // fDBQuery.prepare()
    return currentKey;
}

int DBManager::ReadSiPMBoardTestInfo(int boardID, int &boardNo, SIPMBOARDTYPE &bt, int &tempTableEntry, int &biasTableEntry, int &TSlopeEntry, int &BiasSlopeEntry, int &BDVEntry, int &BDTEntry, int &TCompFactorEntry)
{
    QString sSelect = "SELECT * FROM SIPMBOARD WHERE ID=%1;";
    sSelect = sSelect.arg(boardID);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    boardNo = fDBQuery.value(1).toInt();
    bt = (SIPMBOARDTYPE)fDBQuery.value(2).toInt();
    tempTableEntry = fDBQuery.value(3).toInt();
    biasTableEntry = fDBQuery.value(4).toInt();
    TSlopeEntry = fDBQuery.value(5).toInt();
    BiasSlopeEntry = fDBQuery.value(6).toInt();
    BDVEntry = fDBQuery.value(7).toInt();
    BDTEntry = fDBQuery.value(8).toInt();
    TCompFactorEntry = fDBQuery.value(9).toInt();
    return boardID;
}

int DBManager::ReadSiPMBoardTestInfo(int boardNo, SIPMBOARDTYPE bt, int &tempTableEntry, int &biasTableEntry, int &TSlopeEntry, int &BiasSlopeEntry, int &BDVEntry, int &BDTEntry, int &TCompFactorEntry)
{
    QString sSelect = "SELECT ID FROM SIPMBOARD WHERE (BoardNo = %1 AND BT = %2);";
    sSelect = sSelect.arg(boardNo).arg(bt);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    int boardID = fDBQuery.value(0).toInt();
    int boardNo1;
    SIPMBOARDTYPE bt1;
    auto rtn = ReadSiPMBoardTestInfo(boardID, boardNo1, bt1, tempTableEntry, biasTableEntry, TSlopeEntry, BiasSlopeEntry, BDVEntry, BDTEntry, TCompFactorEntry);
    return rtn;
}

bool DBManager::DeleteSiPMBoardTestInfo(int tempID, int boardNo)
{
    return DeleteFromTable("SIPMBOARD", tempID, boardNo);
}

int DBManager::InsertBiasEntryInfo(int boardNo, int biasSet, int biasChEntry)
{
    // Generate Key
    static int gPrimaryKey = 1;
    int currentKey = gPrimaryKey;
    gPrimaryKey++;
    // Only for debug, delete the entry which has the same key
    DeleteBiasEntryInfo(currentKey, boardNo);
    // QVariant
    QString sInsert = "INSERT INTO BIASINFO VALUES (?,?,?,?)";
    fDBQuery.prepare(sInsert);
    fDBQuery.bindValue(0, currentKey);
    fDBQuery.bindValue(1, boardNo);
    fDBQuery.bindValue(2, biasSet);
    fDBQuery.bindValue(3, biasChEntry);
    if (!fDBQuery.exec())
    {
        qDebug() << fDBQuery.lastError();
    }
    return currentKey;
}

int DBManager::ReadBiasEntryInfo(int biasID, int &boardNo, int &biasSet, int &biasChEntry)
{
    QString sSelect = "SELECT * FROM BIASINFO WHERE ID=%1;";
    sSelect = sSelect.arg(biasID);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    boardNo = fDBQuery.value(1).toInt();
    biasSet = fDBQuery.value(2).toInt();
    biasChEntry = fDBQuery.value(3).toInt();
    return biasID;
}

bool DBManager::DeleteBiasEntryInfo(int biasID, int boardNo)
{
    return DeleteFromTable("BIASINFO", biasID, boardNo);
}

int DBManager::InsertAmpEntryInfo(int boardNo, int ampSet, int hgPedChEntry, int hgPedStdChEntry, int lgPedChEntry, int lgPedStdChEntry, int hgCaliChEntry, int lgCaliChEntry)
{
    // Generate Key
    static int gPrimaryKey = 1;
    int currentKey = gPrimaryKey;
    gPrimaryKey++;
    // Only for debug, delete the entry which has the same key
    DeleteAmpEntryInfo(currentKey, boardNo);
    // QVariant
    QString sInsert = "INSERT INTO AMPINFO VALUES (?,?,?,?,?,?,?,?,?)";
    fDBQuery.prepare(sInsert);
    fDBQuery.bindValue(0, currentKey);
    fDBQuery.bindValue(1, boardNo);
    fDBQuery.bindValue(2, ampSet);
    fDBQuery.bindValue(3, hgPedChEntry);
    fDBQuery.bindValue(4, hgPedStdChEntry);
    fDBQuery.bindValue(5, lgPedChEntry);
    fDBQuery.bindValue(6, lgPedStdChEntry);
    fDBQuery.bindValue(7, hgCaliChEntry);
    fDBQuery.bindValue(8, lgCaliChEntry);
    if (!fDBQuery.exec())
    {
        qDebug() << fDBQuery.lastError();
    }
    return currentKey;
}

int DBManager::ReadAmpEntryInfo(int ampID, int &boardNo, int &ampSet, int &hgPedChEntry, int &hgPedStdChEntry, int &lgPedChEntry, int &lgPedStdChEntry, int &hgCaliChEntry, int &lgCaliChEntry)
{
    QString sSelect = "SELECT * FROM AMPINFO WHERE ID=%1;";
    sSelect = sSelect.arg(ampID);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    boardNo = fDBQuery.value(1).toInt();
    ampSet = fDBQuery.value(2).toInt();
    hgPedChEntry = fDBQuery.value(3).toInt();
    hgPedStdChEntry = fDBQuery.value(4).toInt();
    lgPedChEntry = fDBQuery.value(5).toInt();
    lgPedStdChEntry = fDBQuery.value(6).toInt();
    hgCaliChEntry = fDBQuery.value(7).toInt();
    lgCaliChEntry = fDBQuery.value(8).toInt();
    return ampID;
}

bool DBManager::DeleteAmpEntryInfo(int ampID, int boardNo)
{
    return DeleteFromTable("AMPINFO", ampID, boardNo);
}

int DBManager::InsertAmpTable(int boardNo, int *ampEntries, bool *validation)
{
    // Generate Key
    static int gPrimaryKey = 1;
    int currentKey = gPrimaryKey;
    gPrimaryKey++;
    // Only for debug, delete the entry which has the same key
    DeleteAmpTable(currentKey, boardNo);
    // QVariant
    QString sInsert = "INSERT INTO AMPTABLE VALUES (?,?";
    for (int i = 0; i < 63; i++)
    {
        sInsert += ",?";
    }
    sInsert += ");";
    fDBQuery.prepare(sInsert);
    fDBQuery.bindValue(0, currentKey);
    fDBQuery.bindValue(1, boardNo);
    for (int amp = 0; amp < 63; amp++)
    {
        if (validation[amp])
            fDBQuery.bindValue(amp + 2, ampEntries[amp]);
        else
            fDBQuery.bindValue(amp + 2, QVariant::QVariant());
    }
    if (!fDBQuery.exec())
    {
        qDebug() << fDBQuery.lastError();
    }
    return currentKey;
}

int DBManager::ReadAmpTable(int ampTableID, int &boardNo, int *ampEntries, bool *validation)
{
    QString sSelect = "SELECT * FROM AMPTABLE WHERE ID=%1;";
    sSelect = sSelect.arg(ampTableID);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    boardNo = fDBQuery.value(1).toInt();
    for (int amp = 0; amp < 63; amp++)
    {
        if (fDBQuery.isNull(amp + 2))
        {
            validation[amp] = 0;
            ampEntries[amp] = -1;
            continue;
        }
        validation[amp] = 1;
        ampEntries[amp] = fDBQuery.value(amp + 2).toInt();
    }
    return ampTableID;
}

bool DBManager::DeleteAmpTable(int ampTableID, int boardNo)
{
    return DeleteFromTable("AMPTABLE", ampTableID, boardNo);
}

DBManager::DBManager()
{
}

DBManager::~DBManager()
{
    CloseDB();
}

int DBManager::InsertBiasTableEntry(int boardNo, int *biasEntries, bool *validation)
{
    // Generate Key
    static int gPrimaryKey = 1;
    int currentKey = gPrimaryKey;
    gPrimaryKey++;
    // Only for debug, delete the entry which has the same key
    DeleteBiasTableEntry(currentKey, boardNo);
    // QVariant
    QString sInsert = "INSERT INTO BIASTABLE VALUES (?,?";
    for (int i = 0; i < 256; i++)
    {
        sInsert += ",?";
    }
    sInsert += ");";
    fDBQuery.prepare(sInsert);
    fDBQuery.bindValue(0, currentKey);
    fDBQuery.bindValue(1, boardNo);
    for (int bias = 0; bias < 256; bias++)
    {
        if (validation[bias])
            fDBQuery.bindValue(bias + 2, biasEntries[bias]);
        else
            fDBQuery.bindValue(bias + 2, QVariant::QVariant());
    }
    if (!fDBQuery.exec())
    {
        qDebug() << fDBQuery.lastError();
    }
    return currentKey;
}

int DBManager::ReadBiasTableEntry(int biasTableID, int &boardNo, int *biasEntries, bool *validation)
{
    QString sSelect = "SELECT * FROM BIASTABLE WHERE ID=%1;";
    sSelect = sSelect.arg(biasTableID);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    boardNo = fDBQuery.value(1).toInt();
    for (int bias = 0; bias < 256; bias++)
    {
        if (fDBQuery.isNull(bias + 2))
        {
            validation[bias] = 0;
            biasEntries[bias] = -1;
            continue;
        }
        validation[bias] = 1;
        biasEntries[bias] = fDBQuery.value(bias + 2).toInt();
    }
    return biasTableID;
}

bool DBManager::DeleteBiasTableEntry(int biasTableID, int boardNo)
{
    return DeleteFromTable("BIASTABLE", biasTableID, boardNo);
}

int DBManager::InsertFEEBoardEntry(int boardNo, int ampTableEntry, int biasTableEntry)
{
    // Generate Key
    static int gPrimaryKey = 1;
    int currentKey = gPrimaryKey;
    gPrimaryKey++;
    // Only for debug, delete the entry which has the same key
    DeleteFEEBoardEntry(currentKey, boardNo);
    // QVariant
    QString sInsert = "INSERT INTO FEEBOARD VALUES (?,?,?,?)";
    fDBQuery.prepare(sInsert);
    fDBQuery.bindValue(0, currentKey);
    fDBQuery.bindValue(1, boardNo);
    fDBQuery.bindValue(2, ampTableEntry);
    fDBQuery.bindValue(3, biasTableEntry);
    if (!fDBQuery.exec())
    {
        qDebug() << fDBQuery.lastError();
    }
    return currentKey;
}

int DBManager::ReadFEEBoardEntry(int feeBoardID, int &boardNo, int &ampTableEntry, int &biasTableEntry)
{
    QString sSelect = "SELECT * FROM AMPINFO WHERE ID=%1;";
    sSelect = sSelect.arg(feeBoardID);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    boardNo = fDBQuery.value(1).toInt();
    ampTableEntry = fDBQuery.value(2).toInt();
    biasTableEntry = fDBQuery.value(3).toInt();
    return feeBoardID;
}

int DBManager::ReadFEEBoardEntry(int boardNo, int &ampTableEntry, int &biasTableEntry)
{
    QString sSelect = "SELECT ID FROM SIPMBOARD WHERE (BoardNo = %1 AND BT = %2);";
    sSelect = sSelect.arg(boardNo);
    fDBQuery.exec(sSelect);
    if (!fDBQuery.next())
    {
        qDebug() << fDBQuery.lastError();
        return -1;
    }
    int boardID = fDBQuery.value(0).toInt();
    int boardNo1;
    auto rtn = ReadFEEBoardEntry(boardID, boardNo1, ampTableEntry, biasTableEntry);
    return rtn;
}

bool DBManager::DeleteFEEBoardEntry(int feeBoardID, int boardNo)
{
    return DeleteFromTable("FEEBOARD", feeBoardID, boardNo);
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
    fDBQuery = QSqlQuery(fDataBase);

    QString sSelectTable = "SELECT tbl_name FROM sqlite_master WHERE type = 'table';";
    fDBQuery.exec(sSelectTable);
    int count = 0;
    std::cout << "Reading Tables: " << std::endl;
    for (; fDBQuery.next();)
    {
        count++;
        // std::cout << count << '\t' << fDBQuery.value(0).toString().toStdString() << std::endl;
    }
    if (count < 9)
    {
        std::cout << "Do not get enough table, Creating tables." << std::endl;
        QString scmd;
        scmd = "sqlite3 ";
        scmd += sDBName;
        // F:/Projects/FEEDistri/DataBase/Calibration.db
        scmd += " \".read F:/Projects/FEEDistri/DataBase/GenerateDB.sql\"";
        fProcess.execute(scmd);
        // qDebug() << fProcess.error();
        // fDBQuery.exec(".read F:\\Projects\\FEEDistri\\DataBase\\GenerateDB.sql");
        // qDebug() << fDBQuery.lastError();
    }
}

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

int BoardTestResult::WriteBiasTestEntry()
{
    if (!fIsValid)
        return -1;

    int biasKey[256];
    bool biasValid[256];
    for (int bias = 0; bias < 256; bias++)
    {
        biasValid[bias] = 0;
    }

    for (auto iter = fBiasTable.begin(); iter != fBiasTable.end(); iter++)
    {
        bool chValid[32];
        double biasMeasValue[32];
        int biasSet = iter->first;
        for (int ch = 0; ch < 32; ch++)
        {
            chValid[ch] = 1;
            biasMeasValue[ch] = iter->second.vBiasValue[ch];
        }
        int chKey = gDBManager->InsertChannelInfo(fBoardNo, biasMeasValue, chValid);
        biasKey[iter->first] = gDBManager->InsertBiasEntryInfo(fBoardNo, biasSet, chKey);
        biasValid[iter->first] = 1;
    }
    return gDBManager->InsertBiasTableEntry(fBoardNo, biasKey, biasValid);
}

int BoardTestResult::WriteAmpTestEntry()
{
    if (!fIsValid)
        return -1;

    int ampKey[63];
    bool ampValid[63];

    for (int amp = 0; amp < 63; amp++)
    {
        ampValid[amp] = 0;
    }

    for (auto iter = fAMPTable.begin(); iter != fAMPTable.end(); iter++)
    {
        bool chValid[32];
        double hgPedMean[32];
        double hgPedStdDev[32];
        double lgPedMean[32];
        double lgPedStdDev[32];
        double hgCali[32];
        double lgCali[32];
        int ampSet = iter->first;
        for (int ch = 0; ch < 32; ch++)
        {
            chValid[ch] = 1;
            hgPedMean[ch] = iter->second.hgPedMean[ch];
            hgPedStdDev[ch] = iter->second.hgPedStdDev[ch];
            lgPedMean[ch] = iter->second.lgPedMean[ch];
            lgPedStdDev[ch] = iter->second.lgPedStdDev[ch];
            hgCali[ch] = iter->second.hgCali[ch];
            lgCali[ch] = iter->second.lgCali[ch];
        }
        int hgPedMeanKey = gDBManager->InsertChannelInfo(fBoardNo, hgPedMean, chValid);
        int hgPedStdDevKey = gDBManager->InsertChannelInfo(fBoardNo, hgPedStdDev, chValid);
        int lgPedMeanKey = gDBManager->InsertChannelInfo(fBoardNo, lgPedMean, chValid);
        int lgPedStdDevKey = gDBManager->InsertChannelInfo(fBoardNo, lgPedStdDev, chValid);
        int hgCaliKey = gDBManager->InsertChannelInfo(fBoardNo, hgCali, chValid);
        int lgCaliKey = gDBManager->InsertChannelInfo(fBoardNo, lgCali, chValid);

        ampKey[iter->first] = gDBManager->InsertAmpEntryInfo(fBoardNo, ampSet, hgPedMeanKey, hgPedStdDevKey, lgPedMeanKey, lgPedStdDevKey, hgCaliKey, lgCaliKey);
        ampValid[iter->first] = 1;
    }
    return gDBManager->InsertAmpTable(fBoardNo, ampKey, ampValid);
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
    if (fIsValid)
        return false;
    if (board < 0 || board > 10)
        return false;
    fsDepoPath = sDepoPath;
    fBoardNo = board;

    GenerateAMPCali();
    GeneratePed();
    GeneratePedDev();
    GenerateBias();
    fIsValid = 1;
    return true;
}

void BoardTestResult::Dump(std::ostream &os)
{
    os << "Board: " << fBoardNo << std::endl;
    os << "Is valid: " << fIsValid << std::endl;
    os << "Amp entries: " << fAMPTable.size() << std::endl;
    std::for_each(fAMPTable.begin(), fAMPTable.end(), [&os](std::pair<int, AMPInfo> a)
                  { os << a.second.ampSet << '\t' << a.second.hgCali[31] << '\t' << a.second.lgCali[31] << '\t' << a.second.hgPedMean[31] << '\t' << a.second.hgPedStdDev[31] << '\t' << a.second.lgPedMean[31] << '\t' << a.second.lgPedStdDev[31] << std::endl; });
    os << "Bias entries: " << fBiasTable.size() << std::endl;
    std::for_each(fBiasTable.begin(), fBiasTable.end(), [&os](std::pair<int, BiasInfo> a)
                  { os << a.second.biasSet << '\t' << a.second.vBiasValue[31] << std::endl; });
}

double BoardTestResult::GetCaliResult(int ch, int ampDAC, GAINTYPE hl)
{
    if (ampDAC < 0 || ampDAC > 62)
        return -1;
    if (fAMPTable.find(ampDAC) == fAMPTable.end())
    {
        if (hl == hg)
        {
            double factor = fAMPTable[43].hgCali[ch] / CalcAmpMultiFactor(43, hg);
            return factor * CalcAmpMultiFactor(ampDAC, hg);
        }
        else
        {
            double factor = fAMPTable[43].hgCali[ch] / CalcAmpMultiFactor(43, hg);
            return factor * CalcAmpMultiFactor(ampDAC, lg);
        }
    }
    if (hl == hg)
        return fAMPTable[ampDAC].hgCali[ch];
    else
        return fAMPTable[ampDAC].lgCali[ch];
    return 0.0;
}

double BoardTestResult::GetPed(int ch, int ampDAC, GAINTYPE hl)
{
    if (ampDAC < 0 || ampDAC > 62)
        return -1;
    if (fAMPTable.find(ampDAC) == fAMPTable.end())
    {
        if (hl == hg)
        {
            return fAMPTable[43].hgPedMean[ch];
        }
        else
        {
            return fAMPTable[43].lgPedMean[ch];
        }
    }
    if (hl == hg)
        return fAMPTable[ampDAC].hgPedMean[ch];
    else
        return fAMPTable[ampDAC].lgPedMean[ch];
    return 0.0;
}

int BoardTestResult::WriteIntoDB()
{
    std::cout << "Writing into Database: board:" << fBoardNo << std::endl;
    auto ampEntry = WriteAmpTestEntry();
    auto biasEntry = WriteBiasTestEntry();
    return gDBManager->InsertFEEBoardEntry(fBoardNo, ampEntry, biasEntry);
}

std::stringstream SiPMTestResult::gss;
bool SiPMTestResult::GenerateFromSiPMTestFile(int board, SIPMBOARDTYPE bt)
{
    if (fIsValid)
        return false;

    InitBoardNo(board, bt);

    // fBoardNo = board;
    // fBT = bt;

    // fsPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\~SiPM~SiPMTest\\3\\";
    // if (bt == bottom)
    // {
    //     fsBT = "bottom";
    //     fChannelOffset = 0;
    // }
    // else
    // {
    //     fsBT = "top";
    //     fChannelOffset = 8;
    // }
    // fsFileName = fsBT + std::to_string(board) + "SiPMTest.csv";

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
        if (gss.bad() || gss.eof() || !gss.good())
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
        if (gss.eof() || !gss.good())
            continue;
        gss >> c;
        // fBiasSet[fNbiasSetPoints] = biasSetNow;
        for (int ch = 0; ch < 24; ch++)
        {
            gss >> gainNow;
            gss >> c;
            // fValue[ch][fNbiasSetPoints] = gainNow;
            fBiasSetMap[biasSetNow][ch] = gainNow;
        }
        fNbiasSetPoints++;
    }
    fIsValid = 1;
    fin.close();
    fGeneratedFromDB = 0;
    fGeneratedFromSource = 1;

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

    int offset = fChannelOffset;
    std::for_each(fBiasSetMap.begin(), fBiasSetMap.end(), [offset, &os](std::pair<const int, double[24]> &a)
                  {
                    os << a.first << '\t';
                    for (int ch = 0; ch < 24; ch++)
                    {
                      os << "Ch:" << ch + offset << '\t' << a.second[ch] << '\t';
                    } 
                    os << std::endl; });
    for (auto iter = fBiasSetMap.begin(); iter != fBiasSetMap.end(); iter++)
    {
        os << iter->first << '\t';
        for (int ch = 0; ch < 24; ch++)
        {
            if (ch % 6 != 0)
                continue;
            os << "Ch: " << ch + fChannelOffset << '\t' << iter->second[ch] << '\t';
        }
        os << std::endl;
    }
}

TGraphErrors *SiPMTestResult::GetTMeasureGraph(TGraphErrors *tge, int ch)
{
    if (!IsDetailed())
        return NULL;
    tge->Set(0);
    for (int i = 0; i < fNTMeasPoints; i++)
    {
        tge->SetPoint(i, fTMeas[ch / 8][i], fTMeasResult[ch / 8][ch % 8][i]);
        tge->SetPointError(i, 0.5, 1);
    }
    return tge;
}

int SiPMTestResult::WriteTempEntry()
{
    if (!fIsValid)
        return -1;
    int TKey[10];
    int TResultKey[10];

    double TMeas[32];
    double TMeasResult[32];
    bool chValid[32];

    for (int point = 0; point < fNTMeasPoints; point++)
    {
        for (int ch = 0; ch < 32; ch++)
        {
            if (ch < fChannelOffset || ch - fChannelOffset > 23)
            {
                chValid[ch] = 0;
                continue;
            }

            chValid[ch] = 1;
            TMeas[ch] = fTMeas[(ch - fChannelOffset) / 8][point];
            TMeasResult[ch] = fTMeasResult[(ch - fChannelOffset) / 8][(ch - fChannelOffset) % 8][point];
        }
        TKey[point] = gDBManager->InsertChannelInfo(fBoardNo, TMeas, chValid);
        TResultKey[point] = gDBManager->InsertChannelInfo(fBoardNo, TMeasResult, chValid);
    }
    return gDBManager->InsertTemperatureTestInfo(fBoardNo, fBT, fNTMeasPoints, TKey, TResultKey);
}

int SiPMTestResult::WriteBiasTestEntry()
{
    if (!fIsValid)
        return -1;

    int biasKey[256];
    bool biasValid[256];
    for (int bias = 0; bias < 256; bias++)
    {
        biasValid[bias] = 0;
    }

    for (auto iter = fBiasSetMap.begin(); iter != fBiasSetMap.end(); iter++)
    {
        bool chValid[32];
        double biasMeasValue[32];
        int biasSet = iter->first;
        for (int ch = 0; ch < 32; ch++)
        {
            if (ch < fChannelOffset || ch - fChannelOffset > 23)
            {
                chValid[ch] = 0;
                continue;
            }
            chValid[ch] = 1;
            biasMeasValue[ch] = iter->second[ch - fChannelOffset];
            // if (biasMeasValue[ch - fChannelOffset] == 0 && fBT == top)
            //     std::cout << fBoardNo << '\t' << fsBT << '\t' << biasSet << '\t' << ch << '\t' << fChannelOffset << std::endl;
        }
        int chKey = gDBManager->InsertChannelInfo(fBoardNo, biasMeasValue, chValid);
        biasKey[iter->first] = gDBManager->InsertBiasEntryInfo(fBoardNo, biasSet, chKey);
        biasValid[iter->first] = 1;
    }
    return gDBManager->InsertBiasTableEntry(fBoardNo, biasKey, biasValid);
}

bool SiPMTestResult::ReadTempEntry(int tempTableEntry)
{
    int board1;
    SIPMBOARDTYPE bt1;

    int TKey[10];
    int TResultKey[10];

    double TMeas[32];
    double TMeasResult[32];
    bool chValid[32];
    auto rtn = gDBManager->ReadTemperatureTestInfo(tempTableEntry, board1, bt1, fNTMeasPoints, TKey, TResultKey);
    if (rtn < 0)
        return false;

    for (int point = 0; point < fNTMeasPoints; point++)
    {
        rtn = gDBManager->ReadChannelInfo(TKey[point], board1, TMeas, chValid);
        if (rtn < 0)
            return false;
        rtn = gDBManager->ReadChannelInfo(TResultKey[point], board1, TMeasResult, chValid);
        if (rtn < 0)
            return false;
        for (int ch = 0; ch < 32; ch++)
        {
            if (!chValid[ch])
                continue;
            fTMeas[(ch - fChannelOffset) / 8][point];
            fTMeasResult[(ch - fChannelOffset) / 8][(ch - fChannelOffset) % 8][point] = TMeasResult[ch];
        }
    }

    return true;
}

bool SiPMTestResult::ReadBiasTestEntry(int biasTableEntry)
{
    int board1;

    int biasKey[256];
    bool biasValid[256];
    int rtn;
    rtn = gDBManager->ReadBiasTableEntry(biasTableEntry, board1, biasKey, biasValid);
    if (rtn < 0)
        return false;
    for (int bias = 0; bias < 256; bias++)
    {
        if (!biasValid[bias])
            continue;
        bool chValid[32];
        double biasMeasValue[32];
        int biasSet;
        int biaschKey;
        gDBManager->ReadBiasEntryInfo(biasKey[bias], board1, biasSet, biaschKey);
        if (rtn < 0)
            return false;
        // Here biasSet, which is read from DB, mush be eqiuvalence with bias
        if (biasSet != bias)
        {
            std::cout << "Fatal error in reading algorithm:" << std::endl;
            return false;
        }
        gDBManager->ReadChannelInfo(biaschKey, board1, biasMeasValue, chValid);
        if (rtn < 0)
            return false;
        for (int ch = 0; ch < 32; ch++)
        {
            if (!chValid[ch])
                continue;
            fBiasSetMap[bias][ch] = biasMeasValue[ch - fChannelOffset];
        }
    }
    return true;
}

void SiPMTestResult::InitBoardNo(int boardNo, SIPMBOARDTYPE bt, std::string spath)
{
    if (fIsValid)
        return;

    fBoardNo = boardNo;
    fBT = bt;

    fsPath = spath;
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
    fsFileName = fsBT + std::to_string(boardNo) + "SiPMTest.csv";
}

int SiPMTestResult::WriteIntoDB()
{
    std::cout << "Writing into Database: " << fsBT << "-" << fBoardNo << std::endl;

    int tempTableEntry;
    int biasTableEntry;
#if (DATABASE_VERBOSE == 1)
    tempTableEntry = WriteTempEntry();
    biasTableEntry = WriteBiasTestEntry();
#endif

    bool chValid[32];
    double TSlope[32];
    double BiasSlope[32];
    double BDV[32];
    double BDT[32];
    double TCompFactor[32];
    for (int ch = 0; ch < 32; ch++)
    {
        if (ch < fChannelOffset || ch - fChannelOffset > 23)
        {
            chValid[ch] = 0;
            continue;
        }
        TSlope[ch] = fTSlope[ch - fChannelOffset];
        BiasSlope[ch] = fBiasSlope[ch - fChannelOffset];
        BDV[ch] = fBDVoltage[ch - fChannelOffset];
        BDT[ch] = fBDTemperature[ch - fChannelOffset];
        TCompFactor[ch] = fTCompFactor[ch - fChannelOffset];
        chValid[ch] = 1;
    }
    auto TSlopeEntry = gDBManager->InsertChannelInfo(fBoardNo, TSlope, chValid);
    auto BiasSlopeEntry = gDBManager->InsertChannelInfo(fBoardNo, BiasSlope, chValid);
    auto BDVEntry = gDBManager->InsertChannelInfo(fBoardNo, BDV, chValid);
    auto BDTEntry = gDBManager->InsertChannelInfo(fBoardNo, BDT, chValid);
    auto TCompFactorEntry = gDBManager->InsertChannelInfo(fBoardNo, TCompFactor, chValid);

    return gDBManager->InsertSiPMBoardTestInfo(fBoardNo, fBT, tempTableEntry, biasTableEntry, TSlopeEntry, BiasSlopeEntry, BDVEntry, BDTEntry, TCompFactorEntry);
}

bool SiPMTestResult::ReadFromDB(int board, SIPMBOARDTYPE bt)
{
    if (!gDBManager->IsInitiated())
        return false;
    if (fIsValid)
        return false;
    InitBoardNo(board, bt);

    int tempTableEntry, biasTableEntry, TSlopeEntry, BiasSlopeEntry, BDVEntry, BDTEntry, TCompFactorEntry;
    auto rtn = gDBManager->ReadSiPMBoardTestInfo(board, bt, tempTableEntry, biasTableEntry, TSlopeEntry, BiasSlopeEntry, BDVEntry, BDTEntry, TCompFactorEntry);
    if (rtn < 0)
        return false;

    fBoardNo = board;
    fBT = bt;
    int board1;

    bool chValid[32];
    double TSlope[32];
    double BiasSlope[32];
    double BDV[32];
    double BDT[32];
    double TCompFactor[32];
    gDBManager->ReadChannelInfo(TSlopeEntry, board1, TSlope, chValid);
    gDBManager->ReadChannelInfo(BiasSlopeEntry, board1, BiasSlope, chValid);
    gDBManager->ReadChannelInfo(BDVEntry, board1, BDV, chValid);
    gDBManager->ReadChannelInfo(BDTEntry, board1, BDT, chValid);
    gDBManager->ReadChannelInfo(TCompFactorEntry, board1, TCompFactor, chValid);

    //! TODO: write from these arrays to inside arrays
    for (int ch = 0; ch < 32; ch++)
    {
        if (!chValid[ch])
            continue;
        fTSlope[ch - fChannelOffset] = TSlope[ch];
        fBiasSlope[ch - fChannelOffset] = BiasSlope[ch];
        fBDVoltage[ch - fChannelOffset] = BDV[ch];
        fBDTemperature[ch - fChannelOffset] = BDT[ch];
        fTCompFactor[ch - fChannelOffset] = TCompFactor[ch];
    }

    fIsValid = true;
    fGeneratedFromDB = 1;
    fGeneratedFromSource = 0;
    fDBIsDetailed = 0;

#if (DATABASE_VERBOSE == 1)
    ReadBiasTestEntry(biasTableEntry);
    ReadTempEntry(tempTableEntry);
    fDBIsDetailed = 1;
#endif

    return true;
}

// #define ANALYZE_DATA 1

void GenerateDBFromSource()
{
    SiPMTestResult *vSiPMTop[6];
    SiPMTestResult *vSiPMBottom[6];
    BoardTestResult *vFEE[11];

    for (int i = 0; i < 6; i++)
    {
        vSiPMBottom[i] = new SiPMTestResult;
        vSiPMBottom[i]->GenerateFromSiPMTestFile(i, bottom);
#ifndef ANALYZE_DATA
        vSiPMBottom[i]->WriteIntoDB();
#endif
        vSiPMTop[i] = new SiPMTestResult;
        vSiPMTop[i]->GenerateFromSiPMTestFile(i, top);
#ifndef ANALYZE_DATA
        vSiPMTop[i]->WriteIntoDB();
#endif
    }

    for (int i = 0; i < 11; i++)
    {
        vFEE[i] = new BoardTestResult;
        vFEE[i]->GenerateFromSource(i);
#ifndef ANALYZE_DATA
        vFEE[i]->WriteIntoDB();
#endif
    }

#ifdef ANALYZE_DATA
    std::vector<std::pair<int, int>> mapTop;
    std::vector<std::pair<int, int>> mapBottom;
    gStyle->SetOptFit(111);
    auto f = new TF1("f", "pol1", 0, 60);
    auto c = new TCanvas("c", "c", 1);
    auto file = new TFile("AnalyzeFile.root", "recreate");
    auto hBD = new TH1D("hBD", "hBD", 50, 50, 52.5);
    auto hGain = new TH1D("hGain", "hGain", 50, 150, 250);
    auto hGainCor = new TH1D("hGainCor", "hGainCor", 50, 150, 250);
    auto hCompFactor = new TH1D("hCompFactor", "hCompFactor", 50, 30, 80);
    // auto hPedHG = new TH1D("hPedHG", "hPedHG", 50, 3000, 5000);
    auto hPed = new TH2D("hPed", "hPed", 50, 2500, 6000, 50, 2500, 6000);
    auto hBias0 = new TH1D("hBias0", "hBias0", 50, 4, 5);
    auto hBias255 = new TH1D("hBias255", "hBias255", 50, 0, 1);

    auto tg = new TGraphErrors();
    tg->SetTitle("T Measure;T/^#{circ}C;ADCValue");

    double vAmpCor[32];
    for (int ch = 0; ch < 32; ch++)
    {
        vAmpCor[ch] = vFEE[0]->GetCaliResult(ch, 32, hg) / vFEE[0]->GetCaliResult(0, 32, hg);
    }

    for (int board = 0; board < 6; board++)
    {
        for (int ch = 0; ch < 24; ch++)
        {
            auto bd1 = vSiPMBottom[board]->GetBDVoltageV(ch) + (vSiPMBottom[board]->GetBDVoltageT(ch) - 25) * vSiPMBottom[board]->GetTCompFactor(ch) / 1000.0;
            // auto bd1 = vSiPMBottom[board]->GetBDVoltageV(ch);
            if (bd1 < 50 || bd1 > 60)
                std::cout << "Error bot: " << board << '\t' << ch << std::endl;
            auto bd2 = vSiPMTop[board]->GetBDVoltageV(ch) + (vSiPMTop[board]->GetBDVoltageT(ch) - 25) * vSiPMTop[board]->GetTCompFactor(ch) / 1000.0;
            // auto bd2 = vSiPMTop[board]->GetBDVoltageV(ch);
            if (bd2 < 50 || bd2 > 60)
                std::cout << "Error top: " << board << '\t' << ch << std::endl;
            hBD->Fill(bd1);
            hBD->Fill(bd2);

            hGain->Fill(vSiPMBottom[board]->GetBiasSlope(ch));
            hGain->Fill(vSiPMTop[board]->GetBiasSlope(ch));

            hGainCor->Fill(vSiPMBottom[board]->GetBiasSlope(ch) * vAmpCor[ch]);
            hGainCor->Fill(vSiPMTop[board]->GetBiasSlope(ch) * vAmpCor[ch]);

            f->SetParameters(800, -10);
            tg = vSiPMBottom[board]->GetTMeasureGraph(tg, ch);
            tg->SetTitle(Form("board-%d-bottom-ch%d;T/^{#circ}C;ADCValue", board, vSiPMBottom[board]->GetRealChannel(ch)));
            tg->Draw("AZL*");
            tg->Fit(f, "RQ", "", 22, 47);
            c->SaveAs(Form("TMeasure/board-%d-bottom-ch%d.jpg", board, vSiPMBottom[board]->GetRealChannel(ch)));

            auto chi2 = f->GetChisquare();
            if (chi2 < 50)
                hCompFactor->Fill(vSiPMBottom[board]->GetTCompFactor(ch));
            double factor = vSiPMBottom[board]->GetTCompFactor(ch);
            if (factor < 50 || factor > 60)
                mapBottom.push_back(std::pair<int, int>(board, ch));

            tg = vSiPMTop[board]->GetTMeasureGraph(tg, ch);
            tg->SetTitle(Form("board-%d-top-ch%d;T/^{#circ}C;ADCValue", board, vSiPMTop[board]->GetRealChannel(ch)));
            tg->Draw("AZL*");
            tg->Fit(f, "RQ", "", 22, 47);
            c->SaveAs(Form("TMeasure/board-%d-top-ch%d.jpg", board, vSiPMTop[board]->GetRealChannel(ch)));

            chi2 = f->GetChisquare();
            if (chi2 < 50)
                hCompFactor->Fill(vSiPMTop[board]->GetTCompFactor(ch));

            factor = vSiPMTop[board]->GetTCompFactor(ch);
            if (factor < 50 || factor > 60)
                mapTop.push_back(std::pair<int, int>(board, ch));
        }
    }

    for (auto iter = mapTop.begin(); iter != mapTop.end(); iter++)
    {
        int board = iter->first;
        int channel = iter->second;
        std::cout << "Top: " << board << '\t' << channel << '\t' << vSiPMTop[board]->GetTCompFactor(channel) << '\t' << vSiPMTop[board]->GetBiasSlope(channel) << '\t' << vSiPMTop[board]->GetTSlope(channel) << std::endl;
    }
    for (auto iter = mapBottom.begin(); iter != mapBottom.end(); iter++)
    {
        int board = iter->first;
        int channel = iter->second;
        std::cout << "Bottom: " << board << '\t' << channel << '\t' << vSiPMBottom[board]->GetTCompFactor(channel) << '\t' << vSiPMBottom[board]->GetBiasSlope(channel) << '\t' << vSiPMBottom[board]->GetTSlope(channel) << std::endl;
    }

    for (int board = 0; board < 11; board++)
    {
        for (int ch = 0; ch < 32; ch++)
        {
            auto hgped = vFEE[board]->GetPed(ch, 20, hg);
            auto lgped = vFEE[board]->GetPed(ch, 20, lg);
            hPed->Fill(hgped, lgped);
            hBias0->Fill(vFEE[board]->GetBias(ch, 0));
            hBias255->Fill(vFEE[board]->GetBias(ch, 255));
            // std::cout << hgped << '\t' << lgped << std::endl;
        }
    }
    hBD->Write();
    hGain->Write();
    hGainCor->Write();
    hCompFactor->Write();
    hPed->Write();
    hBias0->Write();
    hBias255->Write();
    delete file;
#endif

    for (int i = 0; i < 6; i++)
    {
        delete vSiPMBottom[i];
        delete vSiPMTop[i];
    }
    for (int i = 0; i < 11; i++)
    {
        delete vFEE[i];
    }
}
