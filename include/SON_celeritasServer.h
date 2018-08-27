#ifndef __SON__CELERITAS__SERVER__H
#define __SON__CELERITAS__SERVER__H

#ifndef __SON__CELERITAS__CLIENT__H
#include "SON_celeritasClient.h"
#endif


/*! \namespace SON_PROTOCOL */
namespace SON_PROTOCOL
{

class SON_celeritasClient;

class SON_celeritasServer
{
	public:

		SON_celeritasServer ();

		virtual ~SON_celeritasServer ();

		virtual int init (int port);

		virtual int init (char* port);

		virtual int close();

		virtual SON_celeritasClient* waitForNewConnection ();

		virtual SON_celeritasClient* checkForNewConnection (int timeoutinsecs);

	protected:

		virtual SON_celeritasClient* __establishConnection
												(SON_tcpClient* controlclient);

		virtual SON_celeritasClient* __handleConnectionError
												(SON_celeritasClient*  client);

	protected:

		SON_tcpServer* m_tcpServer;
};

} // End of Namespace

#endif
