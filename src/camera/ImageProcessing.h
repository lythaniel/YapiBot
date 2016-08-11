/*
 * ImageProcessing.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef IMAGEPROCESSING_H_
#define IMAGEPROCESSING_H_

#include "Singleton.h"
#include "YapiBotTypes.h"
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

	void setParameter (YapiBotParam_t param, int8_t * buffer, uint32_t size);
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
