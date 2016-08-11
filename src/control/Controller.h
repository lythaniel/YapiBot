/*
 * Controller.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "YapiBotTypes.h"

#include "Singleton.h"
#include "Thread.h"
#include "Mutex.h"
#include "EventObserver.h"
#include "YapiBotCmd.h"


class CController: public CSingleton <CController> {
public:
	CController();
	~CController();

	void CmdPckReceived (YapiBotCmd_t id, int8_t * buffer, uint32_t size);

	//int32_t toInt (int8_t * buff);

	void run (void *);

	typedef enum {
		CTRL_STATE_IDLE,
		CTRL_STATE_COMP_CAL,
		CTRL_STATE_MOVE_STRAIGHT,
		CTRL_STATE_ALIGN_BEARING,
		CTRL_STATE_MOVE_BEARING,
		CTRL_STATE_ROTATE,
		CTRL_STATE_REFRESH_MAP
	} ctrlState_t;

	void compassCalibration (void);
	void moveStraight (int32_t distance);
	void alignBearing (int32_t bearing);
	void moveBearing (int32_t bearing, int32_t distance);
	void rotate (int32_t rot);
	void refreshMap (void);

	void EventCallback (Event_t evt, int32_t data1, void * data2);

	void setParameter (YapiBotParam_t, int8_t * buffer, uint32_t size);
	void getParameter (YapiBotParam_t param);


private:

	void processCmdMove (YapiBotCmd_t cmd, int8_t * buffer, uint32_t size);
	void processCmd (YapiBotCmd_t cmd, int8_t * buffer, uint32_t size);

	void processCmdParam (YapiBotCmd_t cmd, int8_t * buffer, uint32_t size);



	void runCompassCalibration (YapiBotStatus_t status);
	void runMoveStraight (YapiBotStatus_t status);
	void runAlignBearing (YapiBotStatus_t status);
	void runMoveBearing (YapiBotStatus_t status);
	void runRotate (YapiBotStatus_t status);
	void runRefreshMap (YapiBotStatus_t status);

	CThread * m_Thread;

	ctrlState_t m_State;

	int32_t m_DistMovedLeft;
	int32_t m_DistMovedRight;
	int32_t m_BearingGoodCnt;

	int32_t m_RequestedBearing;
	int32_t m_ReqLeftMov;
	int32_t m_ReqRightMov;

	uint32_t m_CollisionDist;
	uint32_t m_MvtErrGain;
	uint32_t m_BearingErrGain;
	uint32_t m_BearingErrLim;
	uint32_t m_BearingGoodCntLim;

	YapiBotStatus_t m_Status;

	CMutex m_Lock;

};

#endif /* CONTROLLER_H_ */
