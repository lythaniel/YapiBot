/*
 * CallBack.h
 *
 *  Created on: 16 août 2014
 *      Author: lythaniel
 */

#ifndef CALLBACK_H_
#define CALLBACK_H_

/***************************************************************************
 *
 *		Callback with no return value and 0 parameter
 *
 ***************************************************************************/

class Callback0base
{
public:


	virtual ~Callback0base () {}

	virtual void trigger (void) = 0;

	virtual void operator()(void) {trigger();}

};

template <class C>
class Callback0 : public Callback0base
{
public:
	typedef void (C::*Method)(void);

	Callback0 (C * instance, Method method) {m_pInstance = instance; m_Method = method;}

	void trigger (void) {
		(m_pInstance->*m_Method)();
	}
	const C* getInstance (void) {return m_pInstance;}

private:
	C * m_pInstance;
	Method m_Method;
};

#define DECLARE_REG_FUNCTION_CB_0(name, x) \
template <class C> \
void name (C* instance, void (C::*method)(void)) { \
	(x) = new Callback0<C> (instance, method); \
}


/***************************************************************************
 *
 *		Callback with no return value and 1 parameter
 *
 ***************************************************************************/
template <typename param1>
class Callback1base
{
public:


	virtual ~Callback1base () {}

	virtual void trigger (param1 p1) = 0;

	virtual void operator()(param1 p1) {trigger(p1);}



};

template <class C, typename param1>
class Callback1 : public Callback1base<param1>
{
public:
	typedef void (C::*Method)(param1);

	Callback1 (C * instance, Method method) {m_pInstance = instance; m_Method = method;}

	void trigger (param1 p1) {
		(m_pInstance->*m_Method)(p1);
	}

	const C* getInstance (void) {return m_pInstance;}

private:
	C * m_pInstance;
	Method m_Method;
};

#define DECLARE_REG_FUNCTION_CB_1(name, x) \
template <class C,typename param1> \
void name (C* instance, void (C::*method)(param1)) { \
	x = new Callback1<C,param1> (instance, method); \
}

/***************************************************************************
 *
 *		Callback with no return value and 2 parameters
 *
 ***************************************************************************/
template <typename param1, typename param2>
class Callback2base
{
public:


	virtual ~Callback2base () {}

	virtual void trigger (param1 p1, param2 p2) = 0;

	virtual void operator()(param1 p1, param2 p2) {trigger(p1, p2);}

};

template <class C, typename param1, typename param2>
class Callback2 : public Callback2base<param1, param2>
{
public:
	typedef void (C::*Method)(param1,param2);

	Callback2 (C * instance, Method method) {m_pInstance = instance; m_Method = method;}

	void trigger (param1 p1, param2 p2) {
		(m_pInstance->*m_Method)(p1, p2);
	}

	const C* getInstance (void) {return m_pInstance;}

private:
	C * m_pInstance;
	Method m_Method;
};

#define DECLARE_REG_FUNCTION_CB_2(name, cb) \
template <class C,typename param1,typename param2> \
void name (C* instance, void (C::*method)(param1, param2)) { \
	cb = new Callback2<C,param1,param2> (instance, method); \
}


/***************************************************************************
 *
 *		Callback with no return value and 3 parameters
 *
 ***************************************************************************/
template <typename param1, typename param2, typename param3 >
class Callback3base
{
public:


	virtual ~Callback3base () {}

	virtual void trigger (param1 p1, param2 p2, param3 p3) = 0;

	virtual void operator()(param1 p1, param2 p2, param3 p3) {trigger(p1, p2, p3);}

};

template <class C, typename param1, typename param2, typename param3>
class Callback3 : public Callback3base<param1, param2, param3>
{
public:
	typedef void (C::*Method)(param1,param2,param3);

	Callback3 (C * instance, Method method) {m_pInstance = instance; m_Method = method;}

	void trigger (param1 p1, param2 p2, param3 p3) {
		(m_pInstance->*m_Method)(p1, p2, p3);
	}

	virtual const C* getInstance (void) {return m_pInstance;}

private:
	C * m_pInstance;
	Method m_Method;
};

#define DECLARE_REG_FUNCTION_CB_3(name, cb) \
template <class C,typename param1,typename param2, typename param3> \
void name (C* instance, void (C::*method)(param1, param2, param3)) { \
	cb = new Callback2<C,param1,param2, param3> (instance, method); \
}


/***************************************************************************
 *
 *		Callback with return value and 0 parameter
 *
 ***************************************************************************/
