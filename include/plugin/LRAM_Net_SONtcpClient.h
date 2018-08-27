#ifndef __LRAM__NET__SON_TCP__
#define __LRAM__NET__SON_TCP__

#ifndef __LRAM__NET__CLIENT__
#include "LRAM_Net_Client.h"
#endif

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



class LRAM_Net_SONtcpClient : public LRAM_Net_Client
{
	
	public:
		
		/* all the functions */
		LRAM_Net_SONtcpClient();

		LRAM_Net_SONtcpClient (SON_tcpClient* client);

		 ~LRAM_Net_SONtcpClient();
	
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
		
		double getReadBW (void);
		
		double getWriteBW (void);
		
		double getTotalBW (void);

	protected:

		SON_tcpClient* m_client;	
		
};

#endif
