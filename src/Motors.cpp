#include <pigpio.h>
#include <cstdlib>
#include "Motors.h"

#define PWM_GPIO_R_PWM 23
#define PWM_GPIO_R_FW 25
#define	PWM_GPIO_R_RE 24

#define PWM_GPIO_L_PWM 14
#define PWM_GPIO_L_FW 15
#define	PWM_GPIO_L_RE 18

#define	PWM_GPIO_CAM 8

#define ROTENC_GPIO_LA 11
#define ROTENC_GPIO_LB 10

#define ROTENC_GPIO_RA 9
#define ROTENC_GPIO_RB 7

#define MOTORS_CONTROL

#define MOTOR_PERIOD 100

#define MOTOR_FEEDBACK_GAIN 1

static void GpioCbL (int gpio, int level, unsigned int tick, void * user)
{
	CMotors * motor = (CMotors *) user;
	motor->EncoderCbLeft (gpio, level, tick);
}
static void GpioCbR (int gpio, int level, unsigned int tick, void * user)
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
m_DistRight(0)
{
#ifdef MOTORS_CONTROL
	gpioInitialise();
	gpioSetMode (PWM_GPIO_R_PWM,PI_OUTPUT);
	gpioSetMode (PWM_GPIO_R_FW,PI_OUTPUT);
	gpioSetMode (PWM_GPIO_R_RE,PI_OUTPUT);

	gpioSetMode (PWM_GPIO_L_PWM,PI_OUTPUT);
	gpioSetMode (PWM_GPIO_L_FW,PI_OUTPUT);
	gpioSetMode (PWM_GPIO_L_RE,PI_OUTPUT);

	gpioSetPWMrange (PWM_GPIO_R_PWM,100);
	gpioSetPWMrange (PWM_GPIO_L_PWM,100);

	gpioWrite(PWM_GPIO_R_FW, 0);
	gpioWrite(PWM_GPIO_R_RE, 0);
	gpioWrite(PWM_GPIO_L_FW, 0);
	gpioWrite(PWM_GPIO_L_RE, 0);
	gpioPWM (PWM_GPIO_R_PWM,100);
	gpioPWM (PWM_GPIO_L_PWM,100);

	gpioSetMode (PWM_GPIO_CAM,PI_OUTPUT);
	gpioServo(PWM_GPIO_CAM, 1450);

	gpioSetMode(ROTENC_GPIO_LA, PI_INPUT);
	gpioSetMode(ROTENC_GPIO_LB, PI_INPUT);
	gpioSetMode(ROTENC_GPIO_RA, PI_INPUT);
	gpioSetMode(ROTENC_GPIO_RB, PI_INPUT);

	gpioSetPullUpDown(ROTENC_GPIO_LA, PI_PUD_UP);
	gpioSetPullUpDown(ROTENC_GPIO_LB, PI_PUD_UP);
	gpioSetPullUpDown(ROTENC_GPIO_RA, PI_PUD_UP);
	gpioSetPullUpDown(ROTENC_GPIO_RB, PI_PUD_UP);

	gpioSetAlertFuncEx(ROTENC_GPIO_LA, GpioCbL, (void *)this);
	gpioSetAlertFuncEx(ROTENC_GPIO_LB, GpioCbL, (void *)this);
	gpioSetAlertFuncEx(ROTENC_GPIO_RA, GpioCbR, (void *)this);
	gpioSetAlertFuncEx(ROTENC_GPIO_RB, GpioCbR, (void *)this);

	m_Thread = new CThread ();
	m_Thread->regThreadProcess(this, &CMotors::run);
	m_Thread->start();
#endif
}

CMotors::~CMotors ()
{
#ifdef MOTORS_CONTROL
	gpioWrite(PWM_GPIO_R_FW, 0);
	gpioWrite(PWM_GPIO_R_RE, 0);
	gpioWrite(PWM_GPIO_L_FW, 0);
	gpioWrite(PWM_GPIO_L_RE, 0);
	gpioPWM (PWM_GPIO_R_PWM,100);
	gpioPWM (PWM_GPIO_L_PWM,100);
	gpioServo(PWM_GPIO_CAM, 1450);

	gpioSetAlertFunc(ROTENC_GPIO_LA, NULL);
	gpioSetAlertFunc(ROTENC_GPIO_LB, NULL);
	gpioSetAlertFunc(ROTENC_GPIO_RA, NULL);
	gpioSetAlertFunc(ROTENC_GPIO_RB, NULL);

	gpioTerminate();
#endif
}

