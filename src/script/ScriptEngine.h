/*
 * ScriptEngine.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef SCRIPTENGINE_H_
#define SCRIPTENGINE_H_

#include "Singleton.h"
#include "Thread.h"
#include "Semaphore.h"
#include "Mutex.h"


class CScriptEngine : public CSingleton<CScriptEngine>
{
public:
	CScriptEngine();
	~CScriptEngine();

	void Init(void);

	void RunScript (const char * scriptname);

	void run (void *);

private:

	CThread * m_pThread;
	CSemaphore m_Sem;
	CMutex m_Lock;

	const char * m_ScriptName;

};


#endif
