/*
 * Semaphore.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#include "Semaphore.h"
#include <time.h>



CSemaphore::CSemaphore (int maxcount) :
m_MaxCnt(maxcount)
{
	sem_init (&m_Sem, 0, 0);
}

CSemaphore::~CSemaphore ()
{
	sem_destroy(&m_Sem);
}

bool CSemaphore::wait(int timeout)
{
	int ret;
	if(timeout == SEM_TIMEOUT_DONTWAIT)
	{
		ret = sem_trywait(&m_Sem);
	}
	else if (timeout == SEM_TIMEOUT_FOREVER)
	{
		ret = sem_wait(&m_Sem);
	}
	else
	{
		timespec t;
		clock_gettime(CLOCK_REALTIME,&t);
		t.tv_sec += timeout/1000;
		t.tv_nsec += (timeout % 1000) * 1000000;
		ret = sem_timedwait(&m_Sem, &t);
	}
	return (ret == 0);
}

void CSemaphore::post(void)
{
	if (m_MaxCnt > 0)
	{
		int val;
		sem_getvalue(&m_Sem, &val);
		if (val < m_MaxCnt)
		{
			sem_post(&m_Sem);
		}
	}
	else
	{
		sem_post(&m_Sem);
	}

}
