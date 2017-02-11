#ifndef GRAPHSDIAG_H
#define GRAPHSDIAG_H

#include <QObject>
#include <qwt_series_data.h>
#include <qwt_plot_curve.h>
#include <QVector>
#include <QDialog>

namespace Ui {
    class GraphsDiag;
}


class GraphsSerieData;


class CGraphsDiag : public  QDialog
{
    Q_OBJECT
public:
    explicit CGraphsDiag(QWidget *parent = 0);



signals:

public slots:
    void sltPushAccData (float x, float y);
    void sltPushGyroData (float rot);
    void sltPushSpeedData (float left, float right);

private:
    Ui::GraphsDiag * ui;



};

#endif // GRAPHSDIAG_H
