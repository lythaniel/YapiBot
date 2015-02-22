/*
 * Event.h
 *
 *  Created on: 15 févr. 2015
 *      Author: lythaniel
 */

#ifndef EVENT_H_
#define EVENT_H_

typedef enum {
	EVENT_MASK_NETWORK 		= (1 << 16),
	EVENT_MASK_CTRL 		= (1 << 17),
	EVENT_MASK_VIDEO 		= (1 << 18),
	EVENT_MASK_ALL 			= 0xFFFF0000
} EventMask_t;



typedef enum  {
//Network events:
	NetVideoServerReady 		= EVENT_MASK_NETWORK + 0,
	NetVideoClientConnected 	= EVENT_MASK_NETWORK + 1,
	NetVideoClientDisconnected 	= EVENT_MASK_NETWORK + 2,
	NetCmdServerReady 			= EVENT_MASK_NETWORK + 3,
	NetCmdClientConnected 		= EVENT_MASK_NETWORK + 4,
	NetCmdClientDisconnected 	= EVENT_MASK_NETWORK + 5,

//Controler events:
	CtrlMoveCancel 				= EVENT_MASK_CTRL + 0,
	CtrlMoveComplete			= EVENT_MASK_CTRL + 1,
	CtrlMoveObstacle			= EVENT_MASK_CTRL + 2


} Event_t;

#endif /* EVENT_H_ */
