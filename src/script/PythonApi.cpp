/*
 * PythonApi.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#include "PythonApi.h"
#include "Controller.h"
#include "EventObserver.h"

CPythonApi::CPythonApi() :
m_EventSem(0),
m_LastEvent(CtrlMoveCancel)
{
	CEventObserver::getInstance()->registerOnEvent(EVENT_MASK_CTRL,this,&CPythonApi::EventCallback);
}

CPythonApi::~CPythonApi()
{
	CEventObserver::getInstance()->unRegister (this);
}

int CPythonApi::compassCalibration(void)
{
	CController::getInstance()->compassCalibration();
	fprintf(stdout,"Python Api: Started Compass Calibration\n");
	m_EventSem.wait();
	fprintf(stdout,"Python Api: Compass Calibration complete with event: %d\n", m_LastEvent);
	return (int) m_LastEvent;
}

int CPythonApi::moveStraight (int distance)
{
	CController::getInstance()->moveStraight(distance);
	fprintf(stdout,"Python Api: Started move straight (%d)\n",distance);
	m_EventSem.wait();
	fprintf(stdout,"Python Api: Move complete with event = %d\n", m_LastEvent);
	return (int) m_LastEvent;
}
int CPythonApi::alignBearing (int bearing)
{
	CController::getInstance()->alignBearing(bearing);
	fprintf(stdout,"Python Api: Started align bearing (%d)\n",bearing);
	m_EventSem.wait();
	fprintf(stdout,"Python Api: Align bearing complete with event = %d\n", m_LastEvent);
	return (int) m_LastEvent;
}

int CPythonApi::moveBearing (int bearing, int distance)
{
	CController::getInstance()->moveBearing(bearing,distance);
	fprintf(stdout,"Python Api: Started move bearing (bearing = %d distance = %d)\n",bearing, distance);
	m_EventSem.wait();
	fprintf(stdout,"Python Api: Move bearing complete with event = %d\n", m_LastEvent);
	return (int) m_LastEvent;

}
int CPythonApi::rotate (int rot)
{
	CController::getInstance()->rotate(rot);
	fprintf(stdout,"Python Api: Started rotate (%d)\n",rot);
	m_EventSem.wait();
	fprintf(stdout,"Python Api: Rotate complete with event = %d\n", m_LastEvent);
	return (int) m_LastEvent;
}

void CPythonApi::EventCallback (Event_t evt, int data1, void * data2)
{
	m_LastEvent = evt;
	m_EventSem.post();

}
