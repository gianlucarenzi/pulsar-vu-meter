#ifndef DEMOMODE_H
#define DEMOMODE_H

#include <QObject>
#include <QTimer>

class DemoMode : public QObject {
    Q_OBJECT
public:
    DemoMode(QObject *parent, std::function<void(int, bool)> setLevelFunc);
    void start(bool reverseOrder = false);
    void toggleReverse();

private:
    QTimer m_timer;
    int m_value = 0;
    int m_dir = 1;
    int m_cycles = 0;
    bool m_reverseOrder = false;
    std::function<void(int, bool)> m_setLevelFunc;

private slots:
    void step();
};

#endif // DEMOMODE_H