template <typename ret>
class Callback0Retbase
{
public:


	virtual ~Callback0Retbase () {}

	virtual ret trigger (void) = 0;

	virtual ret operator()(void) {return trigger();}

};

template <class C, typename ret>
class Callback0Ret : public Callback0Retbase<ret>
{
public:
	typedef ret (C::*Method)(void);

	Callback0Ret (C * instance, Method method) {m_pInstance = instance; m_Method = method;}

	ret trigger (void) {
		return (m_pInstance->*m_Method)();
	}

	const C* getInstance (void) {return m_pInstance;}

private:
	C * m_pInstance;
	Method m_Method;
};

#define DECLARE_REG_FUNCTION_CB_0_RET(name, x) \
template <class C, typename ret> \
void name (C* instance, ret (C::*method)(void)) { \
	(x) = new Callback0<C, ret> (instance, method); \
}


/***************************************************************************
 *
 *		Callback with return value and 1 parameter
 *
 ***************************************************************************/
template <typename ret, typename param1>
class Callback1Retbase
{
public:


	virtual ~Callback1Retbase () {}

	virtual ret trigger (param1 p1) = 0;

	virtual ret operator()(param1 p1) {return trigger(p1);}

};

template <class C, typename ret, typename param1>
class Callback1Ret : public Callback1Retbase<ret,param1>
{
public:
	typedef ret (C::*Method)(param1);

	Callback1Ret (C * instance, Method method) {m_pInstance = instance; m_Method = method;}

	ret trigger (param1 p1) {
		return (m_pInstance->*m_Method)(p1);
	}

	const C* getInstance (void) {return m_pInstance;}

private:
	C * m_pInstance;
	Method m_Method;
};

#define DECLARE_REG_FUNCTION_CB_1_RET(name, x) \
template <class C, typename ret, typename param1> \
void name (C* instance, ret (C::*method)(param1)) { \
	x = new Callback1<C,ret,param1> (instance, method); \
}

/***************************************************************************
 *
 *		Callback with return value and 2 parameters
 *
 ***************************************************************************/
template <typename ret, typename param1, typename param2>
class Callback2Retbase
{
public:


	virtual ~Callback2Retbase () {}

	virtual ret trigger (param1 p1, param2 p2) = 0;

	virtual ret operator()(param1 p1, param2 p2) {return trigger(p1, p2);}

};

template <class C, typename ret, typename param1, typename param2>
class Callback2Ret : public Callback2Retbase<ret, param1, param2>
{
public:
	typedef ret (C::*Method)(param1,param2);

	Callback2Ret (C * instance, Method method) {m_pInstance = instance; m_Method = method;}

	ret trigger (param1 p1, param2 p2) {
		return (m_pInstance->*m_Method)(p1, p2);
	}

	const C* getInstance (void) {return m_pInstance;}

private:
	C * m_pInstance;
	Method m_Method;
};

#define DECLARE_REG_FUNCTION_CB_2_RET(name, cb) \
template <class C, typename ret, typename param1,typename param2> \
void name (C* instance, ret (C::*method)(param1, param2)) { \
	cb = new Callback2<C,ret,param1,param2> (instance, method); \
}


/***************************************************************************
 *
 *		Callback with return value and 3 parameters
 *
 ***************************************************************************/
template <typename ret, typename param1, typename param2, typename param3 >
class Callback3Retbase
{
public:


	virtual ~Callback3Retbase () {}

	virtual ret trigger (param1 p1, param2 p2, param3 p3) = 0;

	virtual ret operator()(param1 p1, param2 p2, param3 p3) {return trigger(p1, p2, p3);}

};

template <class C,typename ret,  typename param1, typename param2, typename param3>
class Callback3Ret : public Callback3Retbase<ret, param1, param2, param3>
{
public:
	typedef ret (C::*Method)(param1,param2,param3);

	Callback3Ret (C * instance, Method method) {m_pInstance = instance; m_Method = method;}

	ret trigger (param1 p1, param2 p2, param3 p3) {
		return (m_pInstance->*m_Method)(p1, p2, p3);
	}

	const C* getInstance (void) {return m_pInstance;}

private:
	C * m_pInstance;
	Method m_Method;
};

#define DECLARE_REG_FUNCTION_CB_3_RET(name, cb) \
template <class C,typename ret, typename param1,typename param2, typename param3> \
void name (C* instance, ret (C::*method)(param1, param2, param3)) { \
	cb = new Callback2<C,ret,param1,param2, param3> (instance, method); \
}

#endif /* CALLBACK_H_ */
