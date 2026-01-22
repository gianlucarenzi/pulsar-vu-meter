#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serialworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include "demomode.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void startDemo(bool reverseOrder = false);

public:
    void startSerial(const QString &portName);
private:
    Ui::MainWindow *ui;
    SerialWorker *m_serialWorker = nullptr;
    DemoMode *m_demo = nullptr;
    bool m_reverseOrder = false;
};

#endif // MAINWINDOW_H
