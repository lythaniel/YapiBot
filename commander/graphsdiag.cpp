#include "graphsdiag.h"
#include "ui_graphs.h"

#define GRAPH_SIZE 50



CGraphsDiag::CGraphsDiag(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GraphsDiag)

{
    ui->setupUi(this);
    ui->accPlot->configure(2,128,"Acceleration");
    ui->accPlot->setCurveParam(0,QString(""), Qt::blue );
    ui->accPlot->setCurveParam(1,QString(""), Qt::red );

    ui->rotPlot->configure(1,128,"Rotation");
    ui->rotPlot->setCurveParam(0,QString(""), Qt::blue );

    ui->speedPlot->configure(2,128,"Motors speed");
    ui->speedPlot->setCurveParam(0,QString(""), Qt::blue );
    ui->speedPlot->setCurveParam(1,QString(""), Qt::red );


}


void CGraphsDiag::sltPushAccData (float x, float y)
{
    ui->accPlot->pushValue(0,x);
    ui->accPlot->pushValue(1,y);
    ui->accPlot->refresh();
}

void CGraphsDiag::sltPushGyroData (float rot)
{
    ui->rotPlot->pushValue(0,rot);
    ui->rotPlot->refresh();
}

void CGraphsDiag::sltPushSpeedData (float left, float right)
{
    ui->speedPlot->pushValue(0,left);
    ui->speedPlot->pushValue(1,right);
    ui->speedPlot->refresh();

}
