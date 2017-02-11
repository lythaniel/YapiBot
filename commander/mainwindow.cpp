#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_graphs.h"
#include "videowidget.h"
#include <QVBoxLayout>
#include "touchpad.h"
#include <QInputDialog>
#include "QtCore/qmath.h"
#include <qlayout.h>
#include <qwt_compass.h>
#include <qwt_compass_rose.h>
#include <qwt_dial_needle.h>
#include <qwt_series_data.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_polar_marker.h>
#include <qwt_scale_engine.h>
#include "map.h"


#define CMDSERVER "192.168.1.222"
#define CMDPORT 9998

#define MAX_PARAM_NAME_SIZE 30
#define MAX_INT 0x7FFFFFF

typedef struct {
    YapiBotParam_t param;
    QString ParamName;
    char type;
    int precision;
    float min;
    float max;
} ParamList_t;


#define NUM_PARAM 15

const ParamList_t gParamList [NUM_PARAM] =
{
    {CtrlParamColDist, "CtrlParamColDist", 'i', 0, 0, 100},
    {CtrlParamMvErrGain, "CtrlParamMvErrGain", 'i', 0, 1, 100},
    {CtrlParamBearingErrGain, "CtrlParamBearingErrGain", 'i', 0, 1, 100},
    {CtrlParamBearingErrLim, "CtrlParamBearingErrLim", 'i', 0, 0, 100},
    {CtrlParamBearingGoodCnt, "CtrlParamBearingGoodCnt", 'i', 0, 1, 100},
    {MtrParamSpeedConv, "MtrParamSpeedConv", 'f', 1, 0.1, 10},
    {MtrParamSpeedErrGain, "MtrParamSpeedErrGain", 'f', 1, 0.1, 10},
    {MtrParamAccErrGain, "MtrParamAccErrGain", 'f', 1, 0.1, 10},
    {MtrParamMinSpeedCmd, "MtrParamMinSpeedCmd", 'i', 0, 0, 100},
    {CamParamSaturation, "CamParamSaturation", 'i', 0, -100, 100},
    {CamParamContrast, "CamParamContrast", 'i', 0, -100, 100},
    {CamParamBrightness, "CamParamBrightness", 'i', 0, -100, 100},
    {CamParamsharpness, "CamParamsharpness", 'i', 0,-100, 100},
    {CamParamIso, "CamParamIso", 'i', 0, 0, MAX_INT},
    {PosFltGain, "PosFltGain", 'f', 1, 0.1, 20},
};


class CAccelData: public QwtSeriesData<QwtPointPolar>
{
public:
    CAccelData( float x, float y )
    {
        QPointF point ((qreal)y,(qreal)x);
        coord.setPoint(point);

    }

    void setValue(float x, float y)
    {
        QPointF point ((qreal)y,(qreal)x);
        coord.setPoint(point);
    }

    virtual size_t size() const
    {
        return 1;
    }
    virtual QwtPointPolar sample( size_t ) const
    {
        return coord;
    }

    virtual QRectF boundingRect() const
    {
        if ( d_boundingRect.width() < 0.0 )
            d_boundingRect = qwtBoundingRect( *this );

        return d_boundingRect;
    }

protected:

