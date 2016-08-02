#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videowidget.h"
#include <QVBoxLayout>
#include "touchpad.h"
#include <QInputDialog>
#include "QtCore/qmath.h"
#include <qlayout.h>
#include <qwt_compass.h>
#include <qwt_compass_rose.h>
#include <qwt_dial_needle.h>
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


#define NUM_PARAM 13

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
    {CamParamSaturation, "CamParamSaturation", 'i', 0, -100, 100},
    {CamParamContrast, "CamParamContrast", 'i', 0, -100, 100},
    {CamParamBrightness, "CamParamBrightness", 'i', 0, -100, 100},
    {CamParamsharpness, "CamParamsharpness", 'i', 0,-100, 100},
    {CamParamIso, "CamParamIso", 'i', 0, 0, MAX_INT},
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_Connected(false)
{
    QVBoxLayout * layout = new QVBoxLayout();
    QPalette palette0;

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

    TryToConnect();

}

MainWindow::~MainWindow()
{
    sendCommand (CmdMoveStop);
    delete m_pVideo;
    delete m_pVideoProc;
    delete ui;

}

void MainWindow::fromInt (char * buff, int val)
{
    buff [0] = (val >> 24) & 0xFF;
    buff [1] = (val >> 16) & 0xFF;
    buff [2] = (val >> 8) & 0xFF;
    buff [3] = val & 0xFF;
}

void MainWindow::updateSpeed (int x, int y)
{
    char payload [8];
    fromInt(&payload[0],x);
    fromInt(&payload[4],y);
    sendCommand(CmdMove,payload,8);
    qDebug() << "speed updated: (" << x << " / " << y << ")";
}

void MainWindow::on_Forw_pressed()
{
    char payload [4];
    fromInt(&payload[0],m_Speed);
    sendCommand (CmdMoveFwd,payload,4);
}

void MainWindow::on_Rear_pressed()
{
     char payload [4];
     fromInt(&payload[0],m_Speed);
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
    char payload [4];
    fromInt(&payload[0],m_Speed);
    sendCommand (CmdMoveLeft,payload,4);
}

void MainWindow::on_Left_released()
{
     sendCommand (CmdMoveStop);
}

void MainWindow::on_Right_pressed()
{
     char payload [4];
     fromInt(&payload[0],m_Speed);
     sendCommand (CmdMoveRight,payload,4);
}

void MainWindow::on_Right_released()
{
     sendCommand (CmdMoveStop);
}

void MainWindow::Connected()
{

    char payload[8];

    m_Connected = true;

    int index = ui->paramlist->currentIndex();

    fromInt(&payload[0],gParamList[index].param);


    sendCommand(CmdGetParam, payload,4);

    qDebug() << "Cmd socket connected !";
}

void MainWindow::Deconnected()
{
    qDebug() << "Cmd socket connection lost!";
    QTimer::singleShot(1000, this, SLOT(TryToConnect()));
}

