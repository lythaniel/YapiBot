/*
 * Motors.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <pigpiod_if2.h>
#include <cstdlib>
#include "Motors.h"
#include "Network.h"
#include "Settings.h"
#include "Utils.h"

#define PWM_GPIO_R_PWM 23
#define PWM_GPIO_R_FW 24
#define	PWM_GPIO_R_RE 25

#define PWM_GPIO_L_PWM 14
#define PWM_GPIO_L_FW 18
#define	PWM_GPIO_L_RE 15

#define	PWM_GPIO_CAM 8

#define ROTENC_GPIO_LA 10
#define ROTENC_GPIO_LB 9

#define ROTENC_GPIO_RA 7
#define ROTENC_GPIO_RB 11

#define MOTORS_CONTROL

#define MOTOR_PERIOD 10

#define SPEED_CONV  1.5
#define SPEED_ERROR_GAIN  1.5
#define ACC_ERROR_GAIN 0.8

#define SPEED_MOV_AVG 5

static void GpioCbL (int32_t pi, uint32_t gpio, uint32_t level, uint32_t tick, void * user)
{
	CMotors * motor = (CMotors *) user;
	motor->EncoderCbLeft (gpio, level, tick);
}
static void GpioCbR (int32_t pi, uint32_t gpio, uint32_t level, uint32_t tick, void * user)
{
	CMotors * motor = (CMotors *) user;
	motor->EncoderCbRight (gpio, level, tick);
}


CMotors::CMotors () :
m_LeftSpeed(0),
m_RightSpeed(0),
m_LeftCounter(0),
m_RightCounter(0),
m_MeasLeftSpeed(0),
m_MeasRightSpeed(0),
m_LevelLA(0),
m_LevelLB(0),
m_LevelRA(0),
m_LevelRB(0),
m_LastGpioLeft(-1),
m_LastGpioRight(-1),
m_CamPos(50),
m_DistLeft(0),
m_DistRight(0),
m_SpeedConv(SPEED_CONV),
m_SpeedErrGain(SPEED_ERROR_GAIN),
m_AccErrGain(ACC_ERROR_GAIN)
{
	int32_t ret = 0;
	m_SpeedConv = CSettings::getInstance()->getFloat("MOTORS", "Speed conversion", SPEED_CONV);
	m_SpeedErrGain = CSettings::getInstance()->getFloat("MOTORS", "Speed error gain", SPEED_ERROR_GAIN);
	m_AccErrGain = CSettings::getInstance()->getFloat("MOTORS", "Acceleration error gain", ACC_ERROR_GAIN);

#ifdef MOTORS_CONTROL
	m_Pi = pigpio_start(NULL,NULL); //local gpio
	if (m_Pi < 0)
	{
		fprintf(stderr, "[GPIO] Error could not initialize  (pi = %d", m_Pi);
		return;
	}
	ret += set_mode (m_Pi, PWM_GPIO_R_PWM,PI_OUTPUT);
	ret += set_mode (m_Pi, PWM_GPIO_R_FW,PI_OUTPUT);
	ret += set_mode (m_Pi, PWM_GPIO_R_RE,PI_OUTPUT);

	ret += set_mode (m_Pi, PWM_GPIO_L_PWM,PI_OUTPUT);
	ret += set_mode (m_Pi, PWM_GPIO_L_FW,PI_OUTPUT);
	ret += set_mode (m_Pi, PWM_GPIO_L_RE,PI_OUTPUT);

	ret += set_mode (m_Pi, PWM_GPIO_CAM,PI_OUTPUT);

	if (ret != 0)
	{
		fprintf(stderr, "[GPIO] Set mode failed");
		return;
	}

	ret = set_PWM_frequency (m_Pi, PWM_GPIO_R_PWM, 100);
	if (ret < 0)
	{
		fprintf(stderr, "[GPIO] Set PWM frequency failed");
		return;
	}
	else if (ret != 100)
	{
		fprintf(stderr, "[GPIO] Warning PWM frequency different from requested (freq = %d)",ret);
	}
	ret = set_PWM_frequency (m_Pi, PWM_GPIO_L_PWM, 100);
	if (ret < 0)
	{
		fprintf(stderr, "[GPIO] Set PWM frequency failed");
		return;
	}
	else if (ret != 100)
	{
		fprintf(stderr, "[GPIO] Warning PWM frequency different from requested (freq = %d)",ret);
	}


	ret = set_PWM_range (m_Pi, PWM_GPIO_R_PWM,100);
	ret += set_PWM_range (m_Pi, PWM_GPIO_L_PWM,100);
	/*if (ret != 0)
	{
		fprintf(stderr, "[GPIO] Set PWM range failed");
		return;
	}*/

	gpio_write(m_Pi, PWM_GPIO_R_FW, 0);
	gpio_write(m_Pi, PWM_GPIO_R_RE, 0);
	gpio_write(m_Pi, PWM_GPIO_L_FW, 0);
	gpio_write(m_Pi, PWM_GPIO_L_RE, 0);


	set_PWM_dutycycle (m_Pi, PWM_GPIO_R_PWM,100);
	set_PWM_dutycycle (m_Pi, PWM_GPIO_L_PWM,100);


	set_servo_pulsewidth (m_Pi, PWM_GPIO_CAM, 1450);

	ret  = set_mode (m_Pi, ROTENC_GPIO_LA, PI_INPUT);
	ret += set_mode (m_Pi, ROTENC_GPIO_LB, PI_INPUT);
	ret += set_mode (m_Pi, ROTENC_GPIO_RA, PI_INPUT);
	ret += set_mode (m_Pi, ROTENC_GPIO_RB, PI_INPUT);
	if (ret != 0)
	{
		fprintf(stderr, "[GPIO] Set mode failed for encoders input");
		return;
	}

	ret += set_pull_up_down(m_Pi, ROTENC_GPIO_LA, PI_PUD_UP);
	ret += set_pull_up_down(m_Pi, ROTENC_GPIO_LB, PI_PUD_UP);
	ret += set_pull_up_down(m_Pi, ROTENC_GPIO_RA, PI_PUD_UP);
	ret += set_pull_up_down(m_Pi, ROTENC_GPIO_RB, PI_PUD_UP);
	if (ret != 0)
	{
		fprintf(stderr, "[GPIO] Set pull up failed");
		return;
	}

	m_EncCallbackId[0] = callback_ex(m_Pi, ROTENC_GPIO_LA, EITHER_EDGE, GpioCbL, (void *)this);
	m_EncCallbackId[1] = callback_ex(m_Pi, ROTENC_GPIO_LB, EITHER_EDGE, GpioCbL, (void *)this);
	m_EncCallbackId[2] = callback_ex(m_Pi, ROTENC_GPIO_RA, EITHER_EDGE, GpioCbR, (void *)this);
	m_EncCallbackId[3] = callback_ex(m_Pi, ROTENC_GPIO_RB, EITHER_EDGE, GpioCbR, (void *)this);

	m_Thread = new CThread ();
	m_Thread->regThreadProcess(this, &CMotors::run);
	m_Thread->start();
