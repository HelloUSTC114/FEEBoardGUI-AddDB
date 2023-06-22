#ifndef DBWINDOW_H
#define DBWINDOW_H

#include <QMainWindow>
#include <QString>

#include <vector>
#include <map>
#include <list>

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
    SiPMBoardInfo(int boardNo, SIPMBOARDTYPE bt) : fBoardNo(boardNo), fBT(bt){};
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

private slots:
    void on_btnDBFile_clicked();

    void on_btnOpenDB_clicked();

    void on_btnCloseDB_clicked();

    void on_btnBind_clicked();

    void on_btnUnbind_clicked();


    void on_btnShow_clicked();

private:
    explicit DBWindow(QWidget *parent = nullptr);
    Ui::DBWindow *ui;

    QString fsFilePath;
    QString fsFileName;
    bool fFileNameIsInput = 0;

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
};

#endif // DBWINDOW_H
