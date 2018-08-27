#ifndef __LRAM__NET__SERVER__
#define __LRAM__NET__SERVER__

#ifndef __LRAM__NET__CLIENT__
#include "LRAM_Net_Client.h"
#endif

class LRAM_Net_Server 
{

	friend class LRAM_Net_Client;

	public:		

		LRAM_Net_Server() {};

		virtual ~LRAM_Net_Server() {};
		
		virtual int init (int port)  = 0;
	
		virtual int setProtocolOptions (int option, int value) = 0;

		virtual LRAM_Net_Client* waitForNewConnection (void) = 0;
		
		virtual LRAM_Net_Client* checkForNewConnection (int timeoutinsecs) = 0;
		
		virtual int close() = 0;

};

#endif
