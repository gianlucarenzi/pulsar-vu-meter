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
    VuMeterWidget::LedColorMode colorMode = VuMeterWidget::LedColorSegments;
    int customR = 0, customG = 0, customB = 0;
    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        if (arg == "--demo" || arg == "demo") demoMode = true;
        else if (arg == "--reverse" || arg == "-r") reverseOrder = true;
        else if (arg == "--green") colorMode = VuMeterWidget::LedColorGreen;
        else if (arg == "--yellow") colorMode = VuMeterWidget::LedColorYellow;
        else if (arg == "--orange") colorMode = VuMeterWidget::LedColorOrange;
        else if (arg == "--red") colorMode = VuMeterWidget::LedColorRed;
        else if (arg == "--custom" && i+3 < argc) {
            colorMode = VuMeterWidget::LedColorCustom;
            customR = QString(argv[++i]).toInt();
            customG = QString(argv[++i]).toInt();
            customB = QString(argv[++i]).toInt();
        }
        else port = arg;
    }
    w.findChild<VuMeterWidget*>()->setLedColorMode(colorMode);
    if (colorMode == VuMeterWidget::LedColorCustom)
        w.findChild<VuMeterWidget*>()->setCustomColor(customR, customG, customB);
    if (demoMode) {
        w.startDemo(reverseOrder);
    } else {
        w.startSerial(port);
    }
    return a.exec();
}
