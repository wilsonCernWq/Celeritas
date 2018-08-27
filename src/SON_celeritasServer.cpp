#include "SON_celeritasServer.h"


namespace SON_PROTOCOL
{

SON_celeritasServer :: SON_celeritasServer () : m_tcpServer(0)
{

}


SON_celeritasServer :: ~SON_celeritasServer()
{
	if (m_tcpServer )
	{
		delete m_tcpServer;
		m_tcpServer = 0;
	}
}


int SON_celeritasServer :: init (char* port)
{
	m_tcpServer =  new SON_tcpServer();
	if ( 0 == m_tcpServer)
		return SON_PROTOCOL::FAILURE;

	if (SON_PROTOCOL::FAILURE == m_tcpServer->init (port))
		return SON_PROTOCOL::FAILURE;

	return SON_PROTOCOL::SUCCESS;
}


int SON_celeritasServer :: init (int port)
{
	m_tcpServer =  new SON_tcpServer();
	if ( 0 == m_tcpServer)
		return SON_PROTOCOL::FAILURE;

	if (SON_PROTOCOL::FAILURE == m_tcpServer->init (port))
		return SON_PROTOCOL::FAILURE;

	return SON_PROTOCOL::SUCCESS;
}


int SON_celeritasServer :: close()
{
	m_tcpServer->close();

	return SON_PROTOCOL::SUCCESS;
}


/*
 *	1. Wait for a connection over TCP
 *	2. Create a UDP Port (Ephermeral)
 *	3. Read the  Remote UDP Port
 *	4. Send the Local UDP Port
 *	5. Connect the UDP socket
*/

SON_celeritasClient* SON_celeritasServer :: waitForNewConnection()
{
	SON_tcpClient* controlclient = m_tcpServer->waitForNewConnection();
	if (0 == controlclient)
		return 0;

	return this->__establishConnection (controlclient);
}


SON_celeritasClient* SON_celeritasServer :: checkForNewConnection
													(int timeoutinsecs)
{
	SON_tcpClient* controlclient = m_tcpServer->checkForNewConnection
														(timeoutinsecs);
	if (0 == controlclient)
		return 0;

	return this->__establishConnection (controlclient);
}


SON_celeritasClient* SON_celeritasServer :: __establishConnection
												(SON_tcpClient* controlclient)
{
	SON_celeritasClient* client =  new SON_celeritasClient();
	if ( 0 == client)
		return 0;

	client->__setTCPClient (controlclient);

	// Create the UDP Socket
	if (SON_PROTOCOL::SUCCESS != client->__createUDPSocket())
		return this->__handleConnectionError(client);

	// Assign the Local UDP Port
	if (SON_PROTOCOL::SUCCESS != client->__initLocalUDPPort())
			return this->__handleConnectionError(client);

	// Read the Remote UDP Port
	if (SON_PROTOCOL::SUCCESS != client->__readRemoteUDPPort())
		return this->__handleConnectionError(client);

	// Send the Local UDP Port
	if (SON_PROTOCOL::SUCCESS != client->__sendLocalUDPPort())
		return this->__handleConnectionError(client);

	// Connect the UDP Socket
	if (SON_PROTOCOL::SUCCESS != client->__connectUDPSocket())
		return this->__handleConnectionError(client);

	// Set the buffer sizes
	int bufsize =  4 * 1024 * 1024;
	client->setSendBufSize(bufsize);
	client->setRecvBufSize(bufsize);

	// Initialize the Start time
	client->initPerf();

	return client;
}


SON_celeritasClient* SON_celeritasServer :: __handleConnectionError
											(SON_celeritasClient*  client)
{
	delete client;
	client = 0;
	return 0;
}

} // end of namespace
