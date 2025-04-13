#ifndef __CLIENT__
#define __CLIENT__

#include "define.h"
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef enum
{
    CLIENT_CONNECTED_NONE,
    CLIENT_CONNECTED_SUCESS,
}SERVER_STATUS;

#define CLIENT_NUMBER 2
#define CLIENT_LOCAL_ADDRESS "127.0.0.1"
#define CLIENT_CONNECT_ADDRESS "2.3.4.5"

int init_client();
int connect_client(char *connect_addr, int port);
void recv_client();
void send_client(int client_index, uint8_t *send_buf, size_t send_size);
void start_thread_client();
#endif