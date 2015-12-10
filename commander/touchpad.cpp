#include "touchpad.h"

#include <QPainter>
#include <QCursor>
#include <QDebug>

TouchPad::TouchPad(QWidget *parent) :
    QWidget(parent),
    m_Mousepos(100,100)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);


}

void TouchPad::mouseMoveEvent(QMouseEvent *ev)
{
    int x, y;
    m_Mousepos.setX(ev->pos().x());
    m_Mousepos.setY(ev->pos().y());
    x = (100-m_Mousepos.y()) * 2;
    if (x > 100) x = 100;
    if (x < -100) x = -100;
    y = (100-m_Mousepos.x()) * 2;
    if (y > 100) y = 100;
    if (y < -100) y = -100;
    emit speedUpdated(x,y);
    update();
}


void TouchPad::mouseReleaseEvent(QMouseEvent * ev)
{
    m_Mousepos.setX(100);
    m_Mousepos.setY(100);
    emit speedUpdated(0,0);
    update();
}

void TouchPad::mousePressEvent(QMouseEvent * ev)
{
    int x, y;
    m_Mousepos.setX(ev->pos().x());
    m_Mousepos.setY(ev->pos().y());
    x = (100-m_Mousepos.y()) * 2;
    if (x > 100) x = 100;
    if (x < -100) x = -100;
    y = (100-m_Mousepos.x()) * 2;
    if (y > 100) y = 100;
    if (y < -100) y = -100;
    emit speedUpdated(x,y);
    update();
}


void TouchPad::paintEvent(QPaintEvent * /* event */)
{
   //QPoint p = this->mapFromGlobal(QCursor::pos());
   QPainter painter(this);
   painter.setPen(m_Pen);
   painter.setBrush(m_Brush);
   painter.drawLine(QPoint(100,100),m_Mousepos);
   painter.setPen(palette().dark().color());
   painter.setBrush(Qt::NoBrush);
   //qDebug() << "Paint event";
}
