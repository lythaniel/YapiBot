#ifndef LOGPLOT_H
#define LOGPLOT_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class CLogPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit CLogPlot(QWidget *parent = 0);

signals:

public slots:
    void configure (unsigned int nbCurves, unsigned int size, QString title = QString::null);
    void pushValue (unsigned int curve, double data);
    void refresh (void);
    void setCurveParam (unsigned int curve, QString name, QColor color);

private:
    QVector<QwtPlotCurve *> m_CurvesVect;
    QVector< double > m_XData;
    QVector < QVector< double > >  m_YDataVect;

    unsigned int m_NumCurves;
    unsigned int m_Size;
};

#endif // LOGPLOT_H
