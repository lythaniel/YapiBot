/*
 * VideoStreamer.h
 *
 *  Created on: 7 déc. 2014
 *      Author: lythaniel
 */

#ifndef VIDEOSTREAMER_H_
#define VIDEOSTREAMER_H_

#include <stdint.h>
#include "Network.h"
#include "EventObserver.h"

class CEncoder;

class CVideoStreamer {
public:
	CVideoStreamer(uint32_t width, uint32_t height,uint32_t framerate);
	~CVideoStreamer();

	void streamFrame (uint8_t * frame, uint32_t size);

	void cb (uint8_t * buff, uint32_t size );
	void Networkcb (Event_t evt, int data1, void * data2);

private:
	CEncoder * m_pEncoder;
	uint32_t m_Width;
	uint32_t m_Height;
	uint32_t m_Framerate;
};

#endif /* VIDEOSTREAMER_H_ */