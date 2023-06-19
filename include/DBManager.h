#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QProcess>
#include <vector>
#include <map>

#define gDBManager (DBManager::Instance())

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
    int InsertChannelInfo(int boardNo, double *values, bool *valids);
    bool DeleteChannelInfo(int chID, int boardNo);

    /// @brief Insert Temperature test info in database
    /// @param boardNo
    /// @param bt board type
    /// @param nTemperatureTestPoints how many temperature points has been tested
    /// @param tempEntry channel info entry for temperature point, (only 3 useful information)
    /// @param valueEntry channel info entry for gain test
    /// @return
    int InsertTemperatureTestInfo(int boardNo, SIPMBOARDTYPE bt, int nTemperatureTestPoints, int *tempEntry, int *valueEntry);
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
    bool DeleteSiPMBoardTestInfo(int tempID, int boardNo);

    /// @brief Insert entry info into table bias
    /// @param boardNo
    /// @param biasSet
    /// @param biasChEntry
    /// @return
    int InsertBiasEntryInfo(int boardNo, int biasSet, int biasChEntry);
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
    bool DeleteAmpEntryInfo(int ampID, int boardNo);

    /// @brief Insert amp table
    /// @param boardNo
    /// @param ampEntries value array for the channel
    /// @param validation whether is valid for channel
    /// @return
    int InsertAmpTable(int boardNo, int *ampEntries, bool *validation);
    bool DeleteAmpTable(int ampTableID, int boardNo);

    /// @brief Insert bias table
    /// @param boardNo
    /// @param biasEntries
    /// @param validation
    /// @return
    int InsertBiasTableEntry(int boardNo, int *biasEntries, bool *validation);
    bool DeleteBiasTableEntry(int biasTableID, int boardNo);

    /// @brief 
    /// @param boardNo 
    /// @param ampTableEntry 
    /// @param biasTableEntry 
    /// @return 
    int InsertFEEBoardEntry(int boardNo, int ampTableEntry, int biasTableEntry);
    bool DeleteFEEBoardEntry(int biasTableID, int boardNo);

    void Init(QString sDBName = "CalibrationDB.db");

private:
    DBManager();
    ~DBManager();

    bool fInitiated = 0;

    QSqlDatabase fDataBase;
    QSqlQuery fDBQuery;
    QProcess fProcess;
};

class BoardTestResult
{
public:
    static double CalcAmpMultiFactor(int ampDAC, GAINTYPE hl);
    static bool ReadAmpCaliFile(int board, GAINTYPE hl);
    static bool ReadPedMeanFile(int board, GAINTYPE hl);
    static bool ReadBiasDACTestFile(int board);

    bool GenerateFromSource(int board, std::string sDepoPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\");
    void Dump(std::ostream &os);

    double GetCaliResult(int ch, int ampDAC, GAINTYPE hl);
    double GetPed(int ch, int ampDAC, GAINTYPE hl);
    double GetBias(int ch, int biasDAC) { return fBiasTable[biasDAC].vBiasValue[ch]; };

    int WriteIntoDB();

private:
    int fBoardNo;
    bool fIsValid = 0;

    std::string fsDepoPath;
    std::string fsCaliPath;
    std::string fsPedPath;
    std::string fsBiasPath;

    std::map<int, AMPInfo> fAMPTable;
    std::map<int, BiasInfo> fBiasTable;

    int WriteBiasTestEntry();
    int WriteAmpTestEntry();

    void GenerateAMPCali();
    void GeneratePed();
    void GeneratePedDev();
    void GenerateBias();
};

class TGraphErrors;
class SiPMTestResult
{
public:
    bool GenerateFromSiPMTestFile(int board, SIPMBOARDTYPE bt);
    void Dump(std::ostream &os);

    double *GetBDVoltageT() { return fBDTemperature; };
    double *GetBDVoltageV() { return fBDVoltage; };
    double GetBDVoltageT(int ch) { return fBDTemperature[ch]; };
    double GetBDVoltageV(int ch) { return fBDVoltage[ch]; };

    double GetTCompFactor(int ch) { return fTCompFactor[ch]; };
    int GetRealChannel(int ch) { return ch + fChannelOffset; };
    double GetBiasSlope(int ch) { return fBiasSlope[ch]; };

    TGraphErrors *GetTMeasureGraph(TGraphErrors *tge, int ch);

    int WriteIntoDB();

private:
    int fBoardNo;
    SIPMBOARDTYPE fBT;
    bool fIsValid = 0;
    int fChannelOffset = 0;

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
    std::map<int, double[24]> fBiasSetMap;
    // int fBiasSet[256];
    // double fValue[24][256]; //  measured Gain,[ch][row]

    // Write into Database
    int WriteTempEntry();
    int WriteBiasTestEntry();

    static std::stringstream gss;
};

void GenerateDBFromSource();

#endif