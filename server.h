#ifndef __SERVER__
#define __SERVER__

#include "define.h"
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>

typedef enum
{
    SERVER_CONNECTED_NONE,
    SERVER_CONNECTED_SUCESS,
}SERVER_STATUS;

#define SERVER_NUMBER 2

int init_server(int port);
int accept_server();
void recv_server();
void release_server();
void send_server(int client_index, uint8_t *send_buf, size_t send_size);
void start_thread_server();
#endif