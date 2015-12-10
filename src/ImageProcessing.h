/*
 * ImageProcessing.h
 *
 *  Created on: 7 déc. 2014
 *      Author: lythaniel
 */

#ifndef IMAGEPROCESSING_H_
#define IMAGEPROCESSING_H_

#include "Singleton.h"
#include <stdint.h>
#include "YapiBotCmd.h"

class CCamera;
class CVideoStreamer;
class CThread;

class CImageProcessing: public CSingleton<CImageProcessing> {
public:
	CImageProcessing();

	~CImageProcessing();

	void VideoProcThread (void *);
	void init(uint32_t width, uint32_t height,uint32_t framerate);

	void setParameter (YapiBotParam_t param, char * buffer, unsigned int size);
	void getParameter (YapiBotParam_t param);


private:

	CCamera * m_pCamera;
	CVideoStreamer * m_pVideoStream;
	uint32_t m_Width;
	uint32_t m_Height;
	uint32_t m_Framerate;

	CThread * m_pVideoProcThread;

	uint8_t * m_OutputFrameBuffer;


};

#endif /* IMAGEPROCESSING_H_ */
