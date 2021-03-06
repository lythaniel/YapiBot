/*
 * PythonApi.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef PYTHONBINDING_H_
#define PYTHONBINDING_H_

#include "Event.h"
#include "Semaphore.h"
#include "Mutex.h"

class CPythonApi
{
public:
	CPythonApi ();
	~CPythonApi ();

	int32_t compassCalibration (void);
	int32_t moveStraight (int32_t distance);
	int32_t alignBearing (int32_t bearing);
	int32_t moveBearing (int32_t bearing, int32_t distance);
	int32_t rotate (int32_t rot);

	void EventCallback (Event_t evt, int32_t data1, void * data2);


private:
	CSemaphore m_EventSem;
	CMutex m_Lock;
	Event_t m_LastEvent;



};



#endif /* PYTHONBINDING_H_ */
