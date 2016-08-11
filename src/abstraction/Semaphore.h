/*
 * semaphore.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <semaphore.h>

#define SEM_TIMEOUT_DONTWAIT 0
#define SEM_TIMEOUT_FOREVER -1

#define SEM_MAXCOUNT_INF -1

class CSemaphore
{
public:

	CSemaphore (int maxcount = SEM_MAXCOUNT_INF);

	~CSemaphore ();

	bool wait(int timeout = SEM_TIMEOUT_FOREVER);
	void post(void);

private:

	sem_t m_Sem;
	int m_MaxCnt;

};



#endif /* SEMAPHORE_H_ */
