#ifndef VUMETERWIDGET_H
#define VUMETERWIDGET_H

#include <QWidget>

class VuMeterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VuMeterWidget(QWidget *parent = nullptr);
    void setLevel(int level, bool reverseOrder = false);
    void setReverseOrder(bool reverseOrder);

protected:
    void paintEvent(QPaintEvent *event) override;

public:
    enum LedColorMode {
        LedColorSegments = 0,
        LedColorGreen,
        LedColorYellow,
        LedColorOrange,
        LedColorRed,
        LedColorCustom
    };
    void setLedColorMode(LedColorMode mode) { m_colorMode = mode; }
    void setCustomColor(int r, int g, int b) { m_customR = r; m_customG = g; m_customB = b; }
private:
    int m_level = 0;
    bool m_reverseOrder = false;
    LedColorMode m_colorMode = LedColorSegments;
    int m_customR = 0, m_customG = 0, m_customB = 0;
};

#include <QtPlugin>
Q_DECLARE_METATYPE(VuMeterWidget*)
Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QWidget")
#endif // VUMETERWIDGET_H