#endif
}

CMotors::~CMotors ()
{
#ifdef MOTORS_CONTROL
	gpio_write (m_Pi, PWM_GPIO_R_FW, 0);
	gpio_write (m_Pi, PWM_GPIO_R_RE, 0);
	gpio_write (m_Pi, PWM_GPIO_L_FW, 0);
	gpio_write (m_Pi, PWM_GPIO_L_RE, 0);
	set_PWM_dutycycle (m_Pi, PWM_GPIO_R_PWM,100);
	set_PWM_dutycycle (m_Pi, PWM_GPIO_L_PWM,100);
	set_servo_pulsewidth (m_Pi, PWM_GPIO_CAM, 1450);


	callback_cancel(m_EncCallbackId[0]);
	callback_cancel(m_EncCallbackId[1]);
	callback_cancel(m_EncCallbackId[2]);
	callback_cancel(m_EncCallbackId[3]);

	pigpio_stop(m_Pi);
#endif
}

void CMotors::move (int32_t x, int32_t y)
{

	if (x > 100)
		x= 100;
	else if (x < -100)
		x = -100;

	if (y > 100)
		y= 100;
	else if (y < -100)
		y = -100;

	int32_t speed_left = x-y;
	int32_t speed_right = x+y;


	setLeftSpeed(speed_left);
	setRightSpeed(speed_right);
}
void CMotors::setLeftSpeed (int32_t speed)
{
	if (speed > 100)
		speed = 100;
	else if (speed < -100)
		speed = -100;
	m_Lock.get();
	m_LeftSpeed = speed;
	m_Lock.release();
}


