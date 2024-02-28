#ifndef AUTOSCREENZOOM_H
#define AUTOSCREENZOOM_H

#include <QWidget>

class AutoScreenZoom : public QWidget
{
    Q_OBJECT
public:
    explicit AutoScreenZoom(QWidget *parent = nullptr);
    void AutoChildZoom(const QObject &o);
    double m_preRate;
    double m_scaleFromPreRate;

public slots:
    void onLogicalDotsPerInchChanged(qreal dpi);
};

#endif