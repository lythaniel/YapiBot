/*
 * Thread.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "Thread.h"


void *ThreadProc (void * pdata)
{
	CThread * ptr = (CThread *)pdata;
	ptr->run();
	return NULL;
}


CThread::CThread(void * pdata) :
m_Thread (0),
m_pProc(NULL),
m_PrivateData (pdata),
m_Running(false)
{

}

CThread::~CThread()
{
	kill();
}

void CThread::start(void)
{
	if (!m_Running)
	{
		if (!pthread_create (&m_Thread, NULL, ThreadProc,this))
		{
			m_Running = true;
		}
	}

}

void CThread::kill(void)
{
	if (m_Running)
	{
		pthread_cancel(m_Thread);
		m_Running = false;
	}

}



void CThread::run ()
{
	if (m_pProc != NULL)
	{
		m_pProc->trigger(m_PrivateData);
	}
}

