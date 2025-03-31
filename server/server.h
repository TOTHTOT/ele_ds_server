
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
#include <glib.h>
#include "../weather/weather.h"
#include "../client/client.h"

/* 宏定义 */
#define MAX_CLIENTNUM 30 // 最大客户端连接数
#define MAX_MSGLEN 1024  // 最大消息长度
#define SERVER_PORT 24680 // 服务器端口

/* 类型定义 */
typedef int32_t (*client_event_cb)(char *buf, uint32_t len); // 客户端事件回调函数类型
typedef struct server_t
{
    int server_sockfd;                       // 服务器socket描述符
    struct sockaddr_in server_addr;          // 服务器地址
    struct pollfd fds[MAX_CLIENTNUM + 1]; // 客户端文件描述符数组, 0号元素存放服务器socket描述符
    int client_count;                        // 当前连接的客户端数量
    client_event_cb client_event_handler;    // 客户端事件回调函数
    
    // 使用哈希表保存客户端文件描述符, 使用户名作为键, 文件描述符作为值, 发送数据时通过用户名匹配到客户端

} server_t;

typedef struct
{
    union
    {
        char *memo;                   // 备忘录消息
        struct weather_info *weather; // 天气消息, 7天天气
        uint8_t *client_update;       // 客户端升级消息, 升级包数据
    } data;
    uint32_t len;           // 消息长度
    ele_msg_type_t msgtype; // 消息类型
} ele_msg_t;

typedef struct 
{
    ele_msg_type_t msgtype; // 消息类型
    uint32_t len; // 消息长度
    uint8_t *data; // 消息数据
}server_msg_t;

extern int32_t server_init(server_t *server, uint16_t port, client_event_cb cb);
extern int32_t server_close(server_t *server);
extern void server_handle_clients(server_t *server);
extern int32_t server_msg(server_t *server, int32_t fd, ele_msg_t *msg);
#endif /* __SERVER_H__ */
