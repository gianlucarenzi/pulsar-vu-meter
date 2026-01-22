#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "demomode.h"
#include "serialworker.h"
#include "vumeterwidget.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_demo = nullptr;
}

void MainWindow::startDemo(bool reverseOrder) {
    m_reverseOrder = reverseOrder;
    if (!m_demo) {
        m_demo = new DemoMode(this, [this](int value, bool reverseOrder) {
            ui->vumeterWidget->setLevel(value, reverseOrder);
        });
    }
    m_demo->start(m_reverseOrder);
}

void MainWindow::startSerial(const QString &portName)
{
    if (!m_serialWorker) {
        m_serialWorker = new SerialWorker(this);
        connect(m_serialWorker, &SerialWorker::levelReceived, this, [this](int level, bool reverseOrder) {
            m_reverseOrder = reverseOrder;
            ui->vumeterWidget->setLevel(level, m_reverseOrder);
        });
    }
    m_serialWorker->start(portName);
}

MainWindow::~MainWindow()
{
    delete ui;
}
