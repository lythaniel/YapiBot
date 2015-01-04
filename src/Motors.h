
#ifndef MOTORS_H_
#define MOTORS_H_

#include "Singleton.h"
#include "Thread.h"
#include "Sampler.h"


class CMotors : public CSingleton<CMotors>
{
public:
	CMotors ();
	~CMotors ();

	void move (int x, int y);
	void setLeftSpeed (int speed);
	void setRightSpeed (int speed);
	void moveCam (int x);

	int getLeftSpeed (void);
	int getRightSpeed (void);
	int getLeftMeas (void);
	int getRightMeas (void);
	int getCamPos (void) {return m_CamPos;}

	void EncoderCbLeft (int gpio, int level, unsigned int tick);
	void EncoderCbRight (int gpio, int level, unsigned int tick);

	void run (void *);

private:
	void ctrlLeftSpeed (int speed);
	void ctrlRightSpeed (int speed);

	int m_LeftSpeed;
	int m_RightSpeed;
	int m_CamPos;
	int m_LeftCounter;
	int m_RightCounter;
	int m_MeasLeftSpeed;
	int m_MeasRightSpeed;
	int m_LevelLA;
	int m_LevelLB;
	int m_LevelRA;
	int m_LevelRB;
	int m_LastGpioLeft;
	int m_LastGpioRight;

	CThread * m_Thread;

};


#endif
