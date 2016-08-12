/*
 * YapiBotCmd.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef YAPIBOTCMD_H_
#define YAPIBOTCMD_H_

#include "YapiBotTypes.h"

#define YAPIBOT_MAGIC_NUMBER 0x1A2B3C4D
#define YAPIBOT_MAX_PL_SIZE 1024

#define CMD_TYPE_MASK 0xF000
#define CMD_TYPE_MOVE 0x1000
#define CMD_TYPE_CMD 0x2000
#define CMD_TYPE_SCRIPT 0x3000
#define CMD_TYPE_PARAM 0x4000

#define PARAM_MASK 0xF000
#define CONTROL_PARAM 0x0000
#define MOTOR_PARAM 0x1000
#define CAMERA_PARAM 0x2000
#define IMGPROC_PARAM 0x3000

#define CMD_TYPE_STATUS 0x80000000
#define CMD_TYPE_PARAM_ANSWER 0x80010000
#define CMD_TYPE_MAP 0x80020000


typedef enum {
	//From controller to Yapibot
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
	CmdRefrehMap = CMD_TYPE_CMD | 5,
	CmdSetParam = CMD_TYPE_PARAM | 0,
	CmdGetParam = CMD_TYPE_PARAM | 1,

	//From Yapibot to Controler.
	CmdInfoStatus = CMD_TYPE_STATUS,
	CmdInfoParam = CMD_TYPE_PARAM_ANSWER,
	CmdInfoMap = CMD_TYPE_MAP,

} YapiBotCmd_t;



typedef enum {


	CtrlParamColDist = CONTROL_PARAM | 0x00,  			//define the collision distance. Integer.
	CtrlParamMvErrGain = CONTROL_PARAM | 0x01,			//define the error gain while moving. Integer.
	CtrlParamBearingErrGain = CONTROL_PARAM | 0x02, 	//define the error gain while aligning on a bearing. Integer.
	CtrlParamBearingErrLim = CONTROL_PARAM | 0x03, 		//define the accuracy of the bearing. Integer.
	CtrlParamBearingGoodCnt = CONTROL_PARAM | 0x04, 	//define the number of time bearing should below the error lim before the alignment is complete . Integer.
	MtrParamSpeedConv = MOTOR_PARAM | 0x00,				//define the conversion for speed consign, float
	MtrParamSpeedErrGain = MOTOR_PARAM | 0x01,			//define the speed error gain, float
	MtrParamAccErrGain = MOTOR_PARAM | 0x02,			//define the acceleration error gain, float
	CamParamSaturation = CAMERA_PARAM | 0x00,           //define the camera saturation, Integer
	CamParamContrast = CAMERA_PARAM | 0x01,             //define the camera contrast, Integer
	CamParamBrightness = CAMERA_PARAM | 0x02,           //define the camera brightness, Integer
	CamParamsharpness = CAMERA_PARAM | 0x03,            //define the camera sharpness, Integer
	CamParamIso = CAMERA_PARAM | 0x04,             		//define the camera ISO, Integer




} YapiBotParam_t;




typedef struct {
	int32_t heading;
	int32_t speed_left;
	int32_t speed_right;
	int32_t camera_pos;
	int32_t range;
	int32_t meas_left;
	int32_t meas_right;
	int32_t accel_x;
	int32_t accel_y;
	int32_t rot_z;
} YapiBotStatus_t;

typedef struct {
	YapiBotParam_t param;
	int32_t val;
} YapiBotParamAnswer_t;

typedef struct {
	int32_t magicNumber;
	YapiBotCmd_t id;
	int32_t payloadSize;
} YapiBotHeader_t;



#endif /* YAPIBOTCMD_H_ */
