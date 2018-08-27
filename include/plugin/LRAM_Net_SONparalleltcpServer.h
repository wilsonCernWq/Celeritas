#ifndef __LRAM__NET__PARALLELTCPSERVER__
#define __LRAM__NET__PARALLELTCPSERVER__

#ifndef __LRAM__NET__SERVER__
#include "LRAM_Net_Server.h"
#endif


#ifndef __SON__PARALLEL__TCP__H
#include "SON_paralleltcp.h"
#endif

using namespace SON_PROTOCOL;

class LRAM_Net_SONparalleltcpServer : public LRAM_Net_Server
{
	public:
	
		LRAM_Net_SONparalleltcpServer();
		
		~LRAM_Net_SONparalleltcpServer();
		
		int init (int port);
	
		int setProtocolOptions (int option, int value);

		LRAM_Net_Client* waitForNewConnection (void);
		
		LRAM_Net_Client* checkForNewConnection (int timeoutinsecs);
		
		int close();

	protected:
	 
		SON_paralleltcpServer* m_server;
};

#endif
