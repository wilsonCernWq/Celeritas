/******************************************************************************
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
 * Direct questions, comments etc about SYNOPTIC to venkat@evl.uic.edu
 *****************************************************************************/
 
#include "SON_iovec.h"
#include <stdlib.h>

namespace SON_PROTOCOL
{

SON_iovec :: SON_iovec() :	m_numAllocated(IOV_MAX), 
							m_numUsed(0), 
							m_totLen(0)
{		
	/* We allocate IOV_MAX buffers by Default */
	m_iovecList = (struct iovec *)
					malloc(sizeof(struct iovec) * m_numAllocated );

	/* Initialize the List */
	for ( int i =0; i < m_numAllocated; i++)
	{
		m_iovecList[i].iov_base = 0;
		m_iovecList[i].iov_len = 0;
	}
}


SON_iovec :: SON_iovec (int size) : m_numAllocated(IOV_MAX),
									m_numUsed(0),
									m_totLen(0)
{	
	/* We allocate in multiples of IOV_MAX */
	if (size ==0)
		m_numAllocated = IOV_MAX;
	else if ((size % IOV_MAX) == 0)
		m_numAllocated = (size / IOV_MAX)* IOV_MAX;
	else	 
		m_numAllocated =  ((size / IOV_MAX) +1)* IOV_MAX;
	
	/* Allocate the iovec List */
	m_iovecList = (struct iovec *)
				malloc(sizeof(struct iovec) * m_numAllocated );

	/* Initialize the List */
	for ( int i =0; i < m_numAllocated; i++)
	{
		m_iovecList[i].iov_base = 0;
		m_iovecList[i].iov_len = 0;
	}
}

/* destructor */
SON_iovec::~SON_iovec()
{
	if  (m_iovecList)
	{
		free (m_iovecList);
		m_iovecList =0;
	}
}

/* add pointer and its length to the array */
int SON_iovec::add(void* buff, long long int length)
{
	/* If we have an overflow, we allocate an additional IOV_MAX buffers */
	if (unlikely( m_numUsed == m_numAllocated ))
	{
		m_numAllocated += IOV_MAX;
		m_iovecList = (struct iovec*)realloc((void *)m_iovecList,  \
							m_numAllocated * sizeof(struct iovec));

		if (0 == m_iovecList)
			return -1;	
	}
	
	/* Add the buffer to the IOVEC List */
	m_iovecList[m_numUsed].iov_base = buff;
	m_iovecList[m_numUsed].iov_len = length;
	
	/* Increment the number of used buffers and the Total length */
	m_numUsed++;
	m_totLen += length;

	return 0;
}


int SON_iovec::clear(void)
{
	m_numUsed =0;
	m_totLen =0;
	
	/* Clear all the Buffers and initialize them */
	for ( int i =0; i < m_numAllocated; i++)
	{
		m_iovecList[i].iov_base = 0;
		m_iovecList[i].iov_len = 0;
	}
	
	return 0;
}


struct iovec* SON_iovec::getIovecPtr(int index)
{
	return &(m_iovecList[index]);
}


//returns the number of buffer elements i.e the iovec count
void* SON_iovec::getIovecBufferPtr(int index)
{
	return m_iovecList[index].iov_base;
}

//returns the number of buffer elements i.e the iovec count

void* SON_iovec::getIovecBufferPtr(int index, long long int offset)
{
	unsigned char* ptr = (unsigned char* )m_iovecList[index].iov_base;
	return (void* )&(ptr[offset]);
}


long long int SON_iovec::getIovecBufferLen(int index)
{
	return m_iovecList[index].iov_len;
}


int SON_iovec::getUsedBuffers(void)
{
	return m_numUsed;
}


int SON_iovec :: getAllocBuffers(void)
{
	return m_numAllocated;
}


long long int SON_iovec :: getTotalLen(void)
{
	return m_totLen;
}

} // End of Namespace

