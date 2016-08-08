#ifndef MAP_H
#define MAP_H

#include <QWidget>
#include <QPen>
#include <QBrush>


class Map : public QWidget
{
    Q_OBJECT
public:
    explicit Map(QWidget *parent = 0);

    void updateMap (unsigned int size, unsigned char * chunk, unsigned int offset, unsigned int chunkSz);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPen m_Pen;
    QBrush m_Brush;

    unsigned char * m_Map;
    unsigned int m_Width;
    unsigned int m_Height;

};

#endif // MAP_H
