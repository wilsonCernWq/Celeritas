/*
 *  SON_perf.cpp
 *
 *
 *  Created by Venkatram Vishwanath on 12/25/07.
 *  Copyright 2007 EVL. All rights reserved.
 *
 */

#include "SON_perf.h"

namespace SON_PROTOCOL
{

SON_perf :: SON_perf () : m_totRead(0), m_totWrite(0)
{
	m_protectReadVal =  new pthread_rwlock_t;
	pthread_rwlock_init(m_protectReadVal, NULL);

	m_protectWriteVal =  new pthread_rwlock_t;
	pthread_rwlock_init(m_protectWriteVal, NULL);

	/* This time is used for both Input and Output BW calculation */
	::gettimeofday(&m_startTime, 0);
}


SON_perf :: ~SON_perf ()
{
	pthread_rwlock_destroy(m_protectWriteVal);
	delete m_protectWriteVal;
	m_protectWriteVal = 0;

	pthread_rwlock_destroy(m_protectReadVal);
	delete m_protectReadVal;
	m_protectReadVal = 0;
}


double SON_perf :: getReadBW_Mbps (void)
{
	struct timeval end;
	::gettimeofday(&end, 0);

	double bw = 0;
	pthread_rwlock_rdlock(m_protectReadVal);
	bw = m_totRead;
	pthread_rwlock_unlock(m_protectReadVal);

	return ((bw * 8) / (double)((end.tv_sec - m_startTime.tv_sec)
	 	+ ((double)(end.tv_usec - m_startTime.tv_usec)/1000000)))/1000000;

}


double SON_perf :: getReadBW(void)
{
	struct timeval end;
	::gettimeofday(&end, 0);

	double bw = 0;
	pthread_rwlock_rdlock(m_protectReadVal);
	bw = m_totRead;
	pthread_rwlock_unlock(m_protectReadVal);

	return (bw * 8) / (double)((end.tv_sec - m_startTime.tv_sec)
	 	+ ((double)(end.tv_usec - m_startTime.tv_usec)/1000000));

}


double SON_perf :: getWriteBW_Mbps (void)
{
	struct timeval end;
	::gettimeofday(&end, 0);

	double bw = 0;
	pthread_rwlock_rdlock(m_protectWriteVal);
	bw = m_totWrite;
	pthread_rwlock_unlock(m_protectWriteVal);

	return ((bw * 8) / (double)((end.tv_sec - m_startTime.tv_sec)
	 	+ ((double)(end.tv_usec - m_startTime.tv_usec)/1000000)))/1000000;

}


double SON_perf :: getWriteBW(void)
{
	struct timeval end;
	::gettimeofday(&end, 0);

	double bw = 0;
	pthread_rwlock_rdlock(m_protectWriteVal);
	bw = m_totWrite;
	pthread_rwlock_unlock(m_protectWriteVal);

	return (bw * 8) / (double)((end.tv_sec - m_startTime.tv_sec)
	 	+ ((double)(end.tv_usec - m_startTime.tv_usec)/1000000));

}

double SON_perf :: getTotalBW_Mbps (void)
{
	return this->getReadBW_Mbps() + this->getWriteBW_Mbps();
}


double SON_perf :: getTotalBW(void)
{
	return this->getReadBW() + this->getWriteBW();
}


double SON_perf :: getDataRead(void)
{
	double retval;

	pthread_rwlock_rdlock(m_protectReadVal);
	retval = m_totRead;
	pthread_rwlock_unlock(m_protectReadVal);

	return retval;
}

double SON_perf :: getDataWritten(void)
{
	double retval;

	pthread_rwlock_rdlock(m_protectWriteVal);
	retval = m_totWrite;
	pthread_rwlock_unlock(m_protectWriteVal);

	return retval;
}



void SON_perf :: updateDataRead (double val)
{
	pthread_rwlock_wrlock(m_protectReadVal);
	m_totRead += val;
	pthread_rwlock_unlock(m_protectReadVal);
	return;
}


void SON_perf :: updateDataWritten (double val)
{
	pthread_rwlock_wrlock(m_protectWriteVal);
	m_totWrite += val;
	pthread_rwlock_unlock(m_protectWriteVal);
	return;
}

struct timeval SON_perf :: getConnStartTime (void)
{
	return m_startTime;
}


void SON_perf :: initConnStartTime(void)
{
	::gettimeofday(&m_startTime, 0);
}


}


