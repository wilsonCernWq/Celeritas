#ifndef __SON__TIMER__H
#include "SON_timer.h"
#endif

#include <iostream>

using namespace std;
using namespace SON_PROTOCOL;

int udp_client(const char *hostname, const char *service)
{
    struct addrinfo hints, *res, *ressave;
    int error, sockfd, yes;

    memset(&hints, 0, sizeof(struct addrinfo));

    /*
       AI_PASSIVE flag: the resulting address is used to bind
       to a socket for accepting incoming connections.
       So, when the hostname==NULL, getaddrinfo function will
       return one entry per allowed protocol family containing
       the unspecified address for that family.
    */

    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    error = getaddrinfo(hostname, service, &hints, &res);

    if (error != 0) {
        fprintf(stderr,
                "getaddrinfo error:: [%s]\n",
                gai_strerror(error));
        return -1;
    }

    ressave=res;

    /*
       Try open socket with each address getaddrinfo returned,
       until getting a valid listening socket.
    */
    sockfd=-1;
    do {
        sockfd = socket(res->ai_family,
                        res->ai_socktype,
                        res->ai_protocol);

        if (sockfd >= 0)
        {
            /* Set the socket options */
             if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
              perror("setsockopt");
              exit(1);
             }

             int bufsize = 4 * 1024 * 1024;

             if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            	 break;
             close(sockfd);
             sockfd=-1;
         }

      } while ((res = res->ai_next) != NULL);

      freeaddrinfo(ressave);

      if ((sockfd < 0)|| (res == NULL)) {
          fprintf(stderr,
                  "socket error:: could not open socket\n");
          return -1;
      }

      return sockfd;
}


