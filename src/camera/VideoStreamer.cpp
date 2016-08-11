/*
 * VideoStreamer.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "VideoStreamer.h"
#include "Network.h"
#include "Encoder.h"
#include "EventObserver.h"

CVideoStreamer::CVideoStreamer(uint32_t width, uint32_t height,uint32_t framerate) :
m_pEncoder (NULL),
m_Width (width),
m_Height (height),
m_Framerate (framerate)
{
	m_pEncoder= new CEncoder(m_Width, m_Height,m_Framerate);
	m_pEncoder->regFrameEncodedCb(this,&CVideoStreamer::cb);
	CEventObserver::getInstance()->registerOnEvent(EVENT_MASK_NETWORK,this, &CVideoStreamer::Networkcb);
	}

CVideoStreamer::~CVideoStreamer() {
	if (m_pEncoder != NULL)
	{
		delete m_pEncoder;
	}
}

void CVideoStreamer::cb (uint8_t * buff, uint32_t size )
{
	//printf ("Send video packet\n");
	CNetwork::getInstance()->sendVideoPacket(buff, size);
}



void CVideoStreamer::Networkcb (Event_t evt, int32_t data1, void * data2)
{
	if ((evt == NetVideoClientConnected))
	{
		uint8_t * spsbuff;
		uint32_t spssize;
		uint8_t * ppsbuff;
		uint32_t ppssize;
		uint8_t * SPSPPSBuffer;
		uint32_t totalsize;
		spsbuff = m_pEncoder->getSPS(&spssize);
		ppsbuff = m_pEncoder->getPPS(&ppssize);
		if ((spsbuff != NULL) && (ppsbuff != NULL) && (spssize > 0) && (ppssize > 0))
		{
			totalsize = spssize + ppssize;

			SPSPPSBuffer = new uint8_t[totalsize];
			memcpy (SPSPPSBuffer,spsbuff,spssize);
			memcpy (SPSPPSBuffer+spssize,ppsbuff,ppssize);
			CNetwork::getInstance()->sendVideoPacket (SPSPPSBuffer, totalsize);
			fprintf(stdout,"SPSPPDBuffer sent\n");
			delete SPSPPSBuffer;

		}

	}
}


void CVideoStreamer::streamFrame (uint8_t * frame, uint32_t size)
{
	//printf ("encode video packet\n");
	m_pEncoder->encode (frame,size);
}
