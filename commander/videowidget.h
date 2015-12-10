#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QtNetwork>
#include <QTcpServer>
extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
}

#include <QLabel>


class VideoWidget : public QObject
{
    Q_OBJECT
public:
    explicit VideoWidget(QLabel* pView);


signals:

public slots:
    void StreamConnected();
    void StreamDeconnected();
    void RxByteRead();
    void hostFound();
    void TryToConnect();

private:

    void startServer (void);
    void startListener (void);
    void initCodec (void);
    void decodeStream (QByteArray * stream);
    void decodeFrame (char * pkt, int len);
    void closeCodec (void);

    QTcpSocket * m_RxSocket;


    bool m_StreamConnected;


    AVCodec * m_pCodec;
    AVCodecContext *m_pCodecCtx;
    AVCodecParserContext* m_pParser;
    AVFrame * m_pPicture;

    AVPicture * m_pRgbPic;
    SwsContext* m_pScaleCtx;

    AVPacket m_Avpkt;

    QLabel * m_pVideoOut;



};

#endif // VIDEOWIDGET_H