   QwtPointPolar coord;
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_Connected(false),
    m_SizeToCopy(0),
    m_PayloadIdx(0),
    m_pGraphs(NULL)
{
    //QVBoxLayout * layout = new QVBoxLayout();
    //QPalette palette0;

    ui->setupUi(this);
    m_pVideoProc = new CVideoProcessing();
    m_pVideo = new VideoWidget(ui->VideoView, m_pVideoProc);
    TouchPad * tp = ui->touchPad;
    m_Speed = 100;

    m_CmdSocket = new QTcpSocket();
    connect(m_CmdSocket, SIGNAL(connected()), this, SLOT(Connected()));
    connect(m_CmdSocket, SIGNAL(readyRead()),this, SLOT(cmdPktReceived()));
    connect(tp, SIGNAL(speedUpdated(int,int)),this,SLOT(updateSpeed(int,int)));
    qDebug() << "connecting to cmd socket ...";

    for (int i = 0; i < NUM_PARAM; i++)
    {
        ui->paramlist->addItem(gParamList[i].ParamName);
    }
    m_UpdateParam = false;




    QwtCompassScaleDraw *scaleDraw = new QwtCompassScaleDraw();
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Ticks, true );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Labels, true );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    scaleDraw->setTickLength( QwtScaleDiv::MinorTick, 1 );
    scaleDraw->setTickLength( QwtScaleDiv::MediumTick, 1 );
    scaleDraw->setTickLength( QwtScaleDiv::MajorTick, 3 );

    ui->Compass->setScaleDraw( scaleDraw );

    ui->Compass->setScaleMaxMajor( 36 );
    ui->Compass->setScaleMaxMinor( 5 );

    ui->Compass->setNeedle(new QwtCompassMagnetNeedle( QwtCompassMagnetNeedle::ThinStyle ) );


    QPalette newPalette = ui->Compass->palette();

    //Set colors for compass.
    newPalette.setColor( QPalette::Base, Qt::darkBlue );
    newPalette.setColor( QPalette::WindowText, QColor( Qt::darkBlue ).dark( 120 ) );
    newPalette.setColor( QPalette::Text, Qt::white );

    //initialise color group for compass.
    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        const QPalette::ColorGroup colorGroup = static_cast<QPalette::ColorGroup>( i );

        const QColor light = newPalette.color( colorGroup, QPalette::Base ).light( 170 );
        const QColor dark = newPalette.color( colorGroup, QPalette::Base ).dark( 170 );
        const QColor mid = ui->Compass->frameShadow() == QwtDial::Raised
            ? newPalette.color( colorGroup, QPalette::Base ).dark( 110 )
            : newPalette.color( colorGroup, QPalette::Base ).light( 110 );

        newPalette.setColor( colorGroup, QPalette::Dark, dark );
        newPalette.setColor( colorGroup, QPalette::Mid, mid );
        newPalette.setColor( colorGroup, QPalette::Light, light );
    }

    ui->Compass->setPalette( newPalette );
    ui->AngularRatePlot->setPalette( newPalette);
    ui->AngularRatePlot->setNeedle(new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow,true,Qt::white,Qt::gray));


    const QwtInterval radialInterval( 0.0, 2.0 );
    const QwtInterval azimuthInterval( 0.0, 2* M_PI );

    ui->AccelPlot->setPlotBackground(Qt::darkBlue);

    //ui->AccelPlot->setAutoReplot(true);

    // scales
    ui->AccelPlot->setScale( QwtPolar::Azimuth,azimuthInterval.minValue(), azimuthInterval.maxValue(), azimuthInterval.width() / 12 );

    //ui->AccelPlot->setScaleMaxMinor( QwtPolar::Azimuth, 2 );
    ui->AccelPlot->setScale( QwtPolar::Radius,radialInterval.minValue(), radialInterval.maxValue() );


    // grids, axes

    QwtPolarGrid * d_grid = new QwtPolarGrid();
    d_grid->setPen( QPen( Qt::white ) );

    d_grid->setAxisPen( QwtPolar::AxisAzimuth, QPen( Qt::black ) );
    d_grid->showAxis( QwtPolar::AxisAzimuth, false );
    d_grid->showAxis( QwtPolar::AxisLeft, false );
    d_grid->showAxis( QwtPolar::AxisRight, false );
    d_grid->showAxis( QwtPolar::AxisTop, false );
    d_grid->showAxis( QwtPolar::AxisBottom, false );
    d_grid->showGrid( QwtPolar::Azimuth, false );
    d_grid->showGrid( QwtPolar::Radius, true );
    d_grid->attach( ui->AccelPlot );

    m_AccelData = new CAccelData (0,0);


    QwtPolarCurve * curve = new QwtPolarCurve();
    curve->setStyle( QwtPolarCurve::Lines);
    curve->setPen( QPen( Qt::yellow, 2 ) );
    curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( Qt::red ), QPen( Qt::white ), QSize( 7, 7 ) ) );
    curve->setData(m_AccelData);
    curve->attach( ui->AccelPlot);

    m_pGraphs = new CGraphsDiag (this);


    TryToConnect();

}

MainWindow::~MainWindow()
{
    sendCommand (CmdMoveStop);
    delete m_pVideo;
    delete m_pVideoProc;
    delete ui;

}

