#include "serialworker.h"

SerialWorker::SerialWorker(QObject *parent)
    : QObject(parent)
{
    connect(&m_serial, &QSerialPort::readyRead, this, &SerialWorker::handleReadyRead);
}

#include <QDebug>
void SerialWorker::start(const QString &portName)
{
    m_serial.setPortName(portName);
    m_serial.setBaudRate(QSerialPort::Baud9600);
    bool ok = m_serial.open(QIODevice::ReadOnly);
    if (ok) {
        qDebug() << "Serial port opened:" << portName;
    } else {
        qDebug() << "Failed to open serial port:" << portName << m_serial.errorString();
    }
}

#include <QDebug>
void SerialWorker::handleReadyRead()
{
    QByteArray data = m_serial.readAll();
    qDebug() << "Serial received:" << data.toHex();
    m_buffer.append(data);
    while (m_buffer.size() >= 4) {
        if ((quint8)m_buffer[0] == 0xAB && (quint8)m_buffer[1] == 0xBA) {
            int level = (quint8)m_buffer[2];
            int command = (quint8)m_buffer[3];
            bool reverseOrder = (command == 0x01);
            qDebug() << "Parsed packet: level=" << level << ", command=" << command;
            emit levelReceived(level, reverseOrder);
            m_buffer.remove(0, 4);
        } else {
            m_buffer.remove(0, 1);
        }
    }
}