void MainWindow::cmdPktReceived()
{
    QByteArray buff = m_CmdSocket->readAll();
    int size = buff.size();
    unsigned char * cbuff = (unsigned char *)buff.data();
    unsigned char * mapbuffer;
    while (size >= 4)
    {
        int id = toInt(&cbuff[0]);
        if ((unsigned int)id == YAPIBOT_STATUS)
        {
            int heading = toInt(&cbuff[4]);
            int speedLeft = toInt(&cbuff[8]);
            int speedRight = toInt(&cbuff[12]);
            int campos = toInt(&cbuff[16]);
            int range = toInt(&cbuff[20]);
            int measLeft = toInt(&cbuff[24]);
            int measRight = toInt(&cbuff[28]);
            ui->speedLeft->setValue(abs(speedLeft));
            ui->speedRight->setValue(abs(speedRight));
            ui->heading->display(heading);
            ui->Compass->setValue(heading);
            ui->CameraTild->setSliderPosition(campos);
            ui->range->setText(QString::number(range));
            ui->measLeft->setText(QString::number(measLeft));
            ui->measRight->setText(QString::number(measRight));
            //qDebug() << "camPos:" << campos;
            cbuff +=32;
            size -= 32;
        }
        else if ((unsigned int)id == YAPIBOT_PARAM)
        {
            YapiBotParam_t param = (YapiBotParam_t)toInt(&cbuff[4]);
            int index = ui->paramlist->currentIndex();
            if (param == gParamList[index].param)
            {
                if (gParamList[index].type == 'i')
                {
                    ui->paramval->setValue(toInt(&cbuff[8]));
                }
                else
                {
                    int val = toInt(&cbuff[8]);
                    ui->paramval->setValue(*((float*)&val));
                }
                //ui->paramval->setMinimum(gParamList[index].min);
                //ui->paramval->setMaximum(gParamList[index].max);
                m_UpdateParam = true;

            }
            cbuff+=12;
            size -= 12;
        }
        else if ((unsigned int)id == YAPIBOT_MAP)
        {
            unsigned int sz = toInt(&cbuff[4]);
            unsigned int sz_to_copy = sz * sz;
            unsigned int copy_sz;
            unsigned int copy_idx = 0;
            mapbuffer = new unsigned char [sz_to_copy];
            cbuff += 8;
            size -= 8;
            do
            {
                copy_sz = size>=sz_to_copy?sz_to_copy:size;
                memcpy (&mapbuffer[copy_idx],cbuff,copy_sz);
                copy_idx += copy_sz;
                cbuff += copy_sz;
                size -= copy_sz;
                sz_to_copy -= copy_sz;
                if (size <= 0)
                {
                    buff = m_CmdSocket->readAll();
                    size = buff.size();
                    cbuff = (unsigned char *)buff.data();
                }
            } while (sz_to_copy > 0);

            Map * map = ui->uiMap;
            map->updateMap (sz, sz, mapbuffer);

        }
        else
        {
            cbuff ++;
            size --;
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

void MainWindow::sendCommand (YapiBotCmd_t cmd, char * payload, unsigned int pldsize)
{
    if (m_Connected)
    {

        m_CmdBuffer[0] = (cmd >> 8)& 0xFF;
        m_CmdBuffer[1] =  cmd & 0xFF;
        if ((payload != NULL) && (pldsize > 0))
        {
            if (pldsize > 254)
            {
                qDebug() << "Warning command payload too long. payload is being truncated !!!";
                pldsize = 254;
            }
            memcpy (&m_CmdBuffer[2],payload,pldsize);


        }
        m_CmdSocket->write(m_CmdBuffer,2 + pldsize);
    }
}

void MainWindow::on_CameraTild_valueChanged(int value)
{
    char payload[4];
    fromInt(&payload[0],value);
    sendCommand(CmdMoveCam,payload,4);
}


int MainWindow::toInt (unsigned char * buff)
{
    int val0 = buff[0];
    int val1 = buff[1];
    int val2 = buff[2];
    int val3 = buff[3];

    val1 = val1 << 8;
    val2 = val2 << 16;
    val3 = val3 << 24;

    int ret = val3 + val2 + val1 + val0;
    return (ret);
}

void MainWindow::on_CompassCalib_clicked()
{
    sendCommand(CmdCompassCal);
}

void MainWindow::on_moveStraight_clicked()
{
    char payload [4];
    bool ok;
    int dist = QInputDialog::getInt(this,QString("Enter distance"),QString("Distance:"),0,-10000,10000,10,&ok);
    if (ok)
    {
        fromInt(&payload[0],dist);
        sendCommand(CmdMoveStraight,payload,4);
    }
}

void MainWindow::on_alignBearing_clicked()
{
    char payload [4];
    bool ok;
    int bearing = QInputDialog::getInt(this,QString("Enter bearing"),QString("bearing:"),0,0,360,1,&ok);
    if (ok)
    {
        fromInt(&payload[0],bearing);
        sendCommand(CmdAlignBearing,payload,4);
    }
}

void MainWindow::on_moveBearing_clicked()
{
    char payload [8];
    bool ok;
    int dist = QInputDialog::getInt(this,QString("Enter distance"),QString("Distance:"),0,-10000,10000,10,&ok);
    if (ok)
    {
        int bearing = QInputDialog::getInt(this,QString("Enter bearing"),QString("bearing:"),0,0,360,1,&ok);
        if (ok)
        {
            fromInt(&payload[0],dist);
            fromInt(&payload[4],bearing);
            sendCommand(CmdMoveBearing,payload,8);
        }
    }
}

void MainWindow::on_rotate_clicked()
{
    char payload [4];
    bool ok;
    int dist = QInputDialog::getInt(this,QString("Enter distance to rotate"),QString("Distance:"),0,-10000,10000,10,&ok);
    if (ok)
    {
        fromInt(&payload[0],dist);
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
    char payload[4];

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

    fromInt(&payload[0],gParamList[index].param);

    sendCommand(CmdGetParam, payload,4);
}

void MainWindow::on_paramval_valueChanged(double arg1)
{

    char payload[8];

       if(m_UpdateParam == true)
       {
            int index = ui->paramlist->currentIndex();

            fromInt(&payload[0],gParamList[index].param);

            if (gParamList[index].type == 'i')
            {
                int val;
                val = arg1;
                fromInt(&payload[4],val);
            }
            else if (gParamList[index].type == 'f')
            {
                float val;
                val = arg1;
                fromInt(&payload[4],*((int*)&val));
            }
            sendCommand(CmdSetParam, payload,8);
       }
}


void MainWindow::on_refreshMap_clicked()
{
    sendCommand(CmdRefrehMap);
}
