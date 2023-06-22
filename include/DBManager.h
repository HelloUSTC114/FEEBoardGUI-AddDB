#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QProcess>
#include <vector>
#include <map>

#define gDBManager (DBManager::Instance())
class TGraphErrors;

enum GAINTYPE
{
    hg = 0,
    lg
};

enum SIPMBOARDTYPE
{
    bottom,
    top
};

struct AMPInfo
{
    int ampSet;
    double hgPedMean[32];
    double hgPedStdDev[32];
    double lgPedMean[32]; //  pedestal under different amp dac
    double lgPedStdDev[32];
    double hgCali[32];
    double lgCali[32]; // <amp dac, Amplitude change while TEK change 1mV>
};

struct BiasInfo
{
    int biasSet;
    double vBiasValue[32];
};

class BoardTestResult;
class SiPMTestResult;

class DBManager
{
public:
    QSqlDatabase &GetDB() { return fDataBase; }
    static DBManager *Instance();

    bool OpenDB(QString sDBName);
    void CloseDB();

    bool DeleteFromTable(QString sTableName, int chID, int boardNo);

    /// @brief Insert Channel info in database
    /// @param boardNo boardno for basic info
    /// @param values values for 32 ch
    /// @param valids whether this channel is valid
    /// @return ID for the channel info
    int InsertChannelInfo(int boardNo, double *values, bool *valids, QString sComment = "");
    int ReadChannelInfo(int chID, int &boardNo, double *values, bool *valids);
    bool DeleteChannelInfo(int chID, int boardNo);

    /// @brief Insert Temperature test info in database
    /// @param boardNo
    /// @param bt board type
    /// @param nTemperatureTestPoints how many temperature points has been tested
    /// @param tempEntry channel info entry for temperature point, (only 3 useful information)
    /// @param valueEntry channel info entry for gain test
    /// @return
    int InsertTemperatureTestInfo(int boardNo, SIPMBOARDTYPE bt, int nTemperatureTestPoints, int *tempEntry, int *valueEntry);
    int ReadTemperatureTestInfo(int tempID, int &boardNo, SIPMBOARDTYPE &bt, int &nTemperatureTestPoints, int *tempEntry, int *valueEntry);
    bool DeleteTemperatureTestInfo(int tempID, int boardNo);

    /// @brief
    /// @param boardNo
    /// @param bt
    /// @param tempTableEntry entry id of temperature test table
    /// @param biasTableEntry entry id of bias test table
    /// @param TSlopeEntry entry id of T measure result
    /// @param BiasSlopeEntry entry id of bias measure result
    /// @param BDVEntry breakdown voltage measure result
    /// @param BDTEntry under which temperature BD is measured
    /// @param TCompFactorEntry compensation result
    /// @return
    int InsertSiPMBoardTestInfo(int boardNo, SIPMBOARDTYPE bt, int tempTableEntry, int biasTableEntry, int TSlopeEntry, int BiasSlopeEntry, int BDVEntry, int BDTEntry, int TCompFactorEntry);
    int ReadSiPMBoardTestInfo(int boardID, int &boardNo, SIPMBOARDTYPE &bt, int &tempTableEntry, int &biasTableEntry, int &TSlopeEntry, int &BiasSlopeEntry, int &BDVEntry, int &BDTEntry, int &TCompFactorEntry);
    int ReadSiPMBoardTestInfo(int boardNo, SIPMBOARDTYPE bt, int &tempTableEntry, int &biasTableEntry, int &TSlopeEntry, int &BiasSlopeEntry, int &BDVEntry, int &BDTEntry, int &TCompFactorEntry); // Read through board number and board type
    bool DeleteSiPMBoardTestInfo(int boardID, int boardNo);

    /// @brief Insert entry info into table bias
    /// @param boardNo
    /// @param biasSet
    /// @param biasChEntry
    /// @return
    int InsertBiasEntryInfo(int boardNo, int biasSet, int biasChEntry);
    int ReadBiasEntryInfo(int biasID, int &boardNo, int &biasSet, int &biasChEntry);
    bool DeleteBiasEntryInfo(int biasID, int boardNo);

