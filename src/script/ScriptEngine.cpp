/*
 * ScriptEngine.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "ScriptEngine.h"
#include <Python.h>

extern "C" {
extern PyObject* PyInit_YapiBot(void);
}

CScriptEngine::CScriptEngine() :
m_Sem (0),
m_ScriptName(NULL)
{
	PyImport_AppendInittab("YapiBot", PyInit_YapiBot);
	Py_Initialize();
	m_pThread = new CThread ();
	m_pThread->regThreadProcess(this, &CScriptEngine::run);
	m_pThread->start();

}
void CScriptEngine::run (void *)
{
	while (1) {
		m_Sem.wait();
		m_Lock.get();

		if (m_ScriptName != NULL)
		{
			fprintf(stdout,"Running script: %s\n", m_ScriptName);
			FILE *fp = fopen (m_ScriptName, "r+");
			if (fp != NULL)
			{
				PyRun_SimpleFile(fp, m_ScriptName);
			}
			else
			{
				fprintf(stderr,"Could not open script file: %s\n", m_ScriptName);
			}

			fclose(fp);
			m_ScriptName = NULL;
		}
		else
		{
			fprintf(stderr,"NULL Script name!");
		}

		m_Lock.release();
	}
}

CScriptEngine::~CScriptEngine()
{
	m_pThread->kill();
	Py_Finalize();
}

void CScriptEngine::Init(void)
{

}

void CScriptEngine::RunScript (char * scriptname)
{
	m_Lock.get();
	m_ScriptName = scriptname;
	m_Lock.release();
	m_Sem.post();

}




