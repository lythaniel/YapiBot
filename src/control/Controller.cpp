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
#include "SensorFactory.h"
#include "ScriptEngine.h"
#include "ImageProcessing.h"
#include "Mapper.h"
#include <stdio.h>
#include <stdlib.h>

#define CONTROLLER_PERIOD 250

#define BEARING_ERR_GAIN 5
#define BEARING_ERR_LIM 4
#define BEARING_GOOD_CNT_LIMIT 3
#define BEARING_ERR_CLIP 200

#define MOVE_STRAIGHT_ERR_GAIN 1

#define COLLISION_MIN_DIST 15

#define REFRESH_MAP_SPEED 70
#define REFRESH_MAP_MIN_DIST

CController::CController() :
m_State(CTRL_STATE_IDLE),
m_DistMovedLeft(0),
m_DistMovedRight(0),
m_BearingGoodCnt(0),
m_RequestedBearing(0),
m_ReqLeftMov(0),
m_ReqRightMov(0),
m_CollisionDist(COLLISION_MIN_DIST),
m_MvtErrGain(MOVE_STRAIGHT_ERR_GAIN),
m_BearingErrGain(BEARING_ERR_GAIN),
m_BearingErrLim(BEARING_ERR_LIM),
m_BearingGoodCntLim(BEARING_GOOD_CNT_LIMIT)
{
	m_Status.id = YAPIBOT_STATUS;
	m_Status.speed_left = 0;
	m_Status.speed_right = 0;
	m_Status.camera_pos = 0;
	m_Status.heading = 0;
	m_Status.range = 0;
	m_Status.meas_left = 0;
	m_Status.meas_right = 0;
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


	while (1) {
		sampler.wait();
		CMotors * motors = CMotors::getInstance();
		CCompass * compass = CSensorFactory::getInstance()->getCompass();
		CRangeFinder * rangeFinder = CSensorFactory::getInstance()->getRangeFinder();

		m_Status.speed_left = motors->getLeftSpeed();
		m_Status.speed_right = motors->getRightSpeed();
		m_Status.camera_pos = motors->getCamPos();
		m_Status.heading = compass->getHeading();
		m_Status.range = rangeFinder->getRange();
		m_Status.meas_left = motors->getLeftMeas();
		m_Status.meas_right = motors->getRightMeas();

		m_Lock.get();
		switch (m_State)
		{
			case CTRL_STATE_IDLE:
				break;
			case CTRL_STATE_COMP_CAL:
				runCompassCalibration(m_Status);
				break;
			case CTRL_STATE_MOVE_STRAIGHT:
				runMoveStraight(m_Status);
				break;
			case CTRL_STATE_ALIGN_BEARING:
				runAlignBearing(m_Status);
				break;
			case CTRL_STATE_MOVE_BEARING:
				runMoveBearing(m_Status);
				break;
			case CTRL_STATE_ROTATE:
				runRotate(m_Status);
				break;
			case CTRL_STATE_REFRESH_MAP:
				runRefreshMap(m_Status);
		}
		m_Lock.release();

		CNetwork::getInstance()->sendCmdPck ((unsigned char *)&m_Status, sizeof(YapiBotStatus_t));
	}

}


void CController::runCompassCalibration (YapiBotStatus_t status)
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

		if (abs(err) < m_BearingErrLim)
		{
			m_BearingGoodCnt++;
		}
		else
		{
			m_BearingGoodCnt = 0;
		}
		if (m_BearingGoodCnt >= m_BearingGoodCntLim)
		{
			m_State = CTRL_STATE_IDLE;
			CMotors::getInstance()->move (0,0);
			fprintf(stdout,"Compass calibration complete\n");
			CEventObserver::getInstance()->notify(CtrlMoveComplete);
		}
		else
		{
			err = err * m_BearingErrGain;
			if (err > BEARING_ERR_CLIP)
				err = BEARING_ERR_CLIP;
			else if (err < -BEARING_ERR_CLIP)
				err = -BEARING_ERR_CLIP;

			CMotors::getInstance()->move (0,err);
		}



	}

}


void CController::runRotate (YapiBotStatus_t status)
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


