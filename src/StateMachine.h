/*
 * StateMachine.h
 *
 *  Created on: 13 déc. 2014
 *      Author: lythaniel
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

typedef enum {
	StmEvt_None,
}StmEvent;

class CState {
public:
	virtual CState() = 0;
	virtual ~CState();
	virtual void onEnter(void);
	virtual void run(void) = 0;
	virtual void onExit(void);

};

class CTransition {
public:
	CTransition(CState * from,CState * to, StmEvent event);
	~CTransition();
private:
	CState * m_From;
	CState * m_To;
	StmEvent m_Event;
};

class CStateMachine {
public:
	CStateMachine();
	~CStateMachine();
};

#endif /* STATEMACHINE_H_ */
