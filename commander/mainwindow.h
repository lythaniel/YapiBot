#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTcpServer>
#include "../src/YapiBotCmd.h"
#include "VideoProcessing.h"
#include "videowidget.h"


namespace Ui {
class MainWindow;
}

class CAccelData;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void Connected();
    void Deconnected();
    void cmdPktReceived();
    void TryToConnect();
    void updateSpeed (int x, int y);

private slots:
    void on_Forw_pressed();

    void on_Rear_pressed();

    void on_Forw_released();

    void on_Rear_released();

    void on_Left_pressed();

    void on_Left_released();

    void on_Right_pressed();

    void on_Right_released();

    void on_CameraTild_valueChanged(int value);

    void on_CompassCalib_clicked();

    void on_moveStraight_clicked();

    void on_alignBearing_clicked();

    void on_moveBearing_clicked();

    void on_rotate_clicked();

    void on_script_clicked();

    void on_speed_valueChanged(int value);

    void on_paramlist_currentIndexChanged(int index);

    void on_paramval_valueChanged(double arg1);


    void on_refreshMap_clicked();

private:

    void sendCommand (YapiBotCmd_t cmd, char * payload = NULL, unsigned int pldsize = 0);

    void fromInt (char * buff, int val);
    int toInt (unsigned char * buff);

    Ui::MainWindow *ui;

    CAccelData * m_AccelData;

    int m_Speed;

    bool m_Connected;
    QTcpSocket * m_CmdSocket;
    char m_CmdBuffer[256];
    bool m_UpdateParam;
    VideoWidget * m_pVideo;
    CVideoProcessing * m_pVideoProc;
};

#endif // MAINWINDOW_H
