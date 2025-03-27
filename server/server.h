
#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>

/* 宏定义 */
#define MAX_CLIENTNUM 30 // 最大客户端连接数
#define MAX_MSGLEN 1024  // 最大消息长度
#define SERVER_PORT 24680 // 服务器端口

/* 类型定义 */
typedef struct server_t
{
    int server_sockfd;                       // 服务器socket描述符
    struct sockaddr_in server_addr;          // 服务器地址
    struct pollfd fds[MAX_CLIENTNUM + 1]; // 客户端文件描述符数组, 0号元素存放服务器socket描述符
    int client_count;                        // 当前连接的客户端数量
} server_t;

extern int32_t server_init(server_t *server, uint16_t port);
extern int32_t server_close(server_t *server);
extern void server_handle_clients(server_t *server);

#endif /* __SERVER_H__ */
