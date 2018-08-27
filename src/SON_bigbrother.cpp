/******************************************************************************
 * SYNOPTIC :   A Synergistic Protocol for Optical networks
 * Copyright (C) 2006 Venkatram Vishwanath, Jason Leigh
 * Electronic Visualization Laboratory,
 * University of Illinois at Chicago
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either Version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser Public License along
 * with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Direct questions, comments etc to venkat@evl.uic.edu
 *****************************************************************************/

#include "SON_bigbrother.h"
#include <stdlib.h>

namespace SON_PROTOCOL
{

/* Initialize Static Variables */
 SON_BigBrother* 
				SON_BigBrother :: m_bigbrother = 0;

void serverCleanup(int signum)
{
	SON_PRINT_DEBUG1(" Entering SON_TCP CleanUp Handler :: %d : %s \n ", \
		 __LINE__, __FILE__);

	exit(-1);
	
	SON_PRINT_DEBUG1(" Exiting SON_TCP CleanUp Handler :: %d : %s \n ", \
		__LINE__, __FILE__);
}

void __attribute__ ((destructor)) my_fini(void)
{
	SON_BigBrother* obj = SON_BigBrother :: getObj();
	delete obj;
	obj =0;
}


SON_BigBrother::SON_BigBrother()
{
	SON_PRINT_DEBUG3(" Entering SON_BigBrother Constructor :: %d : %s \n ", \
		 __LINE__, __FILE__);


	struct sigaction old_action;
	memset(&m_sigaction, 0, sizeof(struct sigaction));

	m_sigaction.sa_handler = SON_PROTOCOL::serverCleanup;
	sigemptyset(&m_sigaction.sa_mask);
	m_sigaction.sa_flags =0;

	sigaction(SIGINT, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGINT, &m_sigaction, NULL);

	sigaction(SIGTERM, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGTERM, &m_sigaction, NULL);

	sigaction(SIGHUP, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGHUP, &m_sigaction, NULL);


	this->__initInterfaceDB();

	SON_PRINT_DEBUG3(" EXITING SON_BigBrother Constructor :: %d : %s \n ", \
		 __LINE__, __FILE__);
}

SON_BigBrother::~SON_BigBrother()
{
	m_interfaceDB.clear();
}

SON_BigBrother* SON_BigBrother :: getObj()
{
	if (0 == m_bigbrother)
	{
		m_bigbrother =  new SON_BigBrother();
	}

	return m_bigbrother;
}


int SON_BigBrother :: __initInterfaceDB()
{
	char mystr[1024];
    struct ifaddrs *ifap = 0;
    struct ifaddrs *ifnr =  0;

    struct sockaddr *addr =  0;
    struct sockaddr_in *addr_in =  0;

    int res = getifaddrs( &ifap );
    if ( 0 != res )
    {
        printf( "%s\n", strerror( errno ) );
        exit( -1 );
    }

    ifnr = ifap;

    while ( ( void * ) 0 != ifnr )
    {
        addr = ifnr->ifa_addr;

        if ( AF_INET == addr->sa_family )
        {
            addr_in = ( struct sockaddr_in * ) ifnr->ifa_addr;
            string nicname = string(ifnr->ifa_name);

            memset(mystr, '\0', 1024);
            if (inet_ntop( AF_INET,&addr_in->sin_addr, mystr, 1024) == 0)
				return -1;

            string nicip = string(mystr);

            m_interfaceDB[nicip] = nicname;
        }

        ifnr = ifnr->ifa_next;
    }

	::freeifaddrs(ifap);
	
	return 0;
}




void SON_BigBrother :: printInterfaceList()
{
	map <string, string > :: iterator itr;

	for ( itr = m_interfaceDB.begin(); itr != m_interfaceDB.end(); itr++)
	{
		cout << " IP Addr: " << (*itr).first << " Name: " << (*itr).second << endl;
	} 

}


string SON_BigBrother :: getInterfaceIP(string interface_name)
{
    /* empty string to return in case the interface is not found */
    string retstr;
    retstr.clear();

    map <string, string > :: iterator itr = m_interfaceDB.find(interface_name);
    if (itr == m_interfaceDB.end())
        return retstr;
    else
        return (*itr).second;
}


vector <string> SON_BigBrother :: getInterfaceNameList (void)
{
    vector <string> name_list;
    name_list.clear();

    map <string, string > :: iterator itr;
    for ( itr = m_interfaceDB.begin(); itr != m_interfaceDB.end(); itr++)
    {
        name_list.push_back((*itr).first);
    }

    return name_list;
}


vector <string> SON_BigBrother :: getInterfaceIPList (void)
{
    vector <string> IP_list;
    IP_list.clear();

    map <string, string > :: iterator itr;
    for ( itr = m_interfaceDB.begin(); itr != m_interfaceDB.end(); itr++)
    {
        IP_list.push_back((*itr).second);
    }

    return IP_list;
}



void printInterfaceList (void)
{
	SON_BigBrother* bigb = SON_BigBrother::getObj();
	if (0 == bigb)
		return;
	
	bigb->printInterfaceList();
}


string getInterfaceIP(string interface_name)
{
	/* empty string to return in case the interface is not found */
    string retstr;
    retstr.clear();

	SON_BigBrother* bigb = SON_BigBrother::getObj();
	if( 0 == bigb)
		return retstr;

	return bigb->getInterfaceIP(interface_name);
}



} // endof Namespace

