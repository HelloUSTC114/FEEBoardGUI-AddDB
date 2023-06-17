#include "DBWindow.h"
#include "ui_DBWindow.h"
#include "FEEControlWidget.h"

DBWindow::DBWindow(QWidget *parent) : QMainWindow(parent),
                                      ui(new Ui::DBWindow)
{
    ui->setupUi(this);
}

DBWindow::~DBWindow()
{
    delete ui;
}

DBWindow *DBWindow::Instance()
{
    static auto ins = new DBWindow(gFEEControlWin);
    return ins;
}

#include <QFileDialog>
#include "DBManager.h"
void DBWindow::on_btnDBFile_clicked()
{
    QString temp;
    if (fsFileName != "")
        temp = QFileDialog::getOpenFileName(this, "Choose Database", fsFileName, "*.db");
    else
        temp = QFileDialog::getOpenFileName(this, "Choose Database", "", "*.db");

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
}

void DBWindow::on_btnCloseDB_clicked()
{
    gDBManager->CloseDB();
    ui->lblDBLED->setStyleSheet("background-color:rgb(190,190,190)");
}
