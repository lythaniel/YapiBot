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

#define CMD_TYPE_MASK 0xF000
#define CMD_TYPE_MOVE 0x1000
#define CMD_TYPE_CMD 0x2000

typedef enum {
	CmdMoveStop = CMD_TYPE_MOVE | 0,
	CmdMoveFwd = CMD_TYPE_MOVE | 1,
	CmdMoveRear = CMD_TYPE_MOVE | 2,
	CmdMoveLeft = CMD_TYPE_MOVE | 3,
	CmdMoveRight = CMD_TYPE_MOVE | 4,
	CmdMove = CMD_TYPE_MOVE | 5,
	CmdMoveCam = CMD_TYPE_MOVE | 6,
	CmdCompassCal = CMD_TYPE_CMD | 0,
	CmdMoveStraight = CMD_TYPE_CMD | 1,
	CmdAlignBearing = CMD_TYPE_CMD | 2,
	CmdMoveBearing = CMD_TYPE_CMD | 3,
	CmdRotate = CMD_TYPE_CMD | 4,
} PoBotCmd_t;

#define POBOT_STATUS 0xDEADBEFF
typedef struct {
	int id;
	int heading;
	int speed_left;
	int speed_right;
	int camera_pos;
	int range;
	int meas_left;
	int meas_right;

} PoBotStatus_t;

typedef enum {
	CTRL_EVENT_CANCEL,
	CTRL_EVENT_DONE,
	CTRL_EVENT_OBSTACLE
} ctrlEvent_t;


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

private:

	void processCmdMove (PoBotCmd_t cmd, char * buffer, unsigned int size);
	void processCmd (PoBotCmd_t cmd, char * buffer, unsigned int size);

	void runCompassCalibration (PoBotStatus_t status);
	void runMoveStraight (PoBotStatus_t status);
	void runAlignBearing (PoBotStatus_t status);
	void runMoveBearing (PoBotStatus_t status);
	void runRotate (PoBotStatus_t status);

	CThread * m_Thread;

	ctrlState_t m_State;

	int m_DistMovedLeft;
	int m_DistMovedRight;

	int m_RequestedBearing;
	int m_ReqLeftMov;
	int m_ReqRightMov;

	CMutex m_Lock;

};

#endif /* CONTROLLER_H_ */
