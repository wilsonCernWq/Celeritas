#ifndef __LRAM__NET__CLIENT__
#define __LRAM__NET__CLIENT__

#ifndef __SON__IOVEC__H
#include "SON.h"
#endif

using namespace SON_PROTOCOL;

class LRAM_Net_Client 
{
	public:
	
		LRAM_Net_Client() {};
		
		virtual ~LRAM_Net_Client () {};
		
		//virtual int init (int port, char* address = 0)  = 0;
	
		virtual int setProtocolOptions ( int option, int value) = 0;
		
		virtual int connectToServer (char* ipaddress, int port) = 0;

		virtual int connectToServer (char* ipaddress, char* service) = 0;
		
		virtual int read (char* data, long long& nbytes) = 0;

		virtual int write (char* data, long long& nbytes) = 0;
		
		virtual int readv (SON_iovec* iovec_list, long long& nbytes) = 0;
		
		virtual int writev (SON_iovec* iovec_list, long long& nbytes) = 0;
		
		virtual int close (void) = 0;

		virtual string getConnectionInfo (void) = 0;

		/*	
        virtual int getSelfIP (char* ip) = 0;

        virtual int getRemoteIP (char* ip) = 0;

        virtual int getSelfPort (void) = 0;

        virtual int getRemotePort (void) = 0;
		*/


		/* Query the performance of the Connection */
		
		virtual double getReadBW () = 0;
		
		virtual double getWriteBW () = 0;
		
		virtual double getTotalBW () = 0 ;
};

#endif


