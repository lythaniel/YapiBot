/*
 * Singleton.h
 *
 *  Created on: 17 août 2014
 *      Author: lythaniel
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
