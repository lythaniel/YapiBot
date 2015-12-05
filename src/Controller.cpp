/*
 * Controller.cpp
 *
 *  Created on: 22 nov. 2014
 *      Author: lythaniel
 */

#include "Controller.h"
#include "Network.h"
#include "Motors.h"
#include "Sampler.h"
#include "Compass.h"
#include "RangeFinder.h"
#include "ScriptEngine.h"
#include <stdio.h>
#include <stdlib.h>

#define CONTROLLER_PERIOD 250

#define BEARING_ERR_GAIN 5
#define BEARING_ERR_LIM 4
#define BEARING_GOOD_CNT_LIMIT 3
#define BEARING_ERR_CLIP 200

#define MOVE_STRAIGHT_ERR_GAIN 1

#define COLLISION_MIN_DIST 15

CController::CController() :
m_State(CTRL_STATE_IDLE),
m_DistMovedLeft(0),
m_DistMovedRight(0),
m_RequestedBearing(0),
m_ReqLeftMov(0),
m_ReqRightMov(0)
{
	CNetwork::getInstance()->regCmdReceived(this,&CController::CmdPckReceived);
	CEventObserver::getInstance()->registerOnEvent(EVENT_MASK_ALL,this,&CController::EventCallback);
	m_Thread = new CThread ();
	m_Thread->regThreadProcess(this, &CController::run);
	m_Thread->start();
}

CController::~CController() {

}

void CController::run(void *)
{
	CSampler sampler (CONTROLLER_PERIOD);
	PoBotStatus_t status;
	status.id = POBOT_STATUS;
	while (1) {
		sampler.wait();
		CMotors * motors = CMotors::getInstance();
		CCompass * compass = CCompass::getInstance();
		CRangeFinder * rangeFinder = CRangeFinder::getInstance();

		status.speed_left = motors->getLeftSpeed();
		status.speed_right = motors->getRightSpeed();
		status.camera_pos = motors->getCamPos();
		status.heading = compass->getHeading();
		status.range = rangeFinder->getRange();
		status.meas_left = motors->getLeftMeas();
		status.meas_right = motors->getRightMeas();

		m_Lock.get();
		switch (m_State)
		{
			case CTRL_STATE_IDLE:
				break;
			case CTRL_STATE_COMP_CAL:
				runCompassCalibration(status);
				break;
			case CTRL_STATE_MOVE_STRAIGHT:
				runMoveStraight(status);
				break;
			case CTRL_STATE_ALIGN_BEARING:
				runAlignBearing(status);
				break;
			case CTRL_STATE_MOVE_BEARING:
				runMoveBearing(status);
				break;
			case CTRL_STATE_ROTATE:
				runRotate(status);
				break;
		}
		m_Lock.release();

		CNetwork::getInstance()->sendCmdPck ((unsigned char *)&status, sizeof(PoBotStatus_t));
	}

}


void CController::runCompassCalibration (PoBotStatus_t status)
{
	//First we need to align the sensor by rotating at least 2 turns.
	if ((m_ReqLeftMov != 0)||(m_ReqRightMov != 0))
	{
		m_DistMovedLeft += CMotors::getInstance()->getLeftDist();
		m_DistMovedRight += CMotors::getInstance()->getRightDist();
		if ((abs(m_DistMovedLeft) > abs(m_ReqLeftMov))&&(abs(m_DistMovedRight) > abs(m_ReqRightMov)))
		{
			fprintf(stdout,"Rotation complete\n");
			m_ReqLeftMov = 0;
			m_ReqRightMov = 0;
		}
		else
		{
			CMotors::getInstance()->move (0,100);
		}

	}
	else
	{

		//Sensor should be calibrated, let's align to the north.
		int err = status.heading;
		if (err > 180)
		{
			err -= 360;
		}

	    fprintf(stdout,"Aligning to north current = %d error = %d\n",status.heading, err);

		if (abs(err) < BEARING_ERR_LIM)
		{
			m_BearingGoodCnt++;
		}
		else
		{
			m_BearingGoodCnt = 0;
		}
		if (m_BearingGoodCnt >= BEARING_GOOD_CNT_LIMIT)
		{
			m_State = CTRL_STATE_IDLE;
			CMotors::getInstance()->move (0,0);
			fprintf(stdout,"Compass calibration complete\n");
			CEventObserver::getInstance()->notify(CtrlMoveComplete);
		}
		else
		{
			err = err * BEARING_ERR_GAIN;
			if (err > BEARING_ERR_CLIP)
				err = BEARING_ERR_CLIP;
			else if (err < -BEARING_ERR_CLIP)
				err = -BEARING_ERR_CLIP;

			CMotors::getInstance()->move (0,err);
		}



	}

}