void CController::runMoveStraight (YapiBotStatus_t status)
{
		m_DistMovedLeft += CMotors::getInstance()->getLeftDist();
		m_DistMovedRight += CMotors::getInstance()->getRightDist();
		if (status.range < m_CollisionDist)
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
			int err = (m_DistMovedLeft - m_DistMovedRight) * m_MvtErrGain;
			int dir = 100;
			if (m_ReqLeftMov < 0) dir = -100;

			fprintf(stdout,"Moving (req = %d) distLeft = %d,  distRight = %d, error = %d\n",m_ReqLeftMov, m_DistMovedLeft,m_DistMovedRight,err);
			if (err > 100) err = 100;
			else if (err < -100) err = -100;
			CMotors::getInstance()->move (dir,err);
		}
}
void CController::runAlignBearing (YapiBotStatus_t status)
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


		if (abs(err) < m_BearingErrLim)
		{
			m_BearingGoodCnt++;
		}
		else
		{
			m_BearingGoodCnt = 0;
		}
		if (m_BearingGoodCnt >= m_BearingGoodCntLim)
		{
			m_State = CTRL_STATE_IDLE;
			CMotors::getInstance()->move (0,0);
			fprintf(stdout,"Bearing aligned !\n");
			CEventObserver::getInstance()->notify(CtrlMoveComplete);
		}
		else
		{
			err = err * m_BearingErrGain;
			if (err > BEARING_ERR_CLIP)
				err = BEARING_ERR_CLIP;
			else if (err < -BEARING_ERR_CLIP)
				err = -BEARING_ERR_CLIP;

			CMotors::getInstance()->move (0,err);
		}

}
void CController::runMoveBearing (YapiBotStatus_t status)
{
	m_DistMovedLeft += CMotors::getInstance()->getLeftDist();
	m_DistMovedRight += CMotors::getInstance()->getRightDist();
	if (status.range < m_CollisionDist)
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
		err = err * m_BearingErrGain;
		int dir = 100;
		if (m_ReqLeftMov < 0) dir = -100;

		fprintf(stdout,"Moving (dist = %d, bearing = %d) distLeft = %d,  distRight = %d, error = %d\n",m_ReqLeftMov,m_RequestedBearing, m_DistMovedLeft,m_DistMovedRight,err);
		if (err > BEARING_ERR_CLIP) err = BEARING_ERR_CLIP;
		else if (err < -BEARING_ERR_CLIP) err = -BEARING_ERR_CLIP;
		CMotors::getInstance()->move (dir,err);
	}
}

void CController::runRefreshMap (YapiBotStatus_t status)
{
	//We need to do a complete loop and update the mapper with every new angle / distance detected.
	m_DistMovedLeft += CMotors::getInstance()->getLeftDist();
	m_DistMovedRight += CMotors::getInstance()->getRightDist();

	//Update the map
	CMapper::getInstance()->update(status.heading, status.range);

	//check if we at least turned half a turn before trying to align on the saved bearing
	if (m_DistMovedRight <  2500)
	{
		CMotors::getInstance()->move (0,REFRESH_MAP_SPEED);
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

		fprintf(stdout,"Aligning to %d current = %d\n", m_RequestedBearing, err);


		if (abs(err) < m_BearingErrLim)
		{
			m_BearingGoodCnt++;
		}
		else
		{
			m_BearingGoodCnt = 0;
		}
		if (m_BearingGoodCnt >= m_BearingGoodCntLim)
		{
			m_State = CTRL_STATE_IDLE;
			CMotors::getInstance()->move (0,0);
			fprintf(stdout,"Map refresh complete !\n");
			//Ask the mapper to send the map.
			CMapper::getInstance()->sendMap();
			CEventObserver::getInstance()->notify(CtrlMoveComplete);
		}
		else
		{
			err = err * m_BearingErrGain;
			if (err > REFRESH_MAP_SPEED)
				err = REFRESH_MAP_SPEED;
			else if (err < -REFRESH_MAP_SPEED)
				err = -REFRESH_MAP_SPEED;

			CMotors::getInstance()->move (0,err);
		}
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
	m_ReqLeftMov = -5000;
	m_ReqRightMov = 5000;

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
void CController::refreshMap (void)
{
	m_Lock.get();
	if (m_State != CTRL_STATE_IDLE)
	{
		CEventObserver::getInstance()->notify(CtrlMoveCancel);
		CMotors::getInstance()->move (0,0);
	}

	m_DistMovedLeft = 0;
	m_DistMovedRight = 0;
	m_RequestedBearing = m_Status.heading;


	CMotors::getInstance()->resetDist();

	m_State = CTRL_STATE_REFRESH_MAP;


	m_Lock.release();
}

void CController::CmdPckReceived (char * buffer, unsigned int size) {
	YapiBotCmd_t cmd;
	YapiBotCmd_t cmdType;
	if ((buffer != NULL)&&(size > 1)) //Need at least 2 bytes for the cmd.
	{
		cmd = (YapiBotCmd_t)((buffer[0] << 8) | (buffer[1])); //Cmd is MSB.
		cmdType  = YapiBotCmd_t(cmd & CMD_TYPE_MASK);
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
		case CMD_TYPE_PARAM:
			fprintf(stderr,"received param cmd !!!\n");
			processCmdParam(cmd,&buffer[2],size-2);
			break;
		default:
			fprintf(stderr,"Unknown command received !!!\n");
			break;
		}

	}
}

void CController::processCmdMove (YapiBotCmd_t cmd, char * buffer, unsigned int size)
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

void CController::processCmd (YapiBotCmd_t cmd, char * buffer, unsigned int size)
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
	case CmdRefrehMap:
			refreshMap();
			break;
	default:
		fprintf(stderr,"Unknown command received !!!\n");
		break;
	}
}

