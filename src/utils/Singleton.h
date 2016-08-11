/*
 * Singleton.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <stdio.h>

template <class C>
class CSingleton
{
public:
	CSingleton (void) {};

	static C * getInstance (void) {
		if (m_pInstance == NULL)
		{
			m_pInstance = new C;
		}
		return m_pInstance;
	}
private:
	static C * m_pInstance;
};

template <class C>
C * CSingleton<C>::m_pInstance = NULL;

#endif /* SINGLETON_H_ */
