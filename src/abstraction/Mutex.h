/*
 * mutex.h
 *
 *  Created on: 11 nov. 2014
 *      Author: lythaniel
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include<pthread.h>

#define MUTEX_TIMEOUT_DONTWAIT 0
#define MUTEX_TIMEOUT_FOREVER -1

class CMutex
{
public:
	CMutex ();
	~CMutex ();

	bool get(int timeout = MUTEX_TIMEOUT_FOREVER);
	void release(void);

private:
	pthread_mutex_t m_Lock;
};



#endif /* MUTEX_H_ */
