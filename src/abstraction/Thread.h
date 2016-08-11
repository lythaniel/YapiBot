/*
 * Thread.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef THREAD_H_
#define THREAD_H_

#include "CallBack.h"
#include <pthread.h>


class CThread
{
public:
	CThread(void * pdata = NULL);
	~CThread();

	DECLARE_REG_FUNCTION_CB_1(regThreadProcess, m_pProc)

	virtual void start(void);
	virtual void kill (void);

	virtual void run (void);



private:
	Callback1base<void *> * m_pProc;
	pthread_t m_Thread;
	void * m_PrivateData;
	bool m_Running;

};

static void *ThreadProc (void * pdata);

#if 0
class CAlarmThread : public CThread
{
public:
	typedef enum {
		ALARM_THREAD_CONT,
		ALARM_THREAD_ONESHOOT,
	} trigger_t;

	virtual void run (void);
	virtual void start(void);


	CAlarmThread (int32_t * timer, trigger_t trig);


private:


};
#endif
#endif /* THREAD_H_ */
