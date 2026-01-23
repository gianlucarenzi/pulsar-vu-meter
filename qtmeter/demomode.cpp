#include "demomode.h"

DemoMode::DemoMode(QObject *parent, std::function<void(int, bool)> setLevelFunc)
    : QObject(parent), m_setLevelFunc(setLevelFunc)
{
    connect(&m_timer, &QTimer::timeout, this, &DemoMode::step);
}

void DemoMode::start(bool reverseOrder)
{
    m_value = 0;
    m_dir = 1;
    m_cycles = 0;
    m_reverseOrder = reverseOrder;
    m_timer.start(80); // 80ms per step
}

void DemoMode::toggleReverse()
{
    m_reverseOrder = !m_reverseOrder;
}

#include <QDebug>
extern int debugEnabled;
#define DEBUG_OUT if(debugEnabled) qDebug
void DemoMode::step()
{
    int displayValue = m_value;
    if (m_dir < 0 && m_value < 100) {
        displayValue = m_value; // in discesa mostra solo i led accesi
    }
    DEBUG_OUT() << "Demo VU value:" << displayValue << ", reverse:" << m_reverseOrder;
    m_setLevelFunc(displayValue, m_reverseOrder);
    m_value += m_dir;
    if (m_dir > 0 && m_value > 100) {
        m_value = 100;
        m_dir = -1;
        m_cycles++;
    } else if (m_dir < 0 && m_value < 0) {
        m_value = 0;
        m_dir = 1;
        m_cycles++;
    }
    if (m_cycles >= 4) {
        m_timer.stop();
    }
}