void CMotors::setRightSpeed (int32_t speed)
{
	if (speed > 100)
		speed = 100;
	else if (speed < -100)
		speed = -100;
	m_Lock.get();
	m_RightSpeed = speed;
	m_Lock.release();
}

void CMotors::ctrlLeftSpeed (int32_t speed)
{
#ifdef MOTORS_CONTROL
	if (abs(speed) <= 10)
	{
		gpio_write (m_Pi, PWM_GPIO_L_FW, 1);
		gpio_write (m_Pi, PWM_GPIO_L_RE, 1);
		set_PWM_dutycycle (m_Pi, PWM_GPIO_L_PWM,99);

	}
	else
	{

		if (speed > 0)
		{
			gpio_write (m_Pi, PWM_GPIO_L_FW, 1);
			gpio_write (m_Pi, PWM_GPIO_L_RE, 0);


		}
		else
		{
			gpio_write (m_Pi, PWM_GPIO_L_FW, 0);
			gpio_write (m_Pi, PWM_GPIO_L_RE, 1);
		}
		speed = abs(speed);
		if (speed > 100)
		{
				speed = 100;
		}
		set_PWM_dutycycle (m_Pi, PWM_GPIO_L_PWM,speed);
	}
#endif
}


void CMotors::ctrlRightSpeed (int32_t speed)
{
#ifdef MOTORS_CONTROL
	if (abs(speed) <= 10)
	{
		gpio_write (m_Pi, PWM_GPIO_R_FW, 1);
		gpio_write (m_Pi, PWM_GPIO_R_RE, 1);
		set_PWM_dutycycle (m_Pi, PWM_GPIO_R_PWM,99);

	}
	else
	{

		if (speed > 0)
		{
			gpio_write (m_Pi, PWM_GPIO_R_FW, 1);
			gpio_write (m_Pi, PWM_GPIO_R_RE, 0);
		}
		else
		{
			gpio_write (m_Pi, PWM_GPIO_R_FW, 0);
			gpio_write (m_Pi, PWM_GPIO_R_RE, 1);
		}
		speed = abs(speed);
		if (speed > 100)
		{
				speed = 100;
		}
		set_PWM_dutycycle (m_Pi, PWM_GPIO_R_PWM,speed);
	}
#endif
	}

void CMotors::moveCam (int32_t pos)
{
	if (pos < 0)
		pos = 0;
	if (pos > 100)
		pos = 100;
	int32_t servpos = (pos * 130) / 10;
	servpos += 800;
#ifdef MOTORS_CONTROL
	set_servo_pulsewidth (m_Pi, PWM_GPIO_CAM, servpos);
#endif
	m_Lock.get();
	m_CamPos = pos;
	m_Lock.release();
}

int32_t CMotors::getLeftSpeed (void)
{
	int32_t ret;

	m_Lock.get();
	ret = m_LeftSpeed;
	m_Lock.release();

	return ret;
}
int32_t CMotors::getRightSpeed (void)
{
	int32_t ret;

	m_Lock.get();
	ret = m_RightSpeed;
	m_Lock.release();

	return ret;
}