void CMotors::move (int x, int y)
{

	if (x > 100)
		x= 100;
	else if (x < -100)
		x = -100;

	if (y > 100)
		y= 100;
	else if (y < -100)
		y = -100;

	int speed_left = x-y;
	int speed_right = x+y;


	setLeftSpeed(speed_left);
	setRightSpeed(speed_right);
}
void CMotors::setLeftSpeed (int speed)
{
	if (speed > 100)
		speed = 100;
	else if (speed < -100)
		speed = -100;
	m_Lock.get();
	m_LeftSpeed = speed;
	m_Lock.release();
}


void CMotors::setRightSpeed (int speed)
{
	if (speed > 100)
		speed = 100;
	else if (speed < -100)
		speed = -100;
	m_Lock.get();
	m_RightSpeed = speed;
	m_Lock.release();
}

void CMotors::ctrlLeftSpeed (int speed)
{
#ifdef MOTORS_CONTROL
	if (abs(speed) <= 10)
	{
		gpioWrite(PWM_GPIO_L_FW, 1);
		gpioWrite(PWM_GPIO_L_RE, 1);
		gpioPWM (PWM_GPIO_L_PWM,99);

	}
	else
	{

		if (speed > 0)
		{
			gpioWrite(PWM_GPIO_L_FW, 1);
			gpioWrite(PWM_GPIO_L_RE, 0);


		}
		else
		{
			gpioWrite(PWM_GPIO_L_FW, 0);
			gpioWrite(PWM_GPIO_L_RE, 1);
		}
		speed = abs(speed);
		if (speed > 100)
				speed = 100;
		gpioPWM (PWM_GPIO_L_PWM,speed);
	}
#endif
}


void CMotors::ctrlRightSpeed (int speed)
{
#ifdef MOTORS_CONTROL
	if (abs(speed) <= 10)
	{
		gpioWrite(PWM_GPIO_R_FW, 1);
		gpioWrite(PWM_GPIO_R_RE, 1);
		gpioPWM (PWM_GPIO_R_PWM,99);

	}
	else
	{

		if (speed > 0)
		{
			gpioWrite(PWM_GPIO_R_FW, 1);
			gpioWrite(PWM_GPIO_R_RE, 0);
		}
		else
		{
			gpioWrite(PWM_GPIO_R_FW, 0);
			gpioWrite(PWM_GPIO_R_RE, 1);
		}
		speed = abs(speed);
		if (speed > 100)
				speed = 100;
		gpioPWM (PWM_GPIO_R_PWM,speed);
	}
#endif
	}

void CMotors::moveCam (int pos)
{
	if (pos < 0)
		pos = 0;
	if (pos > 100)
		pos = 100;
	int servpos = (pos * 130) / 10;
	servpos += 800;
#ifdef MOTORS_CONTROL
	gpioServo(PWM_GPIO_CAM, servpos);
#endif
	m_Lock.get();
	m_CamPos = pos;
	m_Lock.release();
}

int CMotors::getLeftSpeed (void)
{
	int ret;

	m_Lock.get();
	ret = m_LeftSpeed;
	m_Lock.release();

	return ret;
}
int CMotors::getRightSpeed (void)
{
	int ret;

	m_Lock.get();
	ret = m_RightSpeed;
	m_Lock.release();

	return ret;
}

int CMotors::getLeftMeas (void)
{
	int ret;

	m_Lock.get();
	ret = m_MeasLeftSpeed;
	m_Lock.release();

	return ret;
}
int CMotors::getRightMeas (void)
{
	int ret;

	m_Lock.get();
	ret = m_MeasRightSpeed;
	m_Lock.release();

	return ret;
}

int CMotors::getLeftDist (void)
{
	int ret;

	m_Lock.get();
	ret = m_DistLeft;
	m_DistLeft = 0;
	m_Lock.release();

	return ret;
}

int CMotors::getRightDist (void)
{
	int ret;

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


void CMotors::EncoderCbLeft (int gpio, int level, unsigned int tick)
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

void CMotors::EncoderCbRight(int gpio, int level, unsigned int tick)
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
	int lefterr = 0;
	int righterr = 0;
	while (1)
	{
		sampler.wait();
		m_Lock.get();
		m_DistLeft += m_LeftCounter;
		m_DistRight += m_RightCounter;
		m_MeasLeftSpeed = m_LeftCounter;
		m_MeasRightSpeed = m_RightCounter;
		lefterr += (m_LeftSpeed  - (m_MeasLeftSpeed*10)) / MOTOR_FEEDBACK_GAIN;
		righterr += (m_RightSpeed - (m_MeasRightSpeed*10)) /MOTOR_FEEDBACK_GAIN;
		ctrlLeftSpeed(m_LeftSpeed);
		ctrlRightSpeed(m_RightSpeed);
		m_LeftCounter = 0;
		m_RightCounter = 0;
		m_Lock.release();
		//printf ("Motor speed %d/%d\n", m_MeasLeftSpeed,m_MeasRightSpeed);

	}
}