void MainWindow::fromInt (int val, void * buff)
{
    memcpy (buff,&val,4);
}

void MainWindow::updateSpeed (int x, int y)
{
    unsigned char payload [8];
    fromInt(x, &payload[0]);
    fromInt(y, &payload[4]);
    sendCommand(CmdMove,payload,8);
    qDebug() << "speed updated: (" << x << " / " << y << ")";
}

void MainWindow::on_Forw_pressed()
{
    unsigned char payload [4];
    fromInt(m_Speed, &payload[0]);
    sendCommand (CmdMoveFwd,payload,4);
}

void MainWindow::on_Rear_pressed()
{
     unsigned char payload [4];
     fromInt(m_Speed, &payload[0]);
     sendCommand (CmdMoveRear,payload,4);
}

void MainWindow::on_Forw_released()
{
     sendCommand (CmdMoveStop);
}

void MainWindow::on_Rear_released()
{
     sendCommand (CmdMoveStop);
}

void MainWindow::on_Left_pressed()
{
    unsigned char payload [4];
    fromInt(m_Speed, &payload[0]);
    sendCommand (CmdMoveLeft,payload,4);
}

void MainWindow::on_Left_released()
{
     sendCommand (CmdMoveStop);
}

void MainWindow::on_Right_pressed()
{
     unsigned char payload [4];
     fromInt(m_Speed, &payload[0]);
     sendCommand (CmdMoveRight,payload,4);
}

void MainWindow::on_Right_released()
{
     sendCommand (CmdMoveStop);
}

void MainWindow::Connected()
{

    unsigned char payload[8];

    m_Connected = true;

    int index = ui->paramlist->currentIndex();

    fromInt(gParamList[index].param, &payload[0]);


    sendCommand(CmdGetParam, payload,4);

    qDebug() << "Cmd socket connected !";
}

void MainWindow::Deconnected()
{
    qDebug() << "Cmd socket connection lost!";
    QTimer::singleShot(1000, this, SLOT(TryToConnect()));
}

void MainWindow::cmdPktReceived(void)
{
    QByteArray buff = m_CmdSocket->readAll();
    unsigned int len = buff.size();
    unsigned char * cbuff = (unsigned char *)buff.data();
    unsigned int idx = 0;
    while (len > 0)
    {
        if (m_SizeToCopy > 0) //We still have payload to copy.
        {
            unsigned int copysz = (m_SizeToCopy<len)?m_SizeToCopy:len;
            if ((copysz + m_PayloadIdx) > YAPIBOT_MAX_PL_SIZE)
            {
                //Something is very wrong here.
                copysz = YAPIBOT_MAX_PL_SIZE - m_PayloadIdx;
                qDebug() << "[NETWORK] Error during payload copy, payload will be truncated";
            }
            memcpy(&m_RxPayload[m_PayloadIdx],&cbuff[idx],copysz);
            len -= copysz;
            m_SizeToCopy -= copysz;
            idx += copysz;
            m_PayloadIdx += copysz;
            if ((m_SizeToCopy == 0)||(m_PayloadIdx == YAPIBOT_MAX_PL_SIZE))
            {
                processCmd(m_CmdId, m_RxPayload, m_PayloadIdx);
                m_SizeToCopy = 0;
                m_PayloadIdx = 0;
            }
        }
        else
        {
            header = (YapiBotHeader_t *)&cbuff[idx];
            if (header->magicNumber == YAPIBOT_MAGIC_NUMBER)
            {
                idx += sizeof (YapiBotHeader_t);
                len -= sizeof (YapiBotHeader_t);
                m_PayloadIdx = 0;

                if (header->payloadSize > YAPIBOT_MAX_PL_SIZE)
                {
                    qDebug() << "[NETWORK] Error max pl size reached for Rx payload, payload will be truncated (id = " << header->id<< " , size = " << header->payloadSize << " )";
                    header->payloadSize = YAPIBOT_MAX_PL_SIZE;
                }
                m_SizeToCopy = header->payloadSize;
                m_PayloadIdx = 0;
                m_CmdId = header->id;
                if (m_SizeToCopy == 0)
                {
                    processCmd(m_CmdId, NULL, 0);
                }
            }
            else //We are not synchronized.
            {
                //Advance to the next byte.
                len -= 1;
                idx += 1;
            }
        }
   }
}

