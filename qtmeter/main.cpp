#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    bool demoMode = false;
    bool reverseOrder = false;
    QString port = "/dev/ttyACM0";
    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        if (arg == "--demo" || arg == "demo") demoMode = true;
        else if (arg == "--reverse" || arg == "-r") reverseOrder = true;
        else port = arg;
    }
    if (demoMode) {
        w.startDemo(reverseOrder);
    } else {
        w.startSerial(port);
    }
    return a.exec();
}
