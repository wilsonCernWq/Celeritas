#ifndef __LRAM__NET__TCPSERVER__
#define __LRAM__NET__TCPSERVER__

#ifndef __LRAM__NET__SERVER__
#include "LRAM_Net_Server.h"
#endif

#ifndef __SON__TCP__H
#include "SON_tcp.h"
#endif

using namespace SON_PROTOCOL;

class LRAM_Net_SONtcpServer : public LRAM_Net_Server
{
	public:
	
		LRAM_Net_SONtcpServer();
		
		~LRAM_Net_SONtcpServer();
		
		int init (int port);
	
		int setProtocolOptions (int option, int value);

		LRAM_Net_Client* waitForNewConnection (void);
		
		LRAM_Net_Client* checkForNewConnection (int timeoutinsecs);
		
		int close();

	protected:
	 
		SON_tcpServer* m_server;
};

#endif