void MainWindow::processCmd(YapiBotCmd_t cmd, unsigned char * payload, unsigned int plsize)
{

    switch (cmd)
    {
    case CmdInfoStatus:
    {
        if (plsize == sizeof (YapiBotStatus_t))
        {
            int heading = toInt(&payload[0]);
            int speedLeft = toInt(&payload[4]);
            int speedRight = toInt(&payload[8]);
            int campos = toInt(&payload[12]);
            int range = toInt(&payload[16]);
            int measLeft = toInt(&payload[20]);
            int measRight = toInt(&payload[24]);
            float accelX = toFloat(&payload[28]);
            float accelY = toFloat(&payload[32]);
            float rotZ = toFloat(&payload[36]);
            ui->speedLeft->setValue(abs(speedLeft));
            ui->speedRight->setValue(abs(speedRight));
            ui->heading->display(heading);
            ui->Compass->setValue(heading);
            ui->CameraTild->setSliderPosition(campos);
            ui->range->setText(QString::number(range));
            ui->measLeft->setText(QString::number(measLeft));
            ui->measRight->setText(QString::number(measRight));
            m_AccelData->setValue(accelX,accelY);
            ui->AccelPlot->replot();
            ui->AngularRatePlot->setValue(rotZ );
            m_pGraphs->sltPushAccData(accelX, accelY);
            m_pGraphs->sltPushSpeedData(measLeft, measRight);
            m_pGraphs->sltPushGyroData(rotZ);
        }
        else
        {
            qDebug() << "[NETWORK] Error: CmdInfoStatus received with incorrect size !";
        }
        break;
    }

    case CmdInfoParam:
    {
        if (plsize == sizeof(YapiBotParamAnswer_t))
        {
            YapiBotParam_t param = (YapiBotParam_t)toInt(&payload[0]);
            int index = ui->paramlist->currentIndex();
            if (param == gParamList[index].param)
            {
                if (gParamList[index].type == 'i')
                {
                    ui->paramval->setValue(toInt(&payload[4]));
                }
                else
                {
                    int val = toInt(&payload[4]);
                    ui->paramval->setValue(*((float*)&val));
                }
                //ui->paramval->setMinimum(gParamList[index].min);
                //ui->paramval->setMaximum(gParamList[index].max);
                m_UpdateParam = true;

            }
        }
        else
        {
            qDebug() << "[NETWORK] Error: CmdInfoParam received with incorrect size !";
        }
        break;
    }

    case CmdInfoMap:
    {
        Map * map = ui->uiMap;
        unsigned int mapSz = toInt(&payload[0]);
        unsigned int offset = toInt(&payload[4]);
        map->updateMap (mapSz, (unsigned char *)&payload[8], offset, plsize-8);
        break;
    }

    default:
    {
        qDebug() << "[NETWORK] Error: unknown cmd received !";
        break;
    }
    }
}

void MainWindow::TryToConnect()
{
    m_CmdSocket->connectToHost(CMDSERVER, CMDPORT);

    // we need to wait...
    if(!m_CmdSocket->waitForConnected(1000))
    {
        qDebug() << "Error: " << m_CmdSocket->errorString();
        QTimer::singleShot(1000, this, SLOT(TryToConnect()));
    }

}

void MainWindow::sendCommand (YapiBotCmd_t cmd, unsigned char * payload, unsigned int plsize)
{
    if (m_Connected)
    {
        YapiBotHeader_t * header = (YapiBotHeader_t * )&m_CmdBuffer[0];

        header->magicNumber = YAPIBOT_MAGIC_NUMBER;
        header->id = cmd;
        header->payloadSize = plsize;

        if ((payload != NULL) && (header->payloadSize > 0))
        {
            if (header->payloadSize > YAPIBOT_MAX_PL_SIZE)
            {
                qDebug() << "Warning command payload too long. payload is being truncated !!!";
                header->payloadSize = YAPIBOT_MAX_PL_SIZE;
            }
            memcpy (&m_CmdBuffer[sizeof(YapiBotHeader_t)],payload,header->payloadSize);
        }
        m_CmdSocket->write((char *)m_CmdBuffer,sizeof(YapiBotHeader_t) + header->payloadSize);
    }
}

