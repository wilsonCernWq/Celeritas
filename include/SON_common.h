/******************************************************************************
 * Copyright (C) 2006-2008 Venkatram Vishwanath, Jason Leigh
 *  Electronic Visualization Laboratory,
 * 	University of Illinois at Chicago
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

#ifndef __SON__COMMON__H
#define __SON__COMMON__H

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <map>

#include "SON_util_printmessage.h"

#include "SON_ErrorCode.h"

using namespace std;

#if __GNUC__ >= 3
# define inline		inline __attribute__ ((always_inline))
# define __packed	__attribute__ ((packed))
# define likely(x)	__builtin_expect (!!(x), 1)
# define unlikely(x)	__builtin_expect (!!(x), 0)
#else
# define inline		/* no inline */
# define __packed	/* no packed */
# define likely(x)	(x)
# define unlikely(x)	(x)
#endif


namespace SON_PROTOCOL
{

const int SON_LISTEN_QUEUE_SIZE =  256;

const int SON_STRING_SIZE_SMALL = 256;

const int SON_STRING_SIZE_LARGE = 1024;
	
const int SON_CONN_RETRIES  = 250;
	
const int SON_SENDBUFFER_SIZE_DEFAULT  = (256 * 1024) ;

const int SON_RECVBUFFER_SIZE_DEFAULT  = (256 * 1024) ;

const int SON_CONN_RETRY_DELAY_USECS_SHORT = 50000;

const int SON_CONN_RETRY_DELAY_USECS_LONG = 800000; 

const int SON_DATASERVER_PORTRANGE = 100;

const int SON_PARALLELTCP_MINSIZE = (64 * 1024);

const int SON_PARALLELTCP_NUMSOCKETS = 8;

const int SON_PARALLELTCP_MINBUFFERS = 1;


enum SON_IO_MODE
{
	SON_IO_BLOCKING,
	
	SON_IO_NONBLOCKING,
	
	SON_IO_POLL,

	SON_IO_EPOLL,

	SON_IO_AIO
};

}



#endif


