

#ifndef NETWORK_H_
#define NETWORK_H_

#include "CallBack.h"
#include "Singleton.h"

#define CMDBUFFER_SIZE 256

class CThread;
class CMutex;
class CSemaphore;

class CNetwork : public CSingleton<CNetwork>
{
public:
	CNetwork();
	~CNetwork();



	DECLARE_REG_FUNCTION_CB_2(regCmdReceived, m_pRxCb)


	void start(void);
	void stop (void);

	void sendCmdPck (unsigned char * buffer, unsigned int size);
	
	void sendVideoPacket (unsigned char * buffer, unsigned int size);

	void VideoServerThread (void *);
	void CmdServerThread (void *);

	void RxCmdThread (void *);

	bool isVideoConnected (void) {return m_VideoClientConnected;}

private:
	Callback2base<char *, unsigned int> *  m_pRxCb;
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

	char m_RxBuffer[CMDBUFFER_SIZE];


};


#endif /* NETWORK_H_ */
