/*
 * Motors.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#ifndef MOTORS_H_
#define MOTORS_H_

#include "YapiBotTypes.h"

#include "Singleton.h"
#include "Thread.h"
#include "Sampler.h"
#include "Mutex.h"
#include "YapiBotCmd.h"


class CMotors : public CSingleton<CMotors>
{
public:
	CMotors ();
	~CMotors ();

	void move (int32_t x, int32_t y);
	void setLeftSpeed (int32_t speed);
	void setRightSpeed (int32_t speed);
	void moveCam (int32_t x);

	int32_t getLeftSpeed (void);
	int32_t getRightSpeed (void);
	int32_t getLeftMeas (void);
	int32_t getRightMeas (void);
	int32_t getLeftDist (void);
	int32_t getRightDist (void);

	void resetDist (void);

	int32_t getCamPos (void) {return m_CamPos;}

	void EncoderCbLeft (int32_t gpio, int32_t level, uint32_t tick);
	void EncoderCbRight (int32_t gpio, int32_t level, uint32_t tick);

	void run (void *);

	void setParameter (YapiBotParam_t, int8_t * buffer, uint32_t size);
	void getParameter (YapiBotParam_t param);


private:
	void ctrlLeftSpeed (int32_t speed);
	void ctrlRightSpeed (int32_t speed);

	//int32_t toInt (int8_t * buff);
	//float toFloat (int8_t * buff);

	int32_t m_Pi;

	int32_t m_EncCallbackId[4];

	int32_t m_LeftSpeed;
	int32_t m_RightSpeed;
	int32_t m_CamPos;
	int32_t m_LeftCounter;
	int32_t m_RightCounter;
	int32_t m_MeasLeftSpeed;
	int32_t m_MeasRightSpeed;
	int32_t m_LevelLA;
	int32_t m_LevelLB;
	int32_t m_LevelRA;
	int32_t m_LevelRB;
	int32_t m_LastGpioLeft;
	int32_t m_LastGpioRight;
	int32_t m_DistLeft;
	int32_t m_DistRight;

	float32_t m_SpeedConv;
	float32_t m_SpeedErrGain;
	float32_t m_AccErrGain;


	CMutex m_Lock;

	CThread * m_Thread;

};


#endif
