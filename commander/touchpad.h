#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>

class TouchPad : public QWidget
{
    Q_OBJECT
public:
    explicit TouchPad(QWidget *parent = 0);

public slots:


signals:
    void speedUpdated (int x, int y);


protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent * ev);
    void mousePressEvent(QMouseEvent * ev);

private:
    QPen m_Pen;
    QBrush m_Brush;
    QPoint m_Mousepos;

};

#endif // TOUCHPAD_H
