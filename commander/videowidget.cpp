#include "videowidget.h"
#include <QVBoxLayout>

#include <QDebug>
#include <vlc-qt/Common.h>
#include <vlc-qt/Instance.h>
#include <vlc-qt/Media.h>
#include <vlc-qt/MediaPlayer.h>
#include <vlc-qt/WidgetVideo.h>
#include <vlc-qt/Video.h>
#include <QTime>
#include <QObject>



#define SERVER "192.168.1.222"
#define PORT 9999


VideoWidget::VideoWidget(QLabel* pView, CVideoProcessing * videoproc) :
m_pVideoOut (pView),
m_pVideoProc(videoproc)
 {
    //m_pVideoOut->setGeometry(0,0,640,480);

    m_RxSocket = NULL;

    m_pRgbPic = NULL;
    m_pScaleCtx = NULL;

    m_StreamConnected = false;

    initCodec();

    startListener();


}


void VideoWidget::startListener (void)
{
    m_RxSocket = new QTcpSocket();
    connect(m_RxSocket, SIGNAL(connected()), this, SLOT(StreamConnected()));
    connect(m_RxSocket, SIGNAL(hostFound()), this, SLOT(hostFound()));
    connect(m_RxSocket, SIGNAL(readyRead()),this, SLOT(RxByteRead()));
    qDebug() << "connecting to video stream ...";

    TryToConnect();

}

void VideoWidget::StreamConnected()
{
    m_StreamConnected = true;
    qDebug() << "Video stream connected !";
}



void VideoWidget::StreamDeconnected()
{
    qDebug() << "Stream connection lost!";
    QTimer::singleShot(1000, this, SLOT(TryToConnect()));
}


void VideoWidget::RxByteRead()
{
    QByteArray buff = m_RxSocket->readAll();
    decodeStream(&buff);
}

void VideoWidget::TryToConnect()
{
    m_RxSocket->connectToHost(SERVER, PORT);

    // we need to wait...
    if(!m_RxSocket->waitForConnected(1000))
    {
        qDebug() << "Error: " << m_RxSocket->errorString();
        QTimer::singleShot(1000, this, SLOT(TryToConnect()));
    }

}

void VideoWidget::hostFound()
{
    qDebug() << "Video stream host found";
}


void VideoWidget::initCodec (void)
{

        avcodec_register_all();

        av_init_packet(&m_Avpkt);

          /* find the H264 video decoder */
        m_pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!m_pCodec) {
            fprintf(stderr, "codec not found\n");
            exit(1);
        }

        m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
        m_pPicture= avcodec_alloc_frame();

        m_pParser = av_parser_init(AV_CODEC_ID_H264);

        if(m_pCodec->capabilities&CODEC_CAP_TRUNCATED)
            m_pCodecCtx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

        /* For some codecs, such as msmpeg4 and mpeg4, width and height
           MUST be initialized there because this information is not
           available in the bitstream. */

        /* open it */
        if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0) {
            fprintf(stderr, "could not open codec\n");
            exit(1);
        }

}

void VideoWidget::decodeStream (QByteArray * stream)
{
    QByteArray pckt;
    QByteArray sync;
    QByteArray padding;

    sync.fill(0x00,4);
    sync[3] = 0x01;
    padding.fill(0x00, FF_INPUT_BUFFER_PADDING_SIZE);

    int idxstart = 0;
    int idxstop = 0;

    bool cont = true;
    uint8_t * datain = (uint8_t * )stream->data();
    int insize =stream->size();
    uint8_t * dataout;
    int outsize;
    int used;

    while (insize > 0)
    {
        used = av_parser_parse2(m_pParser,m_pCodecCtx,&dataout,&outsize,datain,insize,0,0,AV_NOPTS_VALUE);
        if (outsize > 0)
        {
              decodeFrame((char *)dataout,outsize);
        }
        insize -= used;
        datain += used;
    }


}

void VideoWidget::decodeFrame (char * pkt, int len)
{
    m_Avpkt.data = (uint8_t *)pkt;
    m_Avpkt.size = len;
    int declen, got_pict;
    while (m_Avpkt.size > 0)
    {
        declen = avcodec_decode_video2 (m_pCodecCtx,m_pPicture,&got_pict,&m_Avpkt);
        if (declen < 0)
        {
            qDebug() << "H264 decoding error";
            break;
        }

        else if (got_pict)
        {
            if (m_pRgbPic == NULL)
            {
                m_pRgbPic = new AVPicture;
                avpicture_alloc(m_pRgbPic, AV_PIX_FMT_RGB24, m_pPicture->width, m_pPicture->height);
            }
            if (m_pScaleCtx == NULL)
            {
                m_pScaleCtx = sws_getCachedContext (m_pScaleCtx, m_pPicture->width, m_pPicture->height, static_cast<PixelFormat>(m_pPicture->format), m_pPicture->width, m_pPicture->height, PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
            }
            sws_scale(m_pScaleCtx, m_pPicture->data, m_pPicture->linesize, 0, m_pPicture->height, m_pRgbPic->data, m_pRgbPic->linesize);

            if (m_pVideoProc != NULL)
            {
                  m_pVideoProc->process((void*) m_pRgbPic->data[0], m_pPicture->height, m_pPicture->width);
            }
            QImage img ((unsigned char *)m_pRgbPic->data[0],m_pPicture->width, m_pPicture->height,QImage::Format_RGB888);
            m_pVideoOut->setPixmap(QPixmap::fromImage(img));

        }
        m_Avpkt.data += declen;
        m_Avpkt.size -= declen;

    }

}

void VideoWidget::closeCodec (void)
{
    avcodec_close(m_pCodecCtx);
    av_free(m_pCodec);
    av_free(m_pPicture);
}
