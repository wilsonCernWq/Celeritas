#ifndef __LRAM__BASE__THREAD__H__
#define __LRAM__BASE__THREAD__H__

#include <pthread.h>


class LRAM_RAIL_THREAD : public LRAM_Base_Thread
{
	public:
	
		LRAM_Base_Thread (void);
	
		virtual ~LRAM_Base_Thread (void);
				
		virtual int start(void);
	
		virtual int exit (void);
		
		virtual int wait (void);
		
		inline pthread_t* getThreadPtr (void) const { return m_thread; }

		unsigned long getThreadId (void) const { return m_threadId; }

		inline int isInitialized (void) const { return m_initted; }

		inline int isRunning(void) const { return m_running; }

		inline int isFinished(void) const { return m_finished; }

		/**
		* mark this thread as detached to reclaim its resources when it terminates
		* @return 0 if successful, else non-zero.
		**/
		virtual int detach (void);

	
	protected:
	
		virtual void run (void) = 0;
		
		static void* __start(void* args);
		
		void __init(void);

	protected:
	
		pthread_t* m_thread;
			/*!< The Current Thread Handle */
			
		pthread_attr_t m_attr;
		
		THREAD_STATE m_state;	
	
		unsigned long m_threadId;
		
		long long m_thdStackSize;
		
		static const int SUCCESS = 0;
		
		static const int FAILURE = -1;
};


#endif