    /// @brief Insert entry info into table amp
    /// @param boardNo
    /// @param ampSet
    /// @param hgPedChEntry
    /// @param hgPedStdChEntry
    /// @param lgPedChEntry
    /// @param lgPedStdChEntry
    /// @param hgCaliChEntry
    /// @param lgCaliChEntry
    /// @return
    int InsertAmpEntryInfo(int boardNo, int ampSet, int hgPedChEntry, int hgPedStdChEntry, int lgPedChEntry, int lgPedStdChEntry, int hgCaliChEntry, int lgCaliChEntry);
    int ReadAmpEntryInfo(int ampID, int &boardNo, int &ampSet, int &hgPedChEntry, int &hgPedStdChEntry, int &lgPedChEntry, int &lgPedStdChEntry, int &hgCaliChEntry, int &lgCaliChEntry);
    bool DeleteAmpEntryInfo(int ampID, int boardNo);

    /// @brief Insert amp table
    /// @param boardNo
    /// @param ampEntries value array for the channel
    /// @param validation whether is valid for channel
    /// @return
    int InsertAmpTable(int boardNo, int *ampEntries, bool *validation);
    int ReadAmpTable(int ampTableID, int &boardNo, int *ampEntries, bool *validation);
    bool DeleteAmpTable(int ampTableID, int boardNo);

    /// @brief Insert bias table
    /// @param boardNo
    /// @param biasEntries
    /// @param validation
    /// @return
    int InsertBiasTableEntry(int boardNo, int *biasEntries, bool *validation);
    int ReadBiasTableEntry(int biasTableID, int &boardNo, int *biasEntries, bool *validation);
    bool DeleteBiasTableEntry(int biasTableID, int boardNo);

    int WriteBiasTestEntryIntoDB(const std::map<int, BiasInfo> &mapTable, int boardNo, bool *chValid);
    bool ReadBiasTestEntryFromDB(int biasTableEntry, std::map<int, BiasInfo> &biasMap);

    /// @brief
    /// @param boardNo
    /// @param ampTableEntry
    /// @param biasTableEntry
    /// @return
    int InsertFEEBoardEntry(int boardNo, int ampTableEntry, int biasTableEntry);
    int ReadFEEBoardEntry(int feeBoardID, int &boardNo, int &ampTableEntry, int &biasTableEntry);
    int ReadFEEBoardEntry(int boardNo, int &ampTableEntry, int &biasTableEntry); // Read through board number
    bool DeleteFEEBoardEntry(int feeBoardID, int boardNo);

    /// @brief
    /// @param
    /// @return number of fee board entries
    int ReadFEEBoardLists(std::vector<int> &);
    int ReadSiPMBoardLists(std::vector<std::pair<int, SIPMBOARDTYPE>> &);

    std::vector<int> GetFEEBoardLists() { return fvFEEBoardNo; };
    std::vector<std::pair<int, SIPMBOARDTYPE>> GetSiPMBoardLists() { return fvSiPMBoardNo; };

    void Init(QString sDBName = "CalibrationDB.db");
    bool IsInitiated() { return fInitiated; }

    void ReadFromDB(QString sDBName = "CalibrationDB.db");

private:
    DBManager();
    ~DBManager();

    bool fInitiated = 0;

    QString fsDBFile;
    QSqlDatabase fDataBase;
    QSqlQuery fDBQuery;
    QProcess fProcess;

    std::vector<int> fvFEEBoardNo;                            // map<boardNo, board>
    std::vector<std::pair<int, SIPMBOARDTYPE>> fvSiPMBoardNo; // vector<boardNo, boardtype>
};

class BoardTestResult
{
public:
    bool IsValid() { return fIsValid; };
    static double CalcAmpMultiFactor(int ampDAC, GAINTYPE hl);
    static bool ReadAmpCaliFile(int board, GAINTYPE hl);
    static bool ReadPedMeanFile(int board, GAINTYPE hl);
    static bool ReadBiasDACTestFile(int board);

