/*
 * PythonBinding.h
 *
 *  Created on: 29 nov. 2015
 *      Author: lythaniel
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

	int compassCalibration (void);
	int moveStraight (int distance);
	int alignBearing (int bearing);
	int moveBearing (int bearing, int distance);
	int rotate (int rot);

	void EventCallback (Event_t evt, int data1, void * data2);


private:
	CSemaphore m_EventSem;
	CMutex m_Lock;
	Event_t m_LastEvent;



};



#endif /* PYTHONBINDING_H_ */
