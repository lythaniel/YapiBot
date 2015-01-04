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

#define CMD_TYPE_MASK 0xF000
#define CMD_TYPE_MOVE 0x1000

typedef enum {
	CmdMoveStop = CMD_TYPE_MOVE | 0,
	CmdMoveFwd = CMD_TYPE_MOVE | 1,
	CmdMoveRear = CMD_TYPE_MOVE | 2,
	CmdMoveLeft = CMD_TYPE_MOVE | 3,
	CmdMoveRight = CMD_TYPE_MOVE | 4,
	CmdMove = CMD_TYPE_MOVE | 5,
	CmdMoveCam = CMD_TYPE_MOVE | 6,
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


class CController: public CSingleton <CController> {
public:
	CController();
	~CController();

	void CmdPckReceived (char * buffer, unsigned int size);

	int toInt (char * buff);

	void run (void *);


private:
	void processCmdMove (PoBotCmd_t cmd, char * buffer, unsigned int size);
	CThread * m_Thread;



};

#endif /* CONTROLLER_H_ */