    bool GenerateFromSource(int board, std::string sDepoPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\");
    int WriteIntoDB();
    bool ReadFromDB(int board);
    void Dump(std::ostream &os);

    double GetCaliResult(int ch, int ampDAC, GAINTYPE hl);
    double GetPed(int ch, int ampDAC, GAINTYPE hl);
    double GetBias(int ch, int biasDAC) { return fBiasTable[biasDAC].vBiasValue[ch]; };
    TGraphErrors *GetPedStdDevGraph(TGraphErrors *tge, int ch, GAINTYPE hl);
    TGraphErrors *GetAmpCaliGraph(TGraphErrors *tge, int ch, GAINTYPE hl);

private:
    int fBoardNo;
    bool fIsValid = 0;
    bool fGeneratedFromSource = 0;
    bool fGeneratedFromDB = 0;

    std::string fsDepoPath;
    std::string fsCaliPath;
    std::string fsPedPath;
    std::string fsBiasPath;

    std::map<int, AMPInfo> fAMPTable;
    std::map<int, BiasInfo> fBiasTable;

    int WriteBiasTestEntry();
    int WriteAmpTestEntry();
    bool ReadBiasTestEntry(int biasTestEntry);
    bool ReadAmpTestEntry(int ampTestEntry);

    void GenerateAMPCali();
    void GeneratePed();
    void GeneratePedDev();
    void GenerateBias();

    void InitBoard(int boardNo, std::string sDepoPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\");
};

class SiPMTestResult
{
public:
    bool IsValid() { return fIsValid; };
    bool GenerateFromSiPMTestFile(int board, SIPMBOARDTYPE bt);
    int WriteIntoDB();
    bool ReadFromDB(int board, SIPMBOARDTYPE bt);
    bool IsDetailed() { return fGeneratedFromSource || fDBIsDetailed; }
    void Dump(std::ostream &os);

    double *GetBDVoltageT() { return fBDTemperature; };
    double *GetBDVoltageV() { return fBDVoltage; };
    double GetBDVoltageT(int ch) { return fBDTemperature[ch]; };
    double GetBDVoltageV(int ch) { return fBDVoltage[ch]; };

    double GetTCompFactor(int ch) { return fTCompFactor[ch]; };
    int GetRealChannel(int ch) { return ch + fChannelOffset; };
    double GetTSlope(int ch) { return fTSlope[ch]; };
    double GetBiasSlope(int ch) { return fBiasSlope[ch]; };

    double GetRealBDVoltageT(int chReal);
    double GetRealBDVoltageV(int chReal);
    double GetRealTCompFactor(int chReal);
    double GetRealTSlope(int chReal);
    double GetRealBiasSlope(int chReal);

    TGraphErrors *GetTMeasureGraph(TGraphErrors *tge, int ch);
    TGraphErrors *GetVMeasureGraph(TGraphErrors *tge, int ch);

private:
    int fBoardNo;
    SIPMBOARDTYPE fBT;
    bool fIsValid = 0;
    int fChannelOffset = 0;
    bool fGeneratedFromSource = 0;
    bool fGeneratedFromDB = 0;
    bool fDBIsDetailed = 0;

    std::string fsPath;
    std::string fsFileName;
    std::string fsBT;

    int fNTMeasPoints = 0;         // Row count of TMeasure
    double fTMeas[3][10];          // TMeasure [group][row]
    double fTMeasResult[3][8][10]; // Measure result [group][ch][row]

    double fTSlope[24];        // fit k for Gain-T curve
    double fBiasSlope[24];     // slope fo Gain-V curve
    double fBDVoltage[24];     // Break down voltagem, measured by Gain-V curve
    double fBDTemperature[24]; // Temperature under which BD voltage measured
    double fTCompFactor[24];   // fit k for T-Bias curve, in Unit of mV/C

    int fNbiasSetPoints = 0;
    // std::map<int, double[24]> fBiasSetMap;
    std::map<int, BiasInfo> fBiasSetMap;
    // int fBiasSet[256];
    // double fValue[24][256]; //  measured Gain,[ch][row]

    // Write into Database
    int WriteTempEntry();
    int WriteBiasTestEntry();
    bool ReadTempEntry(int tempTableEntry);
    bool ReadBiasTestEntry(int biasTableEntry);

    void InitBoardNo(int boardNo, SIPMBOARDTYPE bt, std::string spath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\~SiPM~SiPMTest\\3\\");

    static std::stringstream gss;
};

void GenerateDBFromSource();

#endif