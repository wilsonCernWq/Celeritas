#ifndef __SON__TIMER__H
#define __SON__TIMER__H

#ifndef __SON__SYS__TIME__H
#define __SON__SYS__TIME__H
#include <sys/time.h>
#endif

#ifndef __SON__UNISTD__H
#define __SON__UNISTD__H
#include <unistd.h>
#endif

namespace SON_PROTOCOL
{

inline
unsigned long long rdtsc(void)
{
  //unsigned long long hi, lo;
  unsigned  hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32);

}

static inline
unsigned long long  __computeFrequency (void)
{
    unsigned long long start_cycle, stop_cycle, tdiff;
    struct timeval start_tv, stop_tv;
    unsigned long long  cyclediff, freq;

    static unsigned long long _cpufreq = 0;

    gettimeofday( &start_tv, 0 );
    start_cycle = rdtsc();

	sleep(1);

    gettimeofday( &stop_tv, 0 );
    stop_cycle = rdtsc();
                                  
    /* usec */
    tdiff =
      ( stop_tv.tv_sec  * 1000 * 1000 + stop_tv.tv_usec ) -
      ( start_tv.tv_sec * 1000 * 1000 + start_tv.tv_usec );

    cyclediff = stop_cycle - start_cycle;

	/* round out freq. */
	freq = (cyclediff/tdiff) * 1000;
	freq *= 1000; // Hz

	if( _cpufreq < freq )
		_cpufreq = freq;

  return _cpufreq;
}

static const unsigned long long m_cpuFrequency = __computeFrequency();

const uint64_t m_msecsTicks = m_cpuFrequency / 1000;

const uint64_t  m_usecsTicks = m_cpuFrequency / 1000000;

const uint64_t   m_nsecsTicks = m_cpuFrequency / 1000000000;

inline
void  __usecDelay_RDTSC ( int& usec)
{
   uint64_t start_ticks;
   start_ticks = rdtsc();

    uint64_t endticks = (m_usecsTicks * usec) +  start_ticks;

    while( rdtsc() < endticks)
    {

    }
}

inline
void  __msecDelay_RDTSC ( int& msec)
{
   uint64_t start_ticks;
   start_ticks = rdtsc();

    uint64_t endticks = (m_msecsTicks * msec) +  start_ticks;

    while( rdtsc() < endticks)
    {

    }
}


}// End of namespace

#endif
