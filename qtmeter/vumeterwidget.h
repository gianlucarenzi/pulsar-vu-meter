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

private:
    int m_level = 0;
    bool m_reverseOrder = false;
};

#include <QtPlugin>
Q_DECLARE_METATYPE(VuMeterWidget*)
Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QWidget")
#endif // VUMETERWIDGET_H
