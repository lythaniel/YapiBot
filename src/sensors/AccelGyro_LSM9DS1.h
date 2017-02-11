/*
 * AccelGyro_LSM9DS1.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef ACCELGYRO_LSM9DS1_H_
#define ACCELGYRO_LSM9DS1_H_

#include <Accelerometer.h>
#include <Gyroscope.h>
#include "Mutex.h"

#define SAMPLES_CIRC_BUFFER_SIZE 128  //MUST BE A POWER OF TWO !!
#define LSM9DS1_FIFO_SIZE 32

class CAccelGyro_LSM9DS1: public CAccelerometer, public CGyroscope
{
	friend class CSensorFactory;

protected:
	CAccelGyro_LSM9DS1();
	~CAccelGyro_LSM9DS1();

public:
	virtual void setBus (CI2Cbus * bus);
	virtual sAccel getAccel (void);
	virtual sAngularRate getAngularRate (void);

	virtual bool accelSamplesAvailable (void) {return (m_NumAccelSamples > 0);}
	virtual bool angRateSamplesAvailable (void) {return (m_NumAngRateSamples > 0);}

	virtual void startAccelCalibration (void);
	virtual void startGyroCalibration (void);
	virtual void stopAccelCalibration (sAccel offset, sAccel scale);
	virtual void stopGyroCalibration (sAngularRate offset, sAngularRate scale);


	void fifoReady (void);
private:
	bool m_Initialized;

	int32_t m_Pi;
	int32_t m_IntCallbackId [2];
	int16_t m_RawSamples[6*LSM9DS1_FIFO_SIZE];
	int32_t m_NumAccelSamples;;
	int32_t m_NumAngRateSamples;
	sAccel m_AccelBuffer[SAMPLES_CIRC_BUFFER_SIZE];
	sAngularRate m_AngRateBuffer[SAMPLES_CIRC_BUFFER_SIZE];
	int32_t m_AccelBufferInIdx;
	int32_t m_AngRateBufferInIdx;
	int32_t m_AccelBufferOutIdx;
	int32_t m_AngRateBufferOutIdx;
	CMutex m_BufferLock;
	sAngularRate m_AverageAngRate;
	sAngularRate m_GyroCalOffset;
	sAngularRate m_GyroCalScale;
	sAccel m_AccelCalOffset;
	sAccel m_AccelCalScale;


};

#endif /* ACCELGYRO_LSM9DS1_H_ */
