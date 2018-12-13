#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "net.h"
void* get_inaddr(struct sockaddr* remote)
{
  if (remote->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in*)remote)->sin_addr);
  }
  return (((struct sockaddr_in6*)remote)->sin6_addr);
}

int start_listen()
{
  int sfd;
  struct addrinfo criteria,* interface,* host;
  int sockopt = 1;

  criteria.ai_family = AF_UNSPEC
  criteria.ai_socktype = SOCK_STREAM;
  criteria.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, "80", &criteria, &interface) < 0)
  {
    perror("getaddrinfo()");
    return -1;
  }

  for (host = interface; host != NULL; host->ai_next)
  {
    if ((sfd = socket(host->ai_family, host->ai_socktype, host->ai_protocol)) == -1)
    {
      perror("socket()");
      continue;
    }

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) == -1)
    {
      perror("setsockopt()");
      close(sfd);
      freeaddrinfo(interface);
      return -2;
    }

    if (bind(sfd, interface->ai_addr, interface->ai_addrlen) == -1)
    {
      close(sfd);
      perror("bind()");
      return -3;
    }

    break;
  }

  freeaddrinfo(interface);
  if (host == NULL)
  {
    perror("No Host");
    return -3;
  }

  if (listen(sfd, SOMAXCONN) == -1)
  {
    perror("listen()");
    close(sfd);
    return -4;
  }

  return sfd;
}
