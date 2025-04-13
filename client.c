#include "client.h"

struct pollfd client_poll[CLIENT_NUMBER] = {
    0,
};

int client_connect_flag[CLIENT_NUMBER] = {0, };

int init_client()
{
    int socket_fd = 0;
    int loop = 0;
    struct timeval opt = {
        0,
    };
    opt.tv_sec = 1; // 500ms

    for (loop = 0; loop < CLIENT_NUMBER; loop++)
    {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0)
        {
            perror("fail to open socket");
            return -1;
        }

        // connect timeout set
        if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(opt)) < 0)
        {
            perror("fail to set sock opt recv timeout");
            return -1;
        }

        client_poll[loop].fd = socket_fd;
    }

    return 1;
}

int find_not_connected_client()
{
    int loop = 0;

    for (loop = 0; loop < CLIENT_NUMBER; loop++)
    {
        if (client_connect_flag[loop] == CLIENT_CONNECTED_NONE)
        {
            return loop;
        }
    }

    return -1;
}

int connect_client(char *connect_addr, int port)
{
    struct sockaddr_in addr = {
        0,
    };

    int client_number = 0;

    addr.sin_addr.s_addr = inet_addr(connect_addr);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    client_number = find_not_connected_client();
    if(client_number == -1)
    {
        DEBUG("all client connected");
        return -1;
    }

    if (connect(client_poll[client_number].fd, (void *)&addr, sizeof(addr)) < 0)
    {
        printf("[%d]", client_number);
        perror("fail to connect client");
        return -1;
    }

    else
    {
        SUCCESS("client[%d] connect success at, IP:%s, PORT:%d", client_number, connect_addr, port);
        client_connect_flag[client_number] = CLIENT_CONNECTED_SUCESS;
        return 1;
    }
}

static void process_recv_client(int loop, uint8_t *recv_buf, size_t recv_size)
{
    DEBUG("CLIENT[%d] recv : \"%s\", size %d", loop, recv_buf, recv_size);
}

void recv_client()
{
    int loop = 0;
    int recv_size = 0;
    STEP step = STEP1;
    uint8_t recv_buf[TEMP_BUF_SIZE] = {
        0,
    };

    while (loop != CLIENT_NUMBER)
    {
        switch (step)
        {
        case STEP1:
            client_poll[loop].events = POLLIN;
            if (poll(&client_poll[loop], sizeof(client_poll[loop]) / sizeof(struct pollfd), POLL_TIMEOUT_CNT) < 0)
            {
                loop++;   
                step = STEP1;
            }
            else
            {
                step = STEP2;
            }
            break;

        case STEP2:
            if(client_connect_flag[loop] == CLIENT_CONNECTED_SUCESS)
            {
                step = STEP3;
            }
            else
            {
                step = STEP1;
                loop++;
            }
            break;

        case STEP3:
            if (client_poll[loop].revents & POLLNVAL ||
                client_poll[loop].revents & POLLHUP)
            {
                DEBUG("close client [%d]", loop);
                close(client_poll[loop].fd);
                client_poll[loop].fd = socket(AF_INET, SOCK_STREAM, 0);
                if(client_poll[loop].fd < 0)
                {
                    perror("fail to socket at recv client");
                }
                client_connect_flag[loop] = CLIENT_CONNECTED_NONE;
            }
            else if (client_poll[loop].revents & POLLIN)
            {
                if ((recv_size = recv(client_poll[loop].fd, recv_buf, sizeof(recv_buf), 0)) < 0)
                {
                    printf("[%d]", loop);
                    perror("fail to recv");
                }
                else
                {
                    process_recv_client(loop, recv_buf, recv_size);
                    memset(recv_buf, 0, sizeof(recv_buf));
                }
            }
            else
            {
                FAIL("recv unkown events : %d", client_poll[loop].revents);
            }
            loop++;
            step = STEP1;
            break;

        default:
            break;
        }
    }
}

void send_client(int client_index, uint8_t *send_buf, size_t send_size)
{
    if (client_connect_flag[client_index] == CLIENT_CONNECTED_NONE)
    {
        FAIL("client not connected");
        return;
    }

    client_poll[client_index].events = POLLOUT;
    if (poll(&client_poll[client_index], sizeof(client_poll[client_index]) / sizeof(struct pollfd), POLL_TIMEOUT_CNT) < 0)
    {
        return;
    }

    if (client_poll[client_index].revents & POLLNVAL ||
        client_poll[client_index].revents & POLLHUP)
    {
        DEBUG("close client [%d]", client_index);
        close(client_poll[client_index].fd);
        client_poll[client_index].fd = socket(AF_INET, SOCK_STREAM, 0);
        if(client_poll[client_index].fd < 0)
        {
            perror("fail to socket at send client");
        }
        client_connect_flag[client_index] = CLIENT_CONNECTED_NONE;
    }

    else if (client_poll[client_index].revents & POLLOUT)
    {
        if (send(client_poll[client_index].fd, send_buf, send_size, POLL_TIMEOUT_CNT) < 0)
        {
            printf("[%d]", client_index);
            perror("fail to send");
        }
        else
        {
            SUCCESS("client[%d] send : %s", client_index, send_buf);
        }
    }
    else
    {
        FAIL("send unkown events : %d", client_poll[client_index].revents);
    }
}

static void thread_client()
{
    int loop = 0;
    char temp_buf[TEMP_BUF_SIZE] = {
        0,
    };
    init_client();

    while (1)
    {
        sleep(1);
        connect_client(CLIENT_CONNECT_ADDRESS, COMMON_PORT);

        for (loop = 0; loop < CLIENT_NUMBER; loop++)
        {
            sprintf(temp_buf, "send[%d]", loop);
            send_client(loop, temp_buf, strlen(temp_buf));
        }

        recv_client();
    }
}

void start_thread_client()
{
    pthread_t tid = 0;

    if (pthread_create(&tid, NULL, (void *)&thread_client, NULL) < 0)
    {
        perror("fail to create client");
        return;
    }

    if (pthread_detach(tid) < 0)
    {
        perror("fail to detach client");
        return;
    }
}