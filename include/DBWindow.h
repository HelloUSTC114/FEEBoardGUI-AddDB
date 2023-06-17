#ifndef DBWINDOW_H
#define DBWINDOW_H

#include <QMainWindow>
#include <QString>

namespace Ui
{
    class DBWindow;
}

#define gDBWin (DBWindow::Instance())
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

private:
    explicit DBWindow(QWidget *parent = nullptr);
    Ui::DBWindow *ui;

    QString fsFilePath;
    QString fsFileName;
    bool fFileNameIsInput = 0;
};

#endif // DBWINDOW_H
