/*
 * YapiBotCmd.h
 *
 *  Created on: 6 déc. 2015
 *      Author: lythaniel
 */

#ifndef YAPIBOTCMD_H_
#define YAPIBOTCMD_H_


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
	CmdSetParam = CMD_TYPE_PARAM | 0,
	CmdGetParam = CMD_TYPE_PARAM | 1,
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


typedef enum
{
	InfoStatus = 0xDEADBEEF,
	InfoParam = 0x00000
} YapiBotInfo_t;

#define YAPIBOT_STATUS 0xDEADBEFF
#define YAPIBOT_PARAM 0x0000000

typedef struct {
	int id;
	int heading;
	int speed_left;
	int speed_right;
	int camera_pos;
	int range;
	int meas_left;
	int meas_right;
} YapiBotStatus_t;

typedef struct {
	int id;
	YapiBotParam_t param;
	int val;
} YapiBotParamAnswer_t;





#endif /* YAPIBOTCMD_H_ */
