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

	void RunScript (char * scriptname);

	void run (void *);

private:

	CThread * m_pThread;
	CSemaphore m_Sem;
	CMutex m_Lock;

	char * m_ScriptName;

};


#endif
