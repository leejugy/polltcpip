#include "server.h"

struct pollfd listen_poll = {
    0,
};
struct pollfd server_poll[SERVER_NUMBER] = {
    0,
};

int server_connect_flag[SERVER_NUMBER] = {
    0,
};

int init_server(int port)
{
    int socket_fd = 0;
    int opt_val = 0;
    struct sockaddr_in addr = {
        0,
    };

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("fail to open socket");
        return -1;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) < 0)
    {
        perror("fail to set socket option");
        return -1;
    }

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    if (bind(socket_fd, (void *)&addr, sizeof(addr)) < 0)
    {
        perror("fail to bind socket");
        return -1;
    }

    if (listen(socket_fd, SERVER_NUMBER) < 0)
    {
        perror("fail to listen socket");
        return -1;
    }

    listen_poll.fd = socket_fd;
    listen_poll.events = POLLIN;
    return 1;
}

int find_not_connected_server()
{
    int loop = 0;

    for (loop = 0; loop < SERVER_NUMBER; loop++)
    {
        if (server_connect_flag[loop] == SERVER_CONNECTED_NONE)
        {
            return loop;
        }
    }

    return -1;
}

int accept_server()
{
    struct sockaddr addr = {
        0,
    };
    int addr_len = sizeof(addr);
    int fd = 0;

    int connecting_client_num = 0;
    connecting_client_num = find_not_connected_server();

    if (connecting_client_num == -1)
    {
        DEBUG("server client number is full");
        return -1;
    }

    if (poll(&listen_poll, sizeof(listen_poll) / sizeof(struct pollfd), POLL_TIMEOUT_CNT) < 0)
    {
        FAIL("no envent at poll");
        return -1;
    }
    else if (listen_poll.revents != POLLIN)
    {
        FAIL("not poll in event [%d]", listen_poll.revents);
        return -1;
    }
    else
    {
        if ((fd = accept(listen_poll.fd, &addr, &addr_len)) < 0)
        {
            perror("fail to accept client");
            return -1;
        }

        server_poll[connecting_client_num].fd = fd;
        server_connect_flag[connecting_client_num] = SERVER_CONNECTED_SUCESS;
        return 1;
    }
}

static void process_recv_server(int loop, uint8_t *recv_buf, size_t recv_size)
{
    DEBUG("SERVER[%d] recv : \"%s\", size %ld", loop, recv_buf, recv_size);
}

void recv_server()
{
    int loop = 0;
    int recv_size = 0;
    STEP step = STEP1;
    uint8_t recv_buf[TEMP_BUF_SIZE] = {
        0,
    };

    while (loop != SERVER_NUMBER)
    {
        switch (step)
        {
        case STEP1:
            if (server_connect_flag[loop] == SERVER_CONNECTED_SUCESS)
            {
                step = STEP2;
            }
            else
            {
                loop++;
            }
            break;

        case STEP2:
            server_poll[loop].events = POLLIN;
            if (poll(&server_poll[loop], sizeof(server_poll[loop]) / sizeof(struct pollfd), POLL_TIMEOUT_CNT) < 0)
            {
                loop++;
                step = STEP1;
            }
            else
            {
                step = STEP3;
            }
            break;

        case STEP3:
            if (server_poll[loop].revents & POLLNVAL ||
                server_poll[loop].revents & POLLHUP)
            {
                DEBUG("close server [%d]", loop);
                close(server_poll[loop].fd);
                server_poll[loop].fd = -1;
                server_connect_flag[loop] = SERVER_CONNECTED_NONE;
            }

            else if (server_poll[loop].revents & POLLIN)
            {
                if ((recv_size = recv(server_poll[loop].fd, recv_buf, sizeof(recv_buf), 0)) < 0)
                {
                    printf("[%d]", loop);
                    perror("fail to recv");
                }
                else
                {
                    process_recv_server(loop, recv_buf, recv_size);
                    memset(recv_buf, 0, sizeof(recv_buf));
                }
            }

            else
            {
                FAIL("recv unkown events : %d", server_poll[loop].revents);
            }
            loop++;
            step = STEP1;
            break;

        default:
            break;
        }
    }
}

void send_server(int client_index, uint8_t *send_buf, size_t send_size)
{
    if (server_connect_flag[client_index] == SERVER_CONNECTED_NONE)
    {
        FAIL("server not connected");
        return;
    }

    server_poll[client_index].events = POLLOUT;
    if (poll(&server_poll[client_index], sizeof(server_poll[client_index]) / sizeof(struct pollfd), POLL_TIMEOUT_CNT) < 0)
    {
        return;
    }

    if (server_poll[client_index].revents & POLLOUT)
    {
        if (server_poll[client_index].revents & POLLNVAL ||
            server_poll[client_index].revents & POLLHUP)
        {
            DEBUG("close server [%d]", client_index);
            close(server_poll[client_index].fd);
            server_poll[client_index].fd = -1;
            server_connect_flag[client_index] = SERVER_CONNECTED_NONE;
        }

        else if (send(server_poll[client_index].fd, send_buf, send_size, 0) < 0)
        {
            printf("[%d]", client_index);
            perror("fail to send");
        }

        else
        {
            SUCCESS("server[%d] send : %s", client_index, send_buf);
        }
    }
    else
    {
        FAIL("send unkown events : %d", server_poll[client_index].revents);
    }
}

static void thread_server()
{
    int loop = 0;
    char temp_buf[TEMP_BUF_SIZE] = {
        0,
    };
    init_server(COMMON_PORT);

    while (1)
    {
        sleep(1);
        accept_server();

        for (loop = 0; loop < SERVER_NUMBER; loop++)
        {
            sprintf(temp_buf, "send[%d]", loop);
            send_server(loop, temp_buf, strlen(temp_buf));
        }

        recv_server();
    }
}

void start_thread_server()
{
    pthread_t tid = 0;

    if (pthread_create(&tid, NULL, (void *)&thread_server, NULL) < 0)
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