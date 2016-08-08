#include "map.h"
#include <QPainter>

Map::Map(QWidget *parent) :
    QWidget(parent),
    m_Map(NULL),
    m_Width(0),
    m_Height(0)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void Map::updateMap (unsigned int size,  unsigned char * chunk, unsigned int offset, unsigned int chunkSz)
{
    if ((size != m_Width))
    {
        if (m_Map != NULL)
        {
            delete m_Map;
        }
        m_Width = size;
        m_Height = size;

        m_Map = new unsigned char [m_Width * m_Height];
        memset (m_Map,0xFF,m_Width * m_Height);
    }
    if (m_Map != NULL)
    {
        memcpy (&m_Map[offset],chunk,chunkSz);
    }

    this->repaint();
}

void Map::paintEvent(QPaintEvent * /* event */)
{
   unsigned int pixel_height;
   unsigned int pixel_width;
   unsigned int pixel_x = 0;
   unsigned int pixel_y = 0;

   QPainter painter(this);
   painter.setPen(m_Pen);
   painter.setBrush(m_Brush);

   if (m_Map != NULL)
   {
       if (m_Height != 0)
       {
           pixel_height = this->height() / m_Height;
       }
       if (m_Width != 0)
       {
           pixel_width = this->width() / m_Width;
       }
       for (unsigned int y = 0; y < m_Height; y++)
       {
           pixel_x = 0;
           for (unsigned int x = 0; x < m_Width; x++)
           {
               QColor color (m_Map[x+(y*m_Width)],m_Map[x+(y*m_Width)],m_Map[x+(y*m_Width)]);
               painter.fillRect(pixel_x, pixel_y, pixel_width, pixel_height, color);
               pixel_x += pixel_width;
           }
           pixel_y += pixel_height;
       }
   }

   painter.setPen(palette().dark().color());
   painter.setBrush(Qt::NoBrush);

}
