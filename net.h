#ifndef _NET_H
#define _NET_H
int start_listen(char* port);
void* get_inaddr(struct sockaddr* addr);
#endif
