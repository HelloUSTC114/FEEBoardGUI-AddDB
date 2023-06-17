#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtSql/QSqlDatabase>
#include <QString>
#include <vector>
#include <map>

#define gDBManager (DBManager::Instance())

class DBManager
{
public:
    QSqlDatabase &GetDB() { return fDataBase; }
    static DBManager *Instance();

    bool OpenDB(QString sDBName);
    void CloseDB();

private:
    DBManager();
    ~DBManager();

    bool fInitiated = 0;
    void Init(QString sDBName = "CalibrationDB.db");

    QSqlDatabase fDataBase;
};

enum GAINTYPE
{
    hg = 0,
    lg
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

class BoardTestResult
{
public:
    static double CalcAmpMultiFactor(int ampDAC, GAINTYPE hl);
    static bool ReadAmpCaliFile(int board, GAINTYPE hl);
    static bool ReadPedMeanFile(int board, GAINTYPE hl);
    static bool ReadBiasDACTestFile(int board);

    bool GenerateFromSource(int board, std::string sDepoPath = "E:\\Data\\~CALI~Calibration-Result\\ProducedForSQLite\\");
    void Dump(std::ostream &os);

private:
    int fBoardNo;
    bool fValid = 0;

    std::string fsDepoPath;
    std::string fsCaliPath;
    std::string fsPedPath;
    std::string fsBiasPath;

    std::map<int, AMPInfo> fAMPTable;
    std::map<int, BiasInfo> fBiasTable;

    void GenerateAMPCali();
    void GeneratePed();
    void GeneratePedDev();
    void GenerateBias();
};

enum SIPMBOARDTYPE
{
    bottom,
    top
};

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
    int fBiasSet[256];
    double fValue[24][256]; //  measured Gain,[ch][row]

    static std::stringstream gss;
};

void GenerateDBFromSource();


#endif