void CController::runRotate (PoBotStatus_t status)
{
	m_DistMovedLeft += CMotors::getInstance()->getLeftDist();
	m_DistMovedRight += CMotors::getInstance()->getRightDist();
	if ((abs(m_DistMovedLeft) > abs(m_ReqLeftMov))&&(abs(m_DistMovedRight) > abs(m_ReqRightMov)))
	{
		CMotors::getInstance()->move (0,0);
		m_ReqLeftMov = 0;
		m_ReqRightMov = 0;
		fprintf(stdout,"Rotation complete\n");
		CEventObserver::getInstance()->notify(CtrlMoveComplete);
	}
	else
	{
		if (m_ReqLeftMov > 0)
		{
			CMotors::getInstance()->move (0,100);
		}
		else
		{
			CMotors::getInstance()->move (0,-100);
		}
	}
}


void CController::runMoveStraight (PoBotStatus_t status)
{
		m_DistMovedLeft += CMotors::getInstance()->getLeftDist();
		m_DistMovedRight += CMotors::getInstance()->getRightDist();
		if (status.range < COLLISION_MIN_DIST)
		{
			m_ReqLeftMov = 0;
			m_ReqRightMov = 0;
			m_State = CTRL_STATE_IDLE;
			CMotors::getInstance()->move (0,0);
			fprintf(stdout,"Move cancel due obstacle(req = %d left = %d right = %d)\n",m_ReqLeftMov,m_DistMovedLeft,m_DistMovedRight);
			CEventObserver::getInstance()->notify(CtrlMoveObstacle);

		}
		else if ((abs(m_DistMovedLeft) > abs(m_ReqLeftMov))&&(abs(m_DistMovedRight) > abs(m_ReqRightMov)))
		{
			m_ReqLeftMov = 0;
			m_ReqRightMov = 0;
			m_State = CTRL_STATE_IDLE;
			CMotors::getInstance()->move (0,0);
			fprintf(stdout,"Move complete (req = %d left = %d right = %d)\n",m_ReqLeftMov,m_DistMovedLeft,m_DistMovedRight);
			CEventObserver::getInstance()->notify(CtrlMoveComplete);
		}
		else
		{
			int err = (m_DistMovedLeft - m_DistMovedRight) * MOVE_STRAIGHT_ERR_GAIN;
			int dir = 100;
			if (m_ReqLeftMov < 0) dir = -100;

			fprintf(stdout,"Moving (req = %d) distLeft = %d,  distRight = %d, error = %d\n",m_ReqLeftMov, m_DistMovedLeft,m_DistMovedRight,err);
			if (err > 100) err = 100;
			else if (err < -100) err = -100;
			CMotors::getInstance()->move (dir,err);
		}
}
void CController::runAlignBearing (PoBotStatus_t status)
{
	//Sensor should be calibrated, let's align to the north.
		int err = status.heading - m_RequestedBearing;
		if (err > 180)
		{
			err -= 360;
		}
		if (err < -180)
		{
			err += 360;
		}

		fprintf(stdout,"Aligning to %d current = %d\n", m_RequestedBearing, err);


		if (abs(err) < BEARING_ERR_LIM)
		{
			m_BearingGoodCnt++;
		}
		else
		{
			m_BearingGoodCnt = 0;
		}
		if (m_BearingGoodCnt >= BEARING_GOOD_CNT_LIMIT)
		{
			m_State = CTRL_STATE_IDLE;
			CMotors::getInstance()->move (0,0);
			fprintf(stdout,"Bearing aligned !\n");
			CEventObserver::getInstance()->notify(CtrlMoveComplete);
		}
		else
		{
			err = err * BEARING_ERR_GAIN;
			if (err > BEARING_ERR_CLIP)
				err = BEARING_ERR_CLIP;
			else if (err < -BEARING_ERR_CLIP)
				err = -BEARING_ERR_CLIP;

			CMotors::getInstance()->move (0,err);
		}

}
void CController::runMoveBearing (PoBotStatus_t status)
{
	m_DistMovedLeft += CMotors::getInstance()->getLeftDist();
	m_DistMovedRight += CMotors::getInstance()->getRightDist();
	if (status.range < COLLISION_MIN_DIST)
	{
		m_ReqLeftMov = 0;
		m_ReqRightMov = 0;
		m_State = CTRL_STATE_IDLE;
		CMotors::getInstance()->move (0,0);
		fprintf(stdout,"Move cancel due obstacle(req = %d left = %d right = %d)\n",m_ReqLeftMov,m_DistMovedLeft,m_DistMovedRight);
		CEventObserver::getInstance()->notify(CtrlMoveObstacle);
	}
	else if ((abs(m_DistMovedLeft) > abs(m_ReqLeftMov))&&(abs(m_DistMovedRight) > abs(m_ReqRightMov)))
	{
		m_ReqLeftMov = 0;
		m_ReqRightMov = 0;
		m_State = CTRL_STATE_IDLE;
		CMotors::getInstance()->move (0,0);
		fprintf(stdout,"Move complete (req = %d left = %d right = %d)\n",m_ReqLeftMov,m_DistMovedLeft,m_DistMovedRight);
		CEventObserver::getInstance()->notify(CtrlMoveComplete);

	}
	else
	{
		int err = status.heading - m_RequestedBearing;
		if (err > 180)
		{
			err -= 360;
		}
		if (err < -180)
		{
			err += 360;
		}
		err = err * BEARING_ERR_GAIN;
		int dir = 100;
		if (m_ReqLeftMov < 0) dir = -100;

		fprintf(stdout,"Moving (dist = %d, bearing = %d) distLeft = %d,  distRight = %d, error = %d\n",m_ReqLeftMov,m_RequestedBearing, m_DistMovedLeft,m_DistMovedRight,err);
		if (err > BEARING_ERR_CLIP) err = BEARING_ERR_CLIP;
		else if (err < -BEARING_ERR_CLIP) err = -BEARING_ERR_CLIP;
		CMotors::getInstance()->move (dir,err);
	}
}

