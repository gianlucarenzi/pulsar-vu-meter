#include "vumeterwidget.h"
#include <QPainter>

VuMeterWidget::VuMeterWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(12 * 16 + 11 * 1 + 10); // led + spacing + margine
    setMinimumWidth(24 + 10); // led + margine
    m_level = 0; // Tutti i led spenti all'avvio
}

void VuMeterWidget::setLevel(int level, bool reverseOrder)
{
    m_level = level;
    m_reverseOrder = reverseOrder;
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
        switch (m_colorMode) {
            case LedColorGreen:
                color = QColor(0, 150, 0);
                break;
            case LedColorYellow:
                color = QColor(200, 200, 0);
                break;
            case LedColorOrange:
                color = QColor(255, 100, 0);
                break;
            case LedColorRed:
                color = QColor(200, 0, 0);
                break;
            case LedColorCustom:
                color = QColor(m_customR, m_customG, m_customB);
                break;
            case LedColorSegments:
            default:
                if (i < 3) color = QColor(0, 150, 0);         // verde
                else if (i < 6) color = QColor(200, 200, 0);  // giallo
                else if (i < 9) color = QColor(255, 100, 0);  // arancione
                else color = QColor(200, 0, 0);               // rosso
                break;
        }
        int y = m_reverseOrder ? (y0 + i * (ledHeight + spacing)) : (y0 + (numLeds - 1 - i) * (ledHeight + spacing));
        QRect ledRect(x0, y, ledWidth, ledHeight);
        painter.setPen(QPen(Qt::black, border));
        bool ledOn;
        if (m_level < 5) {
            ledOn = (i == specialGreenLed);
        } else {
            ledOn = (i < effectiveLevel);
        }
        painter.setBrush(ledOn ? color : QColor(30, 30, 30));
        painter.drawRect(ledRect);
    }
}