void MainWindow::on_CameraTild_valueChanged(int value)
{
    unsigned char payload[4];
    fromInt(value, &payload[0]);
    sendCommand(CmdMoveCam,payload,4);
}


int MainWindow::toInt (void * buff)
{
    int ret;
    memcpy (&ret,buff,4);
    return (ret);
}
float MainWindow::toFloat (void * buff)
{
    float ret;
    memcpy (&ret,buff,4);
    return (ret);
}

void MainWindow::on_CompassCalib_clicked()
{
    sendCommand(CmdCompassCal);
}

void MainWindow::on_moveStraight_clicked()
{
    unsigned char payload [4];
    bool ok;
    int dist = QInputDialog::getInt(this,QString("Enter distance"),QString("Distance:"),0,-10000,10000,10,&ok);
    if (ok)
    {
        fromInt(dist, &payload[0]);
        sendCommand(CmdMoveStraight,payload,4);
    }
}

void MainWindow::on_alignBearing_clicked()
{
    unsigned char payload [4];
    bool ok;
    int bearing = QInputDialog::getInt(this,QString("Enter bearing"),QString("bearing:"),0,0,360,1,&ok);
    if (ok)
    {
        fromInt(bearing, &payload[0]);
        sendCommand(CmdAlignBearing,payload,4);
    }
}

void MainWindow::on_moveBearing_clicked()
{
    unsigned char payload [8];
    bool ok;
    int dist = QInputDialog::getInt(this,QString("Enter distance"),QString("Distance:"),0,-10000,10000,10,&ok);
    if (ok)
    {
        int bearing = QInputDialog::getInt(this,QString("Enter bearing"),QString("bearing:"),0,0,360,1,&ok);
        if (ok)
        {
            fromInt(dist, &payload[0]);
            fromInt(bearing, &payload[4]);
            sendCommand(CmdMoveBearing,payload,8);
        }
    }
}

void MainWindow::on_rotate_clicked()
{
    unsigned char payload [4];
    bool ok;
    int dist = QInputDialog::getInt(this,QString("Enter distance to rotate"),QString("Distance:"),0,-10000,10000,10,&ok);
    if (ok)
    {
        fromInt(dist, &payload[0]);
        sendCommand(CmdRotate,payload,4);
    }
}

void MainWindow::on_script_clicked()
{
    sendCommand((YapiBotCmd_t)CMD_TYPE_SCRIPT,NULL,0);
}

void MainWindow::on_speed_valueChanged(int value)
{
    m_Speed = value;
    ui->ConsSpeed->setText(QString::number(m_Speed));
}

void MainWindow::on_paramlist_currentIndexChanged(int index)
{
    unsigned char payload[4];

    m_UpdateParam = false;
    ui->paramval->setMinimum(gParamList[index].min);
    ui->paramval->setMaximum(gParamList[index].max);

    if (gParamList[index].type == 'i')
    {
        ui->paramval->setDecimals(0);
        ui->paramval->setSingleStep(1);
    }
    else if (gParamList[index].type == 'f')
    {
        ui->paramval->setDecimals(gParamList[index].precision);
        ui->paramval->setSingleStep(qPow(10,-gParamList[index].precision));
    }
    else
    {
        ui->paramval->setDecimals(2);
        ui->paramval->setSingleStep(0.01);
    }

    fromInt(gParamList[index].param, &payload[0]);

    sendCommand(CmdGetParam, payload,4);
}

void MainWindow::on_paramval_valueChanged(double arg1)
{

    unsigned char payload[8];

       if(m_UpdateParam == true)
       {
            int index = ui->paramlist->currentIndex();

            fromInt(gParamList[index].param, &payload[0]);

            if (gParamList[index].type == 'i')
            {
                int val;
                val = arg1;
                fromInt(val, &payload[4]);
            }
            else if (gParamList[index].type == 'f')
            {
                float val;
                val = arg1;
                fromInt(*((int*)&val), &payload[4]);
            }
            sendCommand(CmdSetParam, payload,8);
       }
}


void MainWindow::on_refreshMap_clicked()
{
    sendCommand(CmdRefrehMap);
}

void MainWindow::on_btnGraphs_clicked()
{
    m_pGraphs->show();
}
