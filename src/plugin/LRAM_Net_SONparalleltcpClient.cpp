#include "LRAM_Net_SONparalleltcpClient.h"

LRAM_Net_SONparalleltcpServer :: LRAM_Net_SONparalleltcpServer()
{
	m_server = new SON_paralleltcpServer();
}

LRAM_Net_SONparalleltcpServer :: ~LRAM_Net_SONparalleltcpServer()
{
	if (0 != m_server)
		delete m_server;
	
	m_server = 0;	
}


int LRAM_Net_SONparalleltcpServer :: init(int port)
{	
	return m_server->init(port);
}


int LRAM_Net_SONparalleltcpServer :: setProtocolOptions( int option, int value)
{
	return 0;
}


LRAM_Net_Client* LRAM_Net_SONparalleltcpServer :: waitForNewConnection()
{
	SON_paralleltcpClient  *ntwkclient = 0;
	ntwkclient = m_server->waitForNewConnection();

	/* An error occured */
	if (0 == ntwkclient)
		return 0;
	
	LRAM_Net_SONparalleltcpClient* client = 
								new LRAM_Net_SONparalleltcpClient(ntwkclient);
	
	return  (LRAM_Net_Client*) client;
}


LRAM_Net_Client* LRAM_Net_SONparalleltcpServer :: checkForNewConnection
												(int timeoutinsecs)
{
	SON_paralleltcpClient  *ntwkclient = 0;
	ntwkclient = m_server->checkForNewConnection(timeoutinsecs);

	/* Error or a Timeout */	
	if ( 0 == ntwkclient)
		return 0;

	LRAM_Net_SONparalleltcpClient* client = 
							new LRAM_Net_SONparalleltcpClient(ntwkclient);
	
	return  (LRAM_Net_Client*) client;
}


int LRAM_Net_SONparalleltcpServer :: close()
{
	return m_server->close();
}



LRAM_Net_SONparalleltcpClient :: LRAM_Net_SONparalleltcpClient()
{
	m_client = new SON_paralleltcpClient();
}

LRAM_Net_SONparalleltcpClient :: LRAM_Net_SONparalleltcpClient
						 (SON_paralleltcpClient* client)
						: m_client(client)
{
}


LRAM_Net_SONparalleltcpClient :: ~LRAM_Net_SONparalleltcpClient()
{
	if ( 0 != m_client) 
		delete m_client;
	
	m_client  = 0;
}


int  LRAM_Net_SONparalleltcpClient :: setProtocolOptions( int option, int value)
{
	return 0;
}


int LRAM_Net_SONparalleltcpClient :: connectToServer(char* ipaddress, int port)
{
	return m_client->connectToServer(ipaddress,port);
}


int LRAM_Net_SONparalleltcpClient :: connectToServer(char* ipaddress, char* service)
{
	return m_client->connectToServer(ipaddress, service);
}


int LRAM_Net_SONparalleltcpClient :: read( char* data, long long& nbytes)
{
	return m_client->read(data, nbytes);
}


int LRAM_Net_SONparalleltcpClient :: write( char* data, long long& nbytes)
{
	return m_client->write(data, nbytes);
}

		
int LRAM_Net_SONparalleltcpClient :: readv (SON_iovec* iovec_list, long long& nbytes)
{
	return m_client->readv (iovec_list, nbytes);
}

	
int LRAM_Net_SONparalleltcpClient :: writev (SON_iovec* iovec_list, long long& nbytes)
{
	return m_client->writev (iovec_list, nbytes);
}

		
int LRAM_Net_SONparalleltcpClient :: close()
{
	return m_client->close();
}

string LRAM_Net_SONparalleltcpClient :: getConnectionInfo (void)
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

double LRAM_Net_SONparalleltcpClient :: getReadBW ()
{
	return m_client->getReadBW ();
}
		
double LRAM_Net_SONparalleltcpClient :: getWriteBW ()
{
	return m_client->getWriteBW ();
}
		
double LRAM_Net_SONparalleltcpClient :: getTotalBW ()
{
	return m_client->getTotalBW ();
}


