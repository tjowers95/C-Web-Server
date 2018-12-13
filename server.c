#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <sys/file.h>
#include <fcntl.h>
#include "net.h"
#include "file.h"
#include "mime.h"
#include "cache.h"

#define PORT "80"
#define SRVRF "./srvfiles"
#define SRVRT "./srvroot"

int send_response(int fd, char* header, char* content_type, void* body, int content_length)
{
  char response[20000];
  //Build HTTP Response
  //
  //Send It
  int rv = send(fd, response, 20000, 0);
  if (rv < 0)
  {
    perror("send()");
  }
  return rv;
}

//Read and Return File from Disk or Cache
//void get_file(int fd, struct cache* cache, char* request_path);

/*
 * Search for the end of the HTTP Header
 * Newlines in HTTP can be \r\n or \n alone or \r alone
 */
//char* find_start_of_body(char* header);

void handle_http_request(int fd, struct cache* cache)
{
  char request[20000];

  //Read Request
  int rbytes = recv(fd, request, 20000-1, MSG_DONTWAIT);
  if (rbytes < 0)
  {
    perror("recv()");
    return;
  }
  /*
   * Read the three components of the Request Header on the first line
   * Handle GET and POST
   */
}

void ev_remove(int fd, int ep_fd)
{
  close(fd);
  epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, NULL);
}

int main (void)
{
	
  int remote_fd, n_fds, ep_fd = epoll_create1(0);       // epoll_create1() creates epoll instance, returns fd that refers to that instance
  struct sockaddr_storage remote_addr;
  char remote_ip[INET6_ADDRSTRLEN];

  struct cache* cache = cache_create(10,0);

  int host_fd = start_listen(PORT);
  if (host_fd < 0){fprintf(stderr, "XN: Fatal Error: Listening Socket\r\n"); return -1;}

  	printf("XN: LISTENING ON PORT %s\r\n", PORT);

  struct epoll_event ev[33333];

  //Add host_fd to epoll instance. Epoll instance will monitor for the activity we specify in the last parameter of epoll_ctl() call
  epoll_ctl(ep_fd, EPOLL_CTL_ADD, host_fd, &(struct epoll_event){ .events = EPOLLIN|EPOLLET, .data.fd = host_fd });
  
  /*
   * The Event Loop
   */
  while (n_fds = epoll_wait(ep_fd, ev, 33333, -1))
  {
	  printf("XN: %d EVENT(S)\r\n", n_fds);
    int n = 0;
    for(n; n<n_fds ; n++)
    {

      if (ev[n].data.fd == host_fd)
      {

        remote_fd = accept(host_fd, (struct sockaddr*)&remote_addr, &(socklen_t){sizeof(remote_addr)});
	fcntl(remote_fd, F_SETFL, fcntl(remote_fd, F_GETFL,0) | O_NONBLOCK);

	inet_ntop(remote_addr.ss_family, get_inaddr((struct sockaddr*)&remote_addr), remote_ip, sizeof(remote_ip));
		printf("XN: Connection On Remote IP %s\r\n", remote_ip);

        epoll_ctl(ep_fd, EPOLL_CTL_ADD, ev[n].data.fd, &(struct epoll_event){.events = EPOLLIN|EPOLLET, .data.fd = ev[n].data.fd});

      }
      else
      {

        if (ev[n].events & EPOLLIN)
	{

	  handle_http_request(ev[n].data.fd, cache);
	  epoll_ctl(ep_fd, EPOLL_CTL_MOD, ev[n].data.fd, &(struct epoll_event){.events = EPOLLOUT|EPOLLET, .data.fd = ev[n].data.fd});

	  if (ev[n].events & EPOLLRDHUP){ ev_remove(ev[n].data.fd, ep_fd);}

	}
	else
	{

	  send_http_response(ev[n].data.fd, cache);
	  epoll_ctl(ep_fd, EPOLL_CTL_MOD, ev[n].data.fd, &(struct epoll_event){.events = EPOLLIN|EPOLLRDHUP|EPOLLET, .data.fd = ev[n].data.fd});

	}
      }
    }
  }
}