int32_t CMotors::getLeftMeas (void)
{
	int32_t ret;

	m_Lock.get();
	ret = m_MeasLeftSpeed;
	m_Lock.release();

	return ret;
}
int32_t CMotors::getRightMeas (void)
{
	int32_t ret;

	m_Lock.get();
	ret = m_MeasRightSpeed;
	m_Lock.release();

	return ret;
}

int32_t CMotors::getLeftDist (void)
{
	int32_t ret;

	m_Lock.get();
	ret = m_DistLeft;
	m_DistLeft = 0;
	m_Lock.release();

	return ret;
}

int32_t CMotors::getRightDist (void)
{
	int32_t ret;

	m_Lock.get();
	ret = m_DistRight;
	m_DistRight = 0;
	m_Lock.release();

	return ret;
}

void CMotors::resetDist (void)
{
	m_Lock.get();
	m_DistLeft = 0;
	m_DistRight = 0;
	m_Lock.release();
}


void CMotors::EncoderCbLeft (int32_t gpio, int32_t level, uint32_t tick)
{

	if (m_LastGpioLeft != gpio)
	{
		//printf ("Encoder callback:%d %d\n", gpio, level);
		m_LastGpioLeft = gpio;
		if (gpio == ROTENC_GPIO_LA)
		{
			if (level == 1)
			{
				(m_LevelLB == 1)?(m_LeftCounter++):(m_LeftCounter--);
			}
			else
			{
				(m_LevelLB == 0)?(m_LeftCounter++):(m_LeftCounter--);
			}
			m_LevelLA = level;
		}
		else if (gpio == ROTENC_GPIO_LB)
		{
			if (level == 1)
			{
				(m_LevelLA == 0)?(m_LeftCounter++):(m_LeftCounter--);
			}
			else
			{
				(m_LevelLA == 1)?(m_LeftCounter++):(m_LeftCounter--);
			}
			m_LevelLB = level;
		}

	}

}

void CMotors::EncoderCbRight(int32_t gpio, int32_t level, uint32_t tick)
{

	if (m_LastGpioRight != gpio)
	{
		//printf ("Encoder callback:%d %d\n", gpio, level);
		m_LastGpioRight = gpio;
		if (gpio == ROTENC_GPIO_RA)
		{
			if (level == 1)
			{
				(m_LevelRB == 1)?(m_RightCounter++):(m_RightCounter--);
			}
			else
			{
				(m_LevelRB == 0)?(m_RightCounter++):(m_RightCounter--);
			}
			m_LevelRA = level;
		}
		else if (gpio == ROTENC_GPIO_RB)
		{
			if (level == 1)
			{
				(m_LevelRA == 0)?(m_RightCounter++):(m_RightCounter--);
			}
			else
			{
				(m_LevelRA == 1)?(m_RightCounter++):(m_RightCounter--);
			}
			m_LevelRB = level;
		}
	}

}


