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

#define CONTROLLER_PERIOD 500

CController::CController() {

	CNetwork::getInstance()->regCmdReceived(this,&CController::CmdPckReceived);
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

		CNetwork::getInstance()->sendCmdPck ((unsigned char *)&status, sizeof(PoBotStatus_t));


	}

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
		default:
			fprintf(stderr,"Unknown command received !!!\n");
			break;
		}

	}
}

void CController::processCmdMove (PoBotCmd_t cmd, char * buffer, unsigned int size)
{
	int x,y;
	switch (cmd)
	{
	case CmdMoveStop:
		fprintf(stdout,"All machines stop !!!\n");
		CMotors::getInstance()->move (0,0);

		break;
	case CmdMoveFwd:
		fprintf(stdout,"Forward !!!\n");
		CMotors::getInstance()->move (100,0);

		break;
	case CmdMoveRear:
		fprintf(stdout,"Rear !!!\n");
		CMotors::getInstance()->move (-100,0);

		break;
	case CmdMoveLeft:
		fprintf(stdout,"Left !!!\n");
		CMotors::getInstance()->move (0,100);

		break;
	case CmdMoveRight:
		fprintf(stdout,"Right !!!\n");
		CMotors::getInstance()->move (0,-100);

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

int CController::toInt (char * buff)
{
	int ret = (buff [0] << 24) + (buff [1] << 16) + (buff [2] << 8) + buff [3];
	return (ret);
}


