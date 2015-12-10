/*
 * Controller.h
 *
 *  Created on: 22 nov. 2014
 *      Author: lythaniel
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "Singleton.h"
#include "Thread.h"
#include "Mutex.h"
#include "EventObserver.h"
#include "YapiBotCmd.h"


class CController: public CSingleton <CController> {
public:
	CController();
	~CController();

	void CmdPckReceived (char * buffer, unsigned int size);

	int toInt (char * buff);

	void run (void *);

	typedef enum {
		CTRL_STATE_IDLE,
		CTRL_STATE_COMP_CAL,
		CTRL_STATE_MOVE_STRAIGHT,
		CTRL_STATE_ALIGN_BEARING,
		CTRL_STATE_MOVE_BEARING,
		CTRL_STATE_ROTATE,
	} ctrlState_t;

	void compassCalibration (void);
	void moveStraight (int distance);
	void alignBearing (int bearing);
	void moveBearing (int bearing, int distance);
	void rotate (int rot);

	void EventCallback (Event_t evt, int data1, void * data2);

	void setParameter (YapiBotParam_t, char * buffer, unsigned int size);
	void getParameter (YapiBotParam_t param);


private:

	void processCmdMove (YapiBotCmd_t cmd, char * buffer, unsigned int size);
	void processCmd (YapiBotCmd_t cmd, char * buffer, unsigned int size);

	void processCmdParam (YapiBotCmd_t cmd, char * buffer, unsigned int size);



	void runCompassCalibration (YapiBotStatus_t status);
	void runMoveStraight (YapiBotStatus_t status);
	void runAlignBearing (YapiBotStatus_t status);
	void runMoveBearing (YapiBotStatus_t status);
	void runRotate (YapiBotStatus_t status);

	CThread * m_Thread;

	ctrlState_t m_State;

	int m_DistMovedLeft;
	int m_DistMovedRight;
	int m_BearingGoodCnt;

	int m_RequestedBearing;
	int m_ReqLeftMov;
	int m_ReqRightMov;

	unsigned int m_CollisionDist;
	unsigned int m_MvtErrGain;
	unsigned int m_BearingErrGain;
	unsigned int m_BearingErrLim;
	unsigned int m_BearingGoodCntLim;

	CMutex m_Lock;

};

#endif /* CONTROLLER_H_ */