void CMotors::run (void *)
{
	CSampler sampler (MOTOR_PERIOD);
	float32_t fSpeedErrL = 0;
	float32_t fSpeedErrR = 0;

	float32_t fPrevComdL = 0;
	float32_t fPrevComdR = 0;

	int32_t iPrevSpeedL = 0;
	int32_t iPrevSpeedR = 0;

	float32_t fAccConsL = 0;
	float32_t fAccConsR = 0;

	float32_t fAccL = 0;
	float32_t fAccR = 0;

	float32_t fAccErrL = 0;
	float32_t fAccErrR = 0;
	int32_t MeasSpeedL [SPEED_MOV_AVG] = {0};
	int32_t MeasSpeedR [SPEED_MOV_AVG] = {0};
	int32_t MeasSpeedidx = 0;

	while (1)
	{
		sampler.wait();
		m_Lock.get();
		m_DistLeft += m_LeftCounter;
		m_DistRight += m_RightCounter;
		m_MeasLeftSpeed -= MeasSpeedL[MeasSpeedidx];
		m_MeasRightSpeed -= MeasSpeedR[MeasSpeedidx];

		m_MeasLeftSpeed += m_LeftCounter*2;
		m_MeasRightSpeed += m_RightCounter*2;

		MeasSpeedL[MeasSpeedidx] = m_LeftCounter*2;
		MeasSpeedR[MeasSpeedidx++] = m_RightCounter*2;
		if (MeasSpeedidx >= SPEED_MOV_AVG)
		{
			MeasSpeedidx = 0;
		}

		fAccL = m_MeasLeftSpeed - iPrevSpeedL;
		fAccR = m_MeasRightSpeed - iPrevSpeedR;

		iPrevSpeedL = m_MeasLeftSpeed;
		iPrevSpeedR = m_MeasRightSpeed;

		fSpeedErrL = ((float32_t)m_LeftSpeed*m_SpeedConv - (float32_t)m_MeasLeftSpeed) * m_SpeedErrGain;
		fSpeedErrR = ((float32_t)m_RightSpeed*m_SpeedConv- (float32_t)m_MeasRightSpeed) * m_SpeedErrGain;

		fAccConsL = fSpeedErrL;
		fAccConsR = fSpeedErrR;

		fAccErrL = (fAccConsL - fAccL) * m_AccErrGain;
		fAccErrR = (fAccConsR - fAccR) * m_AccErrGain;

		fPrevComdL = fAccErrL;
		fPrevComdR = fAccErrR;

		ctrlLeftSpeed((int32_t)fPrevComdL);
		ctrlRightSpeed((int32_t)fPrevComdR);

		m_LeftCounter = 0;
		m_RightCounter = 0;
		m_Lock.release();
		//printf ("Motor speed Consigne: %d/%d measured %d/%d err %f,%f\n", m_LeftSpeed, m_RightSpeed, m_MeasLeftSpeed,m_MeasRightSpeed, fSpeedErrL, fSpeedErrR);
		//printf ("Motor torque Consigne: %f/%f measured %f/%f err %f,%f cmd %f,%f \n", fAccConsL, fAccConsR, fAccL,fAccR, fAccErrL, fAccErrR, fPrevComdL, fPrevComdR);

	}
}

void CMotors::setParameter (YapiBotParam_t param, int8_t * buffer, uint32_t size)
{
	uint32_t val;
	if (size < 4)
	{
		fprintf (stderr, "Cannot set controller parameter (not enough arguments)");
	}



	switch (param)
	{
		case MtrParamSpeedConv:
			m_SpeedConv = Utils::toFloat (&buffer[0]);
			CSettings::getInstance()->setFloat("MOTORS", "Speed conversion", m_SpeedConv);
			break;
		case MtrParamSpeedErrGain:
			m_SpeedErrGain = Utils::toFloat (&buffer[0]);
			CSettings::getInstance()->setFloat("MOTORS", "Speed error gain", m_SpeedErrGain);
			break;
		case MtrParamAccErrGain:
			m_AccErrGain = Utils::toFloat (&buffer[0]);
			CSettings::getInstance()->setFloat("MOTORS", "Acceleration error gain", m_AccErrGain);
			break;
		default:
			fprintf (stderr, "Unknown motor parameter !");
			break;
	}
}

void CMotors::getParameter (YapiBotParam_t param)
{
	YapiBotParamAnswer_t answer;

	Utils::fromInt((int32_t)param, &answer.param);

	switch (param)
	{
		case MtrParamSpeedConv:
			Utils::fromFloat (m_SpeedConv, &answer.val);
			break;
		case MtrParamSpeedErrGain:
			Utils::fromFloat (m_SpeedErrGain, &answer.val);
			break;
		case MtrParamAccErrGain:
			Utils::fromFloat (m_AccErrGain, &answer.val);
			break;

		default:
			fprintf (stderr, "Unknown motor parameter !");
			return;
	}
	CNetwork::getInstance()->sendCmdPck (CmdInfoParam, (uint8_t *)&answer, sizeof(YapiBotParamAnswer_t));
}


