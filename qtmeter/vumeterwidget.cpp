#include "vumeterwidget.h"
#include <QPainter>
#include <QDebug>
extern int debugEnabled;
#define DEBUG_OUT if(debugEnabled) qDebug

VuMeterWidget::VuMeterWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(12 * 16 + 11 * 1 + 10); // led + spacing + margine
    setMinimumWidth(24 + 10); // led + margine
    m_level = 0; // Tutti i led spenti all'avvio
}

void VuMeterWidget::setLevel(int level, bool reverseOrder)
{
    // Mappa il livello 0-100 su 0-12 LED accesi (come firmware)
    int mapped = 0;
    if (level < 5) {
        mapped = (level > 0) ? 1 : 0;
    } else {
        mapped = qRound((level - 5) * 11.0 / 95.0) + 1;
        if (mapped < 0) mapped = 0;
        if (mapped > 12) mapped = 12;
    }
    m_level = mapped;
    m_reverseOrder = reverseOrder;
    DEBUG_OUT() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "VU-METER setLevel: level=" << level << ", mapped=" << mapped << ", reverseOrder=" << reverseOrder;
    update();
}

void VuMeterWidget::setRawLevel(int rawLevel, bool reverseOrder)
{
    m_level = rawLevel;
    m_reverseOrder = reverseOrder;
    DEBUG_OUT() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "VU-METER setRawLevel: rawLevel=" << rawLevel << ", reverseOrder=" << reverseOrder;
    update();
}

void VuMeterWidget::setReverseOrder(bool reverseOrder)
{
    m_reverseOrder = reverseOrder;
    update();
}

void VuMeterWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(180, 180, 180)); // sfondo grigio

    const int numLeds = 12;
    const int ledWidth = 24;
    const int ledHeight = 16;
    const int spacing = 1;
    const int border = 2;
    int totalHeight = numLeds * ledHeight + (numLeds - 1) * spacing;
    int x0 = (width() - ledWidth) / 2;
    int y0 = height() - totalHeight - 5; // 5px dal fondo
    if (y0 < 0) y0 = 0;

    int effectiveLevel = 0;
    int specialGreenLed = m_reverseOrder ? (numLeds - 1) : 0;
    if (m_level == 0) {
        effectiveLevel = 0;
    } else if (m_level < 5) {
        effectiveLevel = 1;
    } else {
        effectiveLevel = (m_level > 100) ? numLeds : ((m_level - 5) * (numLeds - 1) / 95 + 1);
    }
    for (int i = 0; i < numLeds; ++i) {
        QColor color;
        int r=0,g=0,b=0;
        switch (m_colorMode) {
            case LedColorGreen:
                r=0; g=150; b=0;
                color = QColor(r, g, b);
                break;
            case LedColorYellow:
                r=200; g=200; b=0;
                color = QColor(r, g, b);
                break;
            case LedColorOrange:
                r=255; g=100; b=0;
                color = QColor(r, g, b);
                break;
            case LedColorRed:
                r=200; g=0; b=0;
                color = QColor(r, g, b);
                break;
            case LedColorCustom:
                color = QColor(m_customR, m_customG, m_customB);
                break;
            case LedColorSegments:
            default:
                if (i < 3) { r=0; g=150; b=0; color = QColor(r, g, b); }         // verde
                else if (i < 6) { r=200; g=200; b=0; color = QColor(r, g, b); }  // giallo
                else if (i < 9) { r=255; g=100; b=0; color = QColor(r, g, b); }  // arancione
                else { r=200; g=0; b=0; color = QColor(r, g, b); }               // rosso
                break;
        }
        int y = m_reverseOrder ? (y0 + i * (ledHeight + spacing)) : (y0 + (numLeds - 1 - i) * (ledHeight + spacing));
        QRect ledRect(x0, y, ledWidth, ledHeight);
        painter.setPen(QPen(Qt::black, border));
        bool ledOn;
        if (m_level < 12) {
            ledOn = (i < m_level);
        } else {
            ledOn = true;
        }
        if (ledOn && m_colorMode == LedColorSegments) {
            DEBUG_OUT() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "LED" << i << "ON: RGB(" << r << g << b << ")";
        }
        painter.setBrush(ledOn ? color : QColor(30, 30, 30));
        painter.drawRect(ledRect);
    }
}