void CController::processCmdParam (YapiBotCmd_t cmd, char * buffer, unsigned int size)
{
	//YapiBotParam_t param = (YapiBotParam_t)((buffer[0] << 8) | (buffer[1]));
	YapiBotParam_t param = (YapiBotParam_t)toInt(&buffer[0]);
	printf("ProcessCmdParam with param = 0x%x\n", param);

	switch (param & PARAM_MASK)
	{
	case CONTROL_PARAM:
		if (cmd == CmdSetParam)
		{
			setParameter(param,&buffer[4],size-4);
		}
		else
		{
			getParameter(param);
		}
		break;
	case MOTOR_PARAM:
		if (cmd == CmdSetParam)
		{
			CMotors::getInstance()->setParameter(param,&buffer[4],size-4);
		}
		else
		{
			CMotors::getInstance()->getParameter(param);
		}
		break;
	case CAMERA_PARAM:
	case IMGPROC_PARAM:
		if (cmd == CmdSetParam)
		{
			CImageProcessing::getInstance()->setParameter(param,&buffer[4],size-4);
		}
		else
		{
			CImageProcessing::getInstance()->getParameter(param);
		}
		break;
	default:
		fprintf (stderr, "Unknown parameter !\n");
		break;
	}

}

void CController::setParameter (YapiBotParam_t param, char * buffer, unsigned int size)
{
	unsigned int val;
	if (size < 4)
	{
		fprintf (stderr, "Cannot set controller parameter (not enough arguments)");
	}


	switch (param)
	{
		case CtrlParamColDist:
			m_CollisionDist = toInt (&buffer[0]);
			break;
		case CtrlParamMvErrGain:
			m_MvtErrGain = toInt (&buffer[0]);
			break;
		case CtrlParamBearingErrGain:
			m_BearingErrGain = toInt (&buffer[0]);
			break;
		case CtrlParamBearingErrLim:
			m_BearingErrLim = toInt (&buffer[0]);
			break;
		case CtrlParamBearingGoodCnt:
			m_BearingGoodCntLim = toInt (&buffer[0]);
			break;
		default:
			fprintf (stderr, "Unknown controller parameter !");
			break;
	}
}

void CController::getParameter (YapiBotParam_t param)
{
	YapiBotParamAnswer_t answer;
	answer.id = YAPIBOT_PARAM;
	answer.param = param;
	switch (param)
	{
		case CtrlParamColDist:
			answer.val = m_CollisionDist;
			break;
		case CtrlParamMvErrGain:
			answer.val = m_MvtErrGain;
			break;
		case CtrlParamBearingErrGain:
			answer.val = m_BearingErrGain;
			break;
		case CtrlParamBearingErrLim:
			answer.val = m_BearingErrLim;
			break;
		case CtrlParamBearingGoodCnt:
			answer.val = m_BearingGoodCntLim;
			break;
		default:
			fprintf (stderr, "Unknown controller parameter !");
			return;
	}
	CNetwork::getInstance()->sendCmdPck ((unsigned char *)&answer, sizeof(YapiBotParamAnswer_t));
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

