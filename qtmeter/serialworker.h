#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include <QSerialPort>

class SerialWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialWorker(QObject *parent = nullptr);
    void start(const QString &portName);

signals:
    void levelReceived(int level, bool reverseOrder);

private slots:
    void handleReadyRead();

private:
    QSerialPort m_serial;
    QByteArray m_buffer;
};

#endif // SERIALWORKER_H
