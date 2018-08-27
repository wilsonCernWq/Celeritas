#include "LRAM_Net_SONtcp.h"

LRAM_Net_SONtcpServer :: LRAM_Net_SONtcpServer()
{
	m_server = new SON_tcpServer();
}

LRAM_Net_SONtcpServer :: ~LRAM_Net_SONtcpServer()
{
	if (0 != m_server)
		delete m_server;
	
	m_server = 0;	
}


int LRAM_Net_SONtcpServer :: init(int port)
{	
	return m_server->init(port);
}


int LRAM_Net_SONtcpServer :: setProtocolOptions( int option, int value)
{
	return 0;
}


LRAM_Net_Client* LRAM_Net_SONtcpServer::waitForNewConnection()
{
	SON_tcpClient  *ntwkclient = 0;
	ntwkclient = m_server->waitForNewConnection();
	
	LRAM_Net_SONtcpClient* client = new LRAM_Net_SONtcpClient(ntwkclient);
	
	return  (LRAM_Net_Client*) client;
}


LRAM_Net_Client* LRAM_Net_SONtcpServer :: checkForNewConnection
											(int timeoutinsecs)
{
	SON_tcpClient  *ntwkclient = 0;
	ntwkclient = m_server->checkForNewConnection(timeoutinsecs);
	
	if ( 0 != ntwkclient)
	{
		LRAM_Net_SONtcpClient* client = new LRAM_Net_SONtcpClient(ntwkclient);
	
		return  (LRAM_Net_Client*) client;
	}
	 
	return 0;
}


int LRAM_Net_SONtcpServer :: close()
{
	return m_server->close();
}




LRAM_Net_SONtcpClient :: LRAM_Net_SONtcpClient()
{
	m_client = new SON_tcpClient();
}

LRAM_Net_SONtcpClient :: LRAM_Net_SONtcpClient (SON_tcpClient* client)
						: m_client(client)
{
}


LRAM_Net_SONtcpClient :: ~LRAM_Net_SONtcpClient()
{
	if ( 0 != m_client) 
		delete m_client;
	
	m_client  = 0;
}


int  LRAM_Net_SONtcpClient :: setProtocolOptions( int option, int value)
{
	return 0;
}


int LRAM_Net_SONtcpClient :: connectToServer(char* ipaddress, int port)
{
	return m_client->connectToServer(ipaddress,port);
}


int LRAM_Net_SONtcpClient :: connectToServer(char* ipaddress, char* service)
{
	return m_client->connectToServer(ipaddress, service);
}


int LRAM_Net_SONtcpClient :: read( char* data, long long& nbytes)
{
	return m_client->read(data, nbytes);
}


int LRAM_Net_SONtcpClient :: write( char* data, long long& nbytes)
{
	return m_client->write(data, nbytes);
}

		
int LRAM_Net_SONtcpClient :: readv (SON_iovec* iovec_list, long long& nbytes)
{
	return m_client->readv (iovec_list, nbytes);
}

	
int LRAM_Net_SONtcpClient :: writev (SON_iovec* iovec_list, long long& nbytes)
{
	return m_client->writev (iovec_list, nbytes);
}

		
int LRAM_Net_SONtcpClient :: close()
{
	return m_client->close();
}

string LRAM_Net_SONtcpClient :: getConnectionInfo (void)
{
	string info;
	char buf[4096];
	int status, port;
	
	/* Get Local IP */
	info += string(" LOCAL IP :: ");
	memset(buf, '\0', 4096);	
	status = m_client->getSelfIP(buf);
	if ( 0 == status)
		info += string(buf);
	else
	{
		info.clear();
		return info;
	}

	/* Get Local PORT */
	info += string(" LOCAL PORT :: ");
	memset(buf, '\0', 4096);
	
	port = m_client->getSelfPort();
	if ( port > 0 )
	{
		sprintf(buf, "%d", port);
		info += string(buf);
	}
	else
	{
		info.clear();
		return info;
	}

	/* Get Remote IP */
	info += string(" -- Remote IP :: ");
	memset(buf, '\0', 4096);	
	status = m_client->getRemoteIP(buf);
	if ( 0 == status)
		info += string(buf);
	else
	{
		info.clear();
		return info;
	}

	
	/* Get Local PORT */
	info += string(" REMOTE PORT :: ");
	memset(buf, '\0', 4096);
	
	port = m_client->getRemotePort();
	if ( port > 0 )
	{
		sprintf(buf, "%d", port);
		info += string(buf);
	}
	else
	{
		info.clear();
		return info;
	}

	return info;

}

double LRAM_Net_SONtcpClient :: getReadBW ()
{
	return m_client->getReadBW ();
}
		
double LRAM_Net_SONtcpClient :: getWriteBW ()
{
	return m_client->getWriteBW ();
}
		
double LRAM_Net_SONtcpClient :: getTotalBW ()
{
	return m_client->getTotalBW ();
}


