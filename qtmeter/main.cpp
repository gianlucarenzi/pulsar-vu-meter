#include "mainwindow.h"
#include <QApplication>
#include "vumeterwidget.h"
#include <QThread>
#include <QDebug>
#include <QDateTime>

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
        else if (arg == "--color-mode" && i+1 < argc) {
            QString val = argv[++i];
            if (val == "green") colorMode = VuMeterWidget::LedColorGreen;
            else if (val == "yellow") colorMode = VuMeterWidget::LedColorYellow;
            else if (val == "orange") colorMode = VuMeterWidget::LedColorOrange;
            else if (val == "red") colorMode = VuMeterWidget::LedColorRed;
            else if (val == "segments") colorMode = VuMeterWidget::LedColorSegments;
        }
        else if (arg == "--color" && i+1 < argc) {
            QString val = argv[++i];
            if (colorMode == VuMeterWidget::LedColorCustom) {
                // handled above
            } else if (val == "green") colorMode = VuMeterWidget::LedColorGreen;
            else if (val == "yellow") colorMode = VuMeterWidget::LedColorYellow;
            else if (val == "orange") colorMode = VuMeterWidget::LedColorOrange;
            else if (val == "red") colorMode = VuMeterWidget::LedColorRed;
            else if (val == "segments") colorMode = VuMeterWidget::LedColorSegments;
        }
        else if (!arg.startsWith("--") && !arg.endsWith("green") && !arg.endsWith("yellow") && !arg.endsWith("orange") && !arg.endsWith("red") && !arg.endsWith("segments")) {
            port = arg;
        }
    }
    w.findChild<VuMeterWidget*>()->setLedColorMode(colorMode);
    if (colorMode == VuMeterWidget::LedColorCustom)
        w.findChild<VuMeterWidget*>()->setCustomColor(customR, customG, customB);
    // --- STARTUP ANIMATION ---
    VuMeterWidget* meter = w.findChild<VuMeterWidget*>();
    if (meter) {
        // 1. Scorrimento avanti (bianco)
        // 1. Scorrimento avanti (bianco) in 1 secondo
        for (int i = 0; i < 12; i++) {
            meter->setLedColorMode(VuMeterWidget::LedColorCustom);
            meter->setCustomColor(255,255,255);
            meter->setLevel(i+1);
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "STATO: sweep avanti, led accesi:" << i+1 << "colore: (255,255,255)";
            a.processEvents();
            QThread::msleep(1000/12);
        }
        // 2. Scorrimento indietro (bianco) in 1 secondo
        for (int i = 11; i >= 0; i--) {
            meter->setLedColorMode(VuMeterWidget::LedColorCustom);
            meter->setCustomColor(255,255,255);
            meter->setLevel(i+1);
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "STATO: sweep indietro, led accesi:" << i+1 << "colore: (255,255,255)";
            a.processEvents();
            QThread::msleep(1000/12);
        }
        // 3. Accensione di tutti i LED bianchi
        meter->setLevel(12);
        meter->setCustomColor(255,255,255);
        meter->setLedColorMode(VuMeterWidget::LedColorCustom);
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "STATO: tutti bianchi, led accesi: 12 colore: (255,255,255)";
        a.processEvents();
        QThread::msleep(500);
        // 4. Tutti rossi
        meter->setCustomColor(255,0,0);
        meter->setLedColorMode(VuMeterWidget::LedColorCustom);
        meter->setLevel(12);
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "STATO: tutti rossi, led accesi: 12 colore: (255,0,0)";
        a.processEvents();
        QThread::msleep(300);
        // 5. Fade rosso -> arancio (barra piena)
        for (int step = 0; step <= 40; step++) {
            int r = 255, g = step*6, b = 0;
            meter->setCustomColor(r, g, b);
            meter->setLedColorMode(VuMeterWidget::LedColorCustom);
            meter->setLevel(12);
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "STATO: fade rosso->arancio, led accesi: 12 colore: (" << r << "," << g << "," << b << ")";
            a.processEvents();
            QThread::msleep(40);
        }
        // 6. Fade arancio -> giallo (barra piena)
        for (int step = 0; step <= 40; step++) {
            int r = 255-step*6, g = 100+step*4, b = 0;
            meter->setCustomColor(r, g, b);
            meter->setLedColorMode(VuMeterWidget::LedColorCustom);
            meter->setLevel(12);
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "STATO: fade arancio->giallo, led accesi: 12 colore: (" << r << "," << g << "," << b << ")";
            a.processEvents();
            QThread::msleep(40);
        }
        // 7. Fade giallo -> verde (barra piena)
        for (int step = 0; step <= 40; step++) {
            int r = 0, g = 200-step*5, b = 0;
            meter->setCustomColor(r, g<0?0:g, b);
            meter->setLedColorMode(VuMeterWidget::LedColorCustom);
            meter->setLevel(12);
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "STATO: fade giallo->verde, led accesi: 12 colore: (" << r << "," << (g<0?0:g) << "," << b << ")";
            a.processEvents();
            QThread::msleep(40);
        }
        // 8. Spegnimento
        meter->setLevel(0);
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "STATO: spento, led accesi: 0 colore: (0,0,0)";
        a.processEvents();
        QThread::msleep(100);
    }
    if (demoMode) {
        w.startDemo(reverseOrder);
    } else {
        w.startSerial(port);
    }
    return a.exec();
}
