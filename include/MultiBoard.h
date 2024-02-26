#ifndef MULTIBOARD_H
#define MULTIBOARD_H

#include <QMainWindow>
#include <QObject>
#include <QThread>
#include <string>
#include <fstream>
#include <QDateTime>
#include <QTimer>
#include <QLabel>

class MultiBoard;
class FEEControl;
class DataManager;
class ConfigFileParser;
class BoardConnection;

namespace Ui
{
    class MultiBoard;
}

extern const std::vector<int> gBoardScanList;

class SingleBoardJob : public QObject
{
    Q_OBJECT
signals:
    void DAQFinished(int nDAQLoop);
public slots:
    void handle_StartDAQ();

public:
    SingleBoardJob(BoardConnection *connection);

private:
    FEEControl *fBoard;
    DataManager *fDataManager;
    BoardConnection *fConnection;

    bool JudgeLoopFlag(int nEventCount);
};

/// @brief Boards initialization and connection
class BoardConnection : public QObject
{
    Q_OBJECT
    friend class SingleBoardJob;
signals:
    void RequestStartDAQ();
private slots:
    void handle_DAQFinished(int nDAQLoop);

public:
    BoardConnection();
    ~BoardConnection();
    bool InitBoard(int boardNo);
    bool CloseBoard();
    bool IsInitialized() { return fInitilized; }

    void ProcessConnection();
    void ProcessDisconnection();

    FEEControl *GetBoard() { return fBoard; }
    DataManager *GetDataManager() { return fDataManager; }

    bool SetHV(double hv = 57.0);
    bool SetBoardConfig(std::string sRepoFolder = "../FEEBoardGUI-AddDB/");

    bool StartDAQ();
    bool StopDAQ();

    /// @brief Monitor count rate & retrieve time stamp of T0

    bool IsConnected() { return fConnectionStatus; }

    void SetDAQConfig(int nCount = -1, QTime time = QTime(0, 0, 0, 0), int nBufferSleepms = 200, int nBufferLeastEvents = 30, bool bClearQueue = 0);
    void SetFilePathName(std::string sFilePath = "", std::string sFileName = "");

private:
    int fBoardNo = -1;
    bool fInitilized = 0;
    FEEControl *fBoard = nullptr;
    DataManager *fDataManager = nullptr;
    ConfigFileParser *fParser = nullptr;

    std::ofstream fout;

    std::string fsFilePath;
    std::string fsFileName;

    // Threads
    SingleBoardJob *fSingleBoardJob;
    QThread fSingleBoardThread;

    // Board Monitor
    QTimer fBoardMonitorTimer; // Monitor DAQ
    bool RetrieveCount();
    QDateTime sLastTestTime = QDateTime::currentDateTime();
    long sLastLiveCount = -1;
    long sLastRealCount = -1;
    QDateTime testTime = QDateTime::currentDateTime();
    double msToLastTest = (double)sLastTestTime.msecsTo(testTime);
    uint32_t realCount = 0;
    uint32_t liveCount = 0;

    // DAQ
    QDateTime fDAQStartTime; // Record start DAQ time

    int fDAQSettingCount = -1;         // -1 means DAQ forever until stopDAQ clicked
    QTime fDAQSettingTime{0, 0, 0, 0}; // DAQ time setting, 0,0,0,0 means forever until stopDAQ clicked
    int fDAQBufferSleepms = 200;       // FEE control while DAQ, Wait time of buffer reading
    int fDAQBufferLeastEvents = 30;    // FEE control while DAQ, Only when buffer length larger than this, Fifo data will be readout
    bool fFlagClearQueue = 0;          // FEE control before DAQ, give signal whether clear Queue before DAQ
    volatile bool fDAQRuningFlag = 0;  // DAQ runing flag, used to break daq process
    // Used before DAQ
    bool InitDataFile();

    // DAQ monitor
    QTimer fMonitorTimer; // Monitor DAQ
    // Read T0 stamp while DAQ, using timer to control
    uint32_t fCurrentT0ID = 0;
    uint32_t fPreviousT0ID = 0;
    double fTimeStampArray[5]{0};
    double fTimeStampArrayStore[5]{0};
    bool MonitorDAQ();
    bool ReadTimeStamp();
    bool UpdateDAQInfo();

    int fDAQRealTime = 0;                               // monitored real DAQ total time
    int fDAQRealCount = 0;                              // Monitored read DAQ count
    int fDAQRealCountLast = 0;                          // Monitored read DAQ count last time
    QDateTime sLastTime = QDateTime::currentDateTime(); // transit time
    int sLastCount = 0;                                 // transit time
    double sTransCR = 0;                                // transit time
    int sTransTimeInteval = 0;                          // transit time
    int transCount = 0;                                 // transit time

    // Connection
    volatile bool fConnectionStatus = 0; // Connection status
};

// Work threads
class MultiBoardJob : public QObject
{
    Q_OBJECT
public:
    ~MultiBoardJob() = default;
signals:
    void UpdateBoardStatus(int boardNo, bool status);
    void BoardsScanned();

public slots:
    void ScanBoards();

private:
    MultiBoard *fMultiBoardWin = NULL;
};

class MultiBoard : public QMainWindow
{
    Q_OBJECT

public:
    explicit MultiBoard(QWidget *parent = nullptr);
    ~MultiBoard();

private:
    Ui::MultiBoard *ui;
    MultiBoardJob *fMultiBoardJob = nullptr;
    QThread fMultiBoardThread;

    // Scan boards
    void ScanBoards();
    void UpdateLists();
    void ClearLists();
    std::map<int, bool> fBoardStatus;
    std::map<int, BoardConnection *> fBoardConnections;

    // Boards Status
    QTimer fBoardStatusTimer;
    void ProcessConnection();
    void ProcessDisconnection();
    void UpdateStatus();

    // Used for update DAQ status
    std::map<int, QLabel *> flblBoardNo;
    std::map<int, QLabel *> flblConnectionStatus;
    std::map<int, QLabel *> flblRealCount;
    std::map<int, QLabel *> flblLiveCount;
    std::map<int, QLabel *> flblDAQCount;
    std::map<int, QLabel *> flblHGCount;
    std::map<int, QLabel *> flblLGCount;
    std::map<int, QLabel *> flblTDCCount;

signals:
    void StartScanBoards(); // Start scanning boards

private slots:
    void handle_BoardStatus(int boardNo, bool status); // Update board status
    void handle_BoardsScanned();                       // Update board status
    void on_btnScanBoards_clicked();
};

#endif // MULTIBOARD_H
