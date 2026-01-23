#include "serialworker.h"

SerialWorker::SerialWorker(QObject *parent)
    : QObject(parent)
{
    connect(&m_serial, &QSerialPort::readyRead, this, &SerialWorker::handleReadyRead);
}

#include <QDebug>
extern int debugEnabled;
#define DEBUG_OUT if(debugEnabled) qDebug
void SerialWorker::start(const QString &portName)
{
    m_serial.setPortName(portName);
    m_serial.setBaudRate(QSerialPort::Baud9600);
    bool ok = m_serial.open(QIODevice::ReadOnly);
    if (ok) {
        DEBUG_OUT() << "Serial port opened:" << portName;
    } else {
        DEBUG_OUT() << "Failed to open serial port:" << portName << m_serial.errorString();
    }
}

#include <QDebug>
void SerialWorker::handleReadyRead()
{
    QByteArray data = m_serial.readAll();
    DEBUG_OUT() << "Serial received:" << data.toHex();
    m_buffer.append(data);
    while (m_buffer.size() >= 4) {
        if ((quint8)m_buffer[0] == 0xAB && (quint8)m_buffer[1] == 0xBA) {
            int level = (quint8)m_buffer[2];
            int command = (quint8)m_buffer[3];
            bool reverseOrder = (command == 0x01);
            DEBUG_OUT() << "Parsed packet: level=" << level << ", command=" << command;
            emit levelReceived(level, reverseOrder);
            m_buffer.remove(0, 4);
        } else {
            m_buffer.remove(0, 1);
        }
    }
}