void CController::compassCalibration (void)
{
	m_Lock.get();
	if (m_State != CTRL_STATE_IDLE)
	{
		CEventObserver::getInstance()->notify(CtrlMoveCancel);
		CMotors::getInstance()->move (0,0);
	}

	m_DistMovedLeft = 0;
	m_DistMovedRight = 0;

	m_RequestedBearing = 0;
	m_ReqLeftMov = -10000;
	m_ReqRightMov = 10000;

	CMotors::getInstance()->resetDist();

	m_State = CTRL_STATE_COMP_CAL;


	m_Lock.release();
}
void CController::moveStraight (int distance)
{
	m_Lock.get();
	if (m_State != CTRL_STATE_IDLE)
	{
		CEventObserver::getInstance()->notify(CtrlMoveCancel);
		CMotors::getInstance()->move (0,0);
	}

	m_DistMovedLeft = 0;
	m_DistMovedRight = 0;

	m_RequestedBearing = 0;
	m_ReqLeftMov = distance;
	m_ReqRightMov = distance;

	CMotors::getInstance()->resetDist();

	m_State = CTRL_STATE_MOVE_STRAIGHT;

	m_Lock.release();

}
void CController::alignBearing (int bearing)
{
	m_Lock.get();
	if (m_State != CTRL_STATE_IDLE)
	{
		CEventObserver::getInstance()->notify(CtrlMoveCancel);
		CMotors::getInstance()->move (0,0);
	}

	m_RequestedBearing = bearing;

	CMotors::getInstance()->resetDist();

	m_State = CTRL_STATE_ALIGN_BEARING;

	m_Lock.release();

}
void CController::moveBearing (int bearing, int distance)
{
	m_Lock.get();
	if (m_State != CTRL_STATE_IDLE)
	{
		CEventObserver::getInstance()->notify(CtrlMoveCancel);
		CMotors::getInstance()->move (0,0);
	}

	m_DistMovedLeft = 0;
	m_DistMovedRight = 0;

	m_RequestedBearing = bearing;
	m_ReqLeftMov = distance;
	m_ReqRightMov = distance;

	CMotors::getInstance()->resetDist();

	m_State = CTRL_STATE_MOVE_BEARING;

	m_Lock.release();
}

void CController::rotate (int rot)
{
	m_Lock.get();
	if (m_State != CTRL_STATE_IDLE)
	{
		CEventObserver::getInstance()->notify(CtrlMoveCancel);
		CMotors::getInstance()->move (0,0);
	}

	m_DistMovedLeft = 0;
	m_DistMovedRight = 0;

	m_RequestedBearing = 0;
	m_ReqLeftMov = rot;
	m_ReqRightMov = -rot;

	CMotors::getInstance()->resetDist();

	m_State = CTRL_STATE_ROTATE;

	m_Lock.release();
}


