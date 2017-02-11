/*
 * Compass_LSM9DS1.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef COMPASS_LSM9DS1_H_
#define COMPASS_LSM9DS1_H_

#include <Compass.h>
#include "Mutex.h"

#define LDSM9DS1_MAG_CIRC_BUFFER_SIZE 128 //MUST BE A POWER OF TWO

class CCompass_LSM9DS1: public CCompass
{
	friend class CSensorFactory;

protected:
	CCompass_LSM9DS1();
	virtual ~CCompass_LSM9DS1();

public:
	virtual void setBus (CI2Cbus * bus);
	virtual float32_t getHeading (void);
	virtual void stopCompassCalibration (sMagField offset, sMagField scale);
	virtual void startCompassCalibration ();
	virtual sMagField getMagField (void);
	virtual bool magFieldAvailable (void) {return (m_NumSample > 0);}


	virtual void dataReady (void);
private:
	int32_t m_Pi;
	int32_t m_IntCallbackId;

	/*float32_t m_MaxX;
	float32_t m_MaxY;
	float32_t m_MaxZ;
	float32_t m_MinX;
	float32_t m_MinY;
	float32_t m_MinZ;
	int16_t m_CalX;
	int16_t m_CalY;
	int16_t m_CalZ;*/

	int32_t m_NumSample;
	sMagField m_SampleBuffer [LDSM9DS1_MAG_CIRC_BUFFER_SIZE];
	int32_t m_BufferInIdx;
	int32_t m_BufferOutIdx;
	CMutex m_BufferLock;

	uint32_t m_InputSampCnt;

	sMagField m_CompCalOffset;
	sMagField m_CompCalScale;


};

#endif /* COMPASS_LSM9DS1_H_ */
