#ifndef DBWINDOW_H
#define DBWINDOW_H

#include <QMainWindow>
#include <QString>

#include <vector>
#include <map>
#include <list>
#include <array>

#include "DBManager.h"

namespace Ui
{
    class DBWindow;
}

#define gDBWin (DBWindow::Instance())
struct SiPMBoardInfo
{
    int fBoardNo;
    SIPMBOARDTYPE fBT;
    SiPMBoardInfo(int boardNo = -1, SIPMBOARDTYPE bt = undefined) : fBoardNo(boardNo), fBT(bt){};
};
bool operator==(const SiPMBoardInfo &a, const SiPMBoardInfo &b);
bool operator<(const SiPMBoardInfo &a, const SiPMBoardInfo &b);

class QLabel;
class QComboBox;
class QSpinBox;
class DBWindow : public QMainWindow
{
    Q_OBJECT

public:
    ~DBWindow();
    static DBWindow *Instance();

    bool FileNameIsValid() { return fFileNameIsInput; }
    void SetHVBias(double hv) { fCurrentHV = hv; }
    std::vector<std::pair<int, int>> GetCurrentCompBias(double temp0, double temp1, double temp2, double temp3); // Set bias = 200 at 25C
    std::vector<std::pair<int, int>> GetCompBias(int feeBoardNo, double temp0, double temp1, double temp2, double temp3); // Set bias = 200 at 25C


private slots:
    void on_btnDBFile_clicked();

    void on_btnOpenDB_clicked();

    void on_btnCloseDB_clicked();

    void on_btnBind_clicked();

    void on_btnUnbind_clicked();

    void on_btnShow_clicked();

    void on_btnNewTComp_clicked();

    void on_btnCloseTComp_clicked();

    void on_btnTempComp_clicked();

private:
    explicit DBWindow(QWidget *parent = nullptr);
    Ui::DBWindow *ui;

    QString fsFilePath;
    QString fsFileName = "F:\\Projects\\FEEDistri\\DataBase\\Calibration.db";
    bool fFileNameIsInput = 1;

    void GetBoardListFromDB();
    void ClearList();
    void UpdateListUI();
    std::list<int> vFEEBoardList;
    std::list<SiPMBoardInfo> vSiPMBoardList;
    std::list<std::pair<int, SiPMBoardInfo>> vPairedInfo;

    void Group2Boards(const int &fee, const SiPMBoardInfo &);
    void Ungroup2Boards(const std::pair<int, SiPMBoardInfo> &);

    static QString ConvertToString(std::pair<int, SiPMBoardInfo>);
    static QString ConvertToString(SiPMBoardInfo);
    static QString ConvertToString(int);

    static std::pair<int, SiPMBoardInfo> ConvertStringToPair(QString);
    static SiPMBoardInfo ConvertStringToSiPM(QString);
    static int ConvertStringToFEE(QString);

    // Channels information
    void ReadChanInfo(int ch);
    QLabel *flblChNo[32];
    QSpinBox *fcombAmp[32];
    QLabel *flblAmp[32];
    QSpinBox *fcombBias[32];
    QLabel *flblBias[32];
    QLabel *flblSiPMTCom[32];
    QLabel *flblSiPMBD[32];
    QLabel *flblSiPMOV[32];

    SiPMTestResult *sipmRes;
    BoardTestResult *feeRes;
    bool fIsRead = 0;

    // Another File for SiPM Info
    bool fOtherFileFlag = 0;
    std::map<SiPMBoardInfo, std::array<double, 32>> fTCompMap;
    std::map<SiPMBoardInfo, std::array<double, 32>> fBDMap;
    bool ReadTCompFile(std::string sInputFile);
    void ProcessTComp();

    // Setting Bias Compensation
    double fCurrentHV = 56;
    std::pair<int, SiPMBoardInfo> fCurrentPair; // Showing Current pair <FEE Board, SiPM Board>
    std::array<double, 32> fCurrentBias;        // FEE Board Info
    std::array<double, 32> fCurrentTComp;       // SiPM Board Info
    std::array<double, 32> fCurrentBD;          // SiPM Board Info

    // Bias Compensation Calculating
    int GetCompBias(double Temp, int ch);                                                           // Set bias = 200 at 25C
    std::array<double, 32> &GetAllCompBias(const std::array<double, 32> &temp);                     // Set bias = 200 at 25C
    std::array<double, 32> &GetAllCompBias(double temp0, double temp1, double temp2, double temp3); // Set bias = 200 at 25C
};

#endif // DBWINDOW_H
