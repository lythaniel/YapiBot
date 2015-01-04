/*
 * mutex.cpp
 *
 *  Created on: 11 nov. 2014
 *      Author: lythaniel
 */


#include "Mutex.h"
#include <stdio.h>


CMutex::CMutex ()
{
	pthread_mutex_init(&m_Lock, NULL);
}

CMutex::~CMutex ()
{
	pthread_mutex_destroy(&m_Lock);
}

bool CMutex::get(int timeout)
{
	int ret;
	if (timeout == MUTEX_TIMEOUT_FOREVER)
	{
		ret = pthread_mutex_lock(&m_Lock);
	}
	else if (timeout == MUTEX_TIMEOUT_DONTWAIT)
	{
		ret = pthread_mutex_trylock(&m_Lock);
	}
	else
	{
		fprintf (stdout, "Error: Mutex timeout not implemented !!!");
	}
	return (ret == 0);

}

void CMutex::release(void)
{
	pthread_mutex_unlock(&m_Lock);
}