void CController::CmdPckReceived (char * buffer, unsigned int size) {
	PoBotCmd_t cmd;
	PoBotCmd_t cmdType;
	if ((buffer != NULL)&&(size > 1)) //Need at least 2 bytes for the cmd.
	{
		cmd = (PoBotCmd_t)((buffer[0] << 8) | (buffer[1])); //Cmd is MSB.
		cmdType  = PoBotCmd_t(cmd & CMD_TYPE_MASK);
		switch (cmdType)
		{
		case CMD_TYPE_MOVE:
			processCmdMove(cmd,&buffer[2],size-2);
			break;
		case CMD_TYPE_CMD:
			processCmd(cmd,&buffer[2],size-2);
			break;
		case CMD_TYPE_SCRIPT:
			CScriptEngine::getInstance()->RunScript("YapiBot.py");
			break;
		default:
			fprintf(stderr,"Unknown command received !!!\n");
			break;
		}

	}
}

void CController::processCmdMove (PoBotCmd_t cmd, char * buffer, unsigned int size)
{
	int x,y,speed;

	m_Lock.get();
	if (m_State != CTRL_STATE_IDLE)
	{
		CEventObserver::getInstance()->notify(CtrlMoveCancel);
		m_State = CTRL_STATE_IDLE;
		CMotors::getInstance()->move (0,0);
	}

	m_Lock.release();
	if (size == 4)
	{
		speed = toInt (&buffer[0]);
	}
	else
	{
		speed = 0;
	}

	switch (cmd)
	{
	case CmdMoveStop:
		fprintf(stdout,"All machines stop !!!\n");
		CMotors::getInstance()->move (0,0);

		break;
	case CmdMoveFwd:

		fprintf(stdout,"Forward (%d) !!!\n", speed);
		CMotors::getInstance()->move (speed,0);

		break;
	case CmdMoveRear:
		fprintf(stdout,"Rear (%d) !!!\n", speed);
		CMotors::getInstance()->move (-speed,0);


		break;
	case CmdMoveLeft:
		fprintf(stdout,"Left (%d) !!!\n", speed);
		CMotors::getInstance()->move (0,speed);

		break;
	case CmdMoveRight:
		fprintf(stdout,"Right (%d) !!!\n", speed);
		CMotors::getInstance()->move (0,-speed);

		break;
	case CmdMove:
		if (size < 8)
		{
			fprintf(stdout,"Incorrect payload size for CmdMove\n");
			break;
		}
		x = toInt (&buffer[0]);
		y = toInt (&buffer[4]);
		fprintf(stdout,"Move x=%d y=%d\n",x , y);
		CMotors::getInstance()->move (x,y);

		break;
	case CmdMoveCam:
			if (size < 4)
			{
				fprintf(stdout,"Incorrect payload size for CmdMoveCam\n");
				break;
			}
			x = toInt (&buffer[0]);
			fprintf(stdout,"Camera Move pos=%d\n",x);
			CMotors::getInstance()->moveCam (x);

			break;
	default:
		fprintf(stderr,"Unknown move command received !!!\n");
		break;
	}

}

void CController::processCmd (PoBotCmd_t cmd, char * buffer, unsigned int size)
{
	int dist,bearing;

	m_Lock.get();
	if (m_State != CTRL_STATE_IDLE)
	{
		CEventObserver::getInstance()->notify(CtrlMoveCancel);
		m_State = CTRL_STATE_IDLE;
		CMotors::getInstance()->move (0,0);
	}
	m_Lock.release();

	switch (cmd)
	{
	case CmdCompassCal:
		m_BearingGoodCnt = 0;
		compassCalibration();
		break;

	case CmdMoveStraight:
		dist = toInt (&buffer[0]);
		moveStraight (dist);
		break;

	case CmdAlignBearing:
		m_BearingGoodCnt = 0;
		bearing = toInt (&buffer[0]);
		alignBearing (bearing);
		break;

	case CmdMoveBearing:
		dist = toInt (&buffer[0]);
		bearing = toInt (&buffer[4]);
		moveBearing(bearing,dist);
		break;
	case CmdRotate:
			dist = toInt (&buffer[0]);
			rotate(dist);
			break;
	default:
		fprintf(stderr,"Unknown command received !!!\n");
		break;
	}
}

void CController::EventCallback (Event_t evt, int data1, void * data2)
{
	fprintf(stdout,"Event received: %x, data1 = %d, data2 = %d\n",evt,data1,data2);
}


int CController::toInt (char * buff)
{
	int ret = (buff [0] << 24) + (buff [1] << 16) + (buff [2] << 8) + buff [3];
	return (ret);
}


