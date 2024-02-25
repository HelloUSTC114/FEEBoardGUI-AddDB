#include "feecontrol.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <QApplication>
#include "MultiBoard.h"

int main_waste(int argc, char *argv[])
{
    int flag = 1;
    if (argc >= 2)
    {
        flag = std::stoi(argv[1]);
    }
    if (flag != 0)
        flag = 1;
    std::cout << "Flag: " << flag << std::endl;

    // Enable 1s clock in board 0
    int boardNo = 0;
    gBoard->InitPort(boardNo);
    auto rtn = gBoard->TestConnect();
    gBoard->write_reg_test(55, flag);

    // Enable TDC
    for (boardNo = 0; boardNo < 8; boardNo++)
    {
        gBoard->InitPort(boardNo);
        rtn = gBoard->TestConnect();
        rtn = gBoard->enable_tdc(flag);
        std::cout << std::setprecision(5) << "Board No: " << boardNo << '\t' << "Enable TDC: " << flag << "\tSuccess: " << rtn << std::endl;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MultiBoard w;
    w.show();
    a.exec();
    return 0;
}