#include "logplot.h"
#include "qwt_plot_canvas.h"
#include "qwt_legend.h"

CLogPlot::CLogPlot(QWidget *parent) :
    QwtPlot(parent),
    m_NumCurves(0)
{
    // canvas
    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setLineWidth( 1 );
    canvas->setFrameStyle( QFrame::Box | QFrame::Plain );
    canvas->setBorderRadius( 15 );

    QPalette canvasPalette( Qt::white );
    canvasPalette.setColor( QPalette::Foreground, QColor( 133, 190, 232 ) );
    canvas->setPalette( canvasPalette );

    setCanvas( canvas );
}


void CLogPlot::pushValue (unsigned int curve, double data)
{
    if (curve < m_NumCurves)
    {
        m_YDataVect[curve].pop_back();
        m_YDataVect[curve].push_front(data);
        m_CurvesVect[curve]->setSamples(m_XData, m_YDataVect[curve]);
    }

}


void CLogPlot::refresh ()
{
    replot();
}


void CLogPlot::configure (unsigned int nbCurves, unsigned int size, QString title)
{

    if (m_NumCurves == 0) //Do not allow reconfiguration
    {
        m_NumCurves = nbCurves;
        m_Size = size;

        for (double y = 0.0; y < size; y+=1.0)
        {
            m_XData.push_back(y);
        }
        setAxisScale( xBottom, 0.0, m_Size );
        for (unsigned int i = 0; i < m_NumCurves; i++)
        {
            QwtPlotCurve * curve = new QwtPlotCurve ();
            m_CurvesVect.push_back(curve);
            QVector< double > data (m_Size,0.0);
            m_YDataVect.push_back(data);
            curve->setSamples(m_XData, m_YDataVect[i]);
            curve->setRenderHint( QwtPlotItem::RenderAntialiased );
            curve->attach( this );
        }
        if (title != QString::null)
        {
            setTitle( title );
        }

    }


}

void CLogPlot::setCurveParam (unsigned int curve, QString name, QColor color)
{
    if (curve < m_NumCurves)
    {
        m_CurvesVect[curve]->setPen(color);
        m_CurvesVect[curve]->setTitle(name);
        if (name != "")
        {
            insertLegend( new QwtLegend(), QwtPlot::RightLegend );
            m_CurvesVect[curve]->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );

        }
        else
        {
            m_CurvesVect[curve]->setLegendAttribute( QwtPlotCurve::LegendShowLine, false );
        }
    }
}
