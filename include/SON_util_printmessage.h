#ifndef __SON__UTIL__PRINTMESSAGE__H
#define __SON__UTIL__PRINTMESSAGE__H

#include <stdio.h> /* printf */
#include <unistd.h> /* gethostname */
#include <errno.h> /* perror */

/* MACROS FOR PRINTING ALERTS, ERRORS, INFO and DEBUGGING MESSAGES */
	
#ifdef SON_PRINT_ALERT
#undef SON_PRINT_ALERT
#define SON_PRINT_ALERT(format, args ...)  do { char SON_HOSTNAME[1024];  \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("--- ALERT: ");printf(format,args);} while(0)
#else
#define SON_PRINT_ALERT(format, args ...) ((void)0)
#endif

#ifdef SON_PRINT_ERROR
#undef SON_PRINT_ERROR
#define SON_PRINT_ERROR(format, args ...) do {char SON_HOSTNAME[1024];  \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("@@@@@ ERROR @@@@@: ");   \
				printf(format,args);} while(0)
#else
#define SON_PRINT_ERROR(format, args ...) ((void)0)
#endif

#ifdef SON_PRINT_PERROR
#undef SON_PRINT_PERROR
#define SON_PRINT_PERROR(format, args ...) do {char SON_HOSTNAME[1024];  \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("@ PERROR @: "); perror(" perror: " );  \
				printf(format,args);} while(0)
#else
#define SON_PRINT_PERROR(format, args ...) ((void)0)
#endif

#ifdef SON_PRINT_INFO
#undef SON_PRINT_INFO
#define SON_PRINT_INFO(format, args ...) do {char SON_HOSTNAME[1024]; \
					gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
					printf("??? INFO : ");printf(format, args);} while(0)
#else
#define SON_PRINT_INFO(format, args ...) ((void)0)
#endif

#ifdef SON_PRINT_DEBUG1
#undef SON_PRINT_DEBUG1
#define SON_PRINT_DEBUG1(format, args ...) do {char SON_HOSTNAME[1024];  \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("D1: ");printf(format, args);} while (0)
#else
#define SON_PRINT_DEBUG1(format, args ...) ((void)0)
#endif

#ifdef SON_PRINT_DEBUG2
#undef SON_PRINT_DEBUG2
#ifdef SON_PRINT_DEBUG1	
#undef SON_PRINT_DEBUG1
#define SON_PRINT_DEBUG1(format, args ...) do {char SON_HOSTNAME[1024];  \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("D1: ");printf(format, args);} while (0)
#endif	
#define SON_PRINT_DEBUG2(format, args ...) do {char SON_HOSTNAME[1024]; \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("--D2: ");printf(format, args); } while(0)
#else
#define SON_PRINT_DEBUG2(format, args ...) ((void)0)
#endif

#ifdef SON_PRINT_DEBUG3
#undef SON_PRINT_DEBUG3
#ifdef SON_PRINT_DEBUG1
#undef SON_PRINT_DEBUG1
#define SON_PRINT_DEBUG1(format, args ...) do {char SON_HOSTNAME[1024]; \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("D1: ");printf(format, args);} while (0)
#endif
#ifdef SON_PRINT_DEBUG2
#undef SON_PRINT_DEBUG2
#define SON_PRINT_DEBUG2(format, args ...) do { char SON_HOSTNAME[1024]; \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("--D2: ");printf(format, args); } while(0)
#endif
#define SON_PRINT_DEBUG3(format, args ...) do { char SON_HOSTNAME[1024]; \
				gethostname(SON_HOSTNAME, 1024); printf("%s -",SON_HOSTNAME ); \
				printf("----D3: ");printf(format, args); }  while(0)
#else
#define SON_PRINT_DEBUG3(format, args ...) ((void)0)
#endif


#endif
