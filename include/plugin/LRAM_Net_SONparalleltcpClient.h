#ifndef __LRAM__NET__SON_PARALLELTCPCLIENT__
#define __LRAM__NET__SON_PARALLELTCPCLIENT__

#ifndef __LRAM__NET__CLIENT__
#include "LRAM_Net_Client.h"
#endif

#ifndef __SON__PARALLEL__TCP__H
#include "SON_paralleltcp.h"
#endif

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
