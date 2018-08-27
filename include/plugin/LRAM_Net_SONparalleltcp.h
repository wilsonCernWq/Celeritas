#ifndef __LRAM__NET__SON_PARALLELTCP__
#define __LRAM__NET__SON_PARALLELTCP__

/* Server includes the Client */
#ifndef __LRAM__NET__SERVER__
#include "LRAM_Net_Server.h"
#endif

#ifndef __SON__H
#include "SON.h"
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

class LRAM_Net_SONparalleltcpClient : public LRAM_Net_Client
{
	
	public:
		
		/* all the functions */
		LRAM_Net_SONparalleltcpClient();

		LRAM_Net_SONparalleltcpClient (SON_paralleltcpClient* client);

		 ~LRAM_Net_SONparalleltcpClient();
	
		int setProtocolOptions (int option, int value);
		
		int connectToServer (char* ipaddress, int port);

		int connectToServer (char* ipaddress, char* service);
		
		int read (char* data, long long& nbytes);

		int write (char* data, long long& nbytes);
		
		int readv (SON_iovec* iovec_list, long long& nbytes);
		
		int writev (SON_iovec* iovec_list, long long& nbytes);
		
		int close (void);

		string getConnectionInfo (void);

		/* Query the performance of the Connection */
		double getReadBW ();
		
		double getWriteBW ();
		
		double getTotalBW ();

	protected:

		SON_paralleltcpClient* m_client;	
		
};

#endif
