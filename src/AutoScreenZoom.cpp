#include "AutoScreenZoom.h"

AutoScreenZoom::AutoScreenZoom(QWidget *parent) : QWidget(parent)
{
    m_preRate = 1.0;
    m_scaleFromPreRate = 1.0;
}

void AutoScreenZoom::AutoChildZoom(const QObject &o)
{
    for (auto &pWidget : o.findChildren<QWidget *>())
    {
        if (pWidget)
        {
            pWidget->resize(pWidget->width() * m_scaleFromPreRate, pWidget->height() * m_scaleFromPreRate);
            // AutoChildZoom(*pWidget);
        }
    }
}

void AutoScreenZoom::onLogicalDotsPerInchChanged(qreal dpi)
{
    qreal rate = dpi / 96.0;
    if (rate == m_preRate)
    {
        return;
    }
    m_scaleFromPreRate = rate / m_preRate;
    m_preRate = rate;
    AutoChildZoom(*this);
}
