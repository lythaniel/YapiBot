/*
 * Network.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#ifndef NETWORK_H_
#define NETWORK_H_

#include "CallBack.h"
#include "Singleton.h"
#include "YapiBotCmd.h"

#define CMDBUFFER_SIZE 256

class CThread;
class CMutex;
class CSemaphore;

class CNetwork : public CSingleton<CNetwork>
{
public:
	CNetwork();
	~CNetwork();



	DECLARE_REG_FUNCTION_CB_3(regCmdReceived, m_pRxCb)


	void start(void);
	void stop (void);

	void sendCmdPck (YapiBotCmd_t id, unsigned char * payload, unsigned int size);
	
	void sendVideoPacket (unsigned char * buffer, unsigned int size);

	void VideoServerThread (void *);
	void CmdServerThread (void *);

	void RxCmdThread (void *);

	bool isVideoConnected (void) {return m_VideoClientConnected;}

private:
	Callback3base<YapiBotCmd_t, char *, unsigned int> *  m_pRxCb;
	CThread * m_pVideoServerThread;
	CMutex * m_pVideoSockMutex;
	
	int m_VideoServerSock;
	int m_VideoClientSock;
	
	bool m_VideoClientConnected;
	
	CSemaphore * m_pVideoClientDisconnected;
	
	CThread * m_pCmdServerThread;
	CMutex * m_pCmdSockMutex;

	int m_CmdServerSock;
	int m_CmdClientSock;

	bool m_CmdClientConnected;

	CSemaphore * m_pCmdClientDisconnected;

	CThread * m_pRxCmdThread;

	char * m_pCmdRxBuffer;
	char * m_pCmdTxBuffer;
	YapiBotHeader_t * m_pCmdTxHeader;
	char * m_pCmdTxPayload;

	short m_VideoPort;
	short m_CmdPort;


};


#endif /* NETWORK_H_ */
