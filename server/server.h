/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-25 14:44:17
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-30 16:03:24
 * @FilePath: \ele_ds_server\server\server.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

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
#include "../client/client.h"
#include "../users/users.h"

/* 宏定义 */
#define MAX_CLIENTNUM 30  // 最大客户端连接数
#define MAX_MSGLEN 1024   // 最大消息长度
#define SERVER_PORT 24680 // 服务器端口
// 客户端升级包每包数据大小, base64会导致升级包膨胀, 而tcp最大一包数据1500, 除去协议使用的数据保守起见暂定800字节
#define CLIENT_SOFTUPDATE_PACK_SIZE 800
#define SERVER_SHUTDOWN_MSG "SERVER_SHUTDOWN" // 服务器关闭消息

/* 类型定义 */
typedef int32_t (*client_event_cb)(int32_t fd, char *buf, uint32_t len, ele_client_msg_t *client_msg); // 客户端事件回调函数类型
typedef struct server
{
    int server_sockfd;                       // 服务器socket描述符
    struct sockaddr_in server_addr;          // 服务器地址
    struct
    {
        struct pollfd fds[MAX_CLIENTNUM + 1];             // 客户端文件描述符数组, 0号元素存放服务器socket描述符
        char username[MAX_CLIENTNUM + 1][USER_NAME_SIZE]; // 客户端用户名数组, 用于存储客户端的用户名
    } clients;
    int client_count;                        // 当前连接的客户端数量
    client_event_cb client_event_handler;    // 客户端事件回调函数
    
    // 使用哈希表保存客户端文件描述符, 使用户名作为键, 文件描述符作为值, 发送数据时通过用户名匹配到客户端

    // 操作函数
    struct 
    {
        int32_t (*connected_client)(struct server *server);
        int32_t (*send_memo)(struct server *server, int32_t fd, char *buf, uint32_t len);
        int32_t (*update_pack_send)(struct server *server, int32_t fd, char *path);
    }ops;
    
    sqlite3 *users_db; // 用户数据库句柄
} server_t;

typedef struct 
{
    ele_msg_type_t msgtype; // 消息类型
    uint32_t len; // 消息长度
    uint8_t *data; // 消息数据
}server_msg_t;

extern int32_t server_init(server_t *server, uint16_t port, client_event_cb cb);
extern int32_t server_close(server_t *server);
extern void server_handle_clients(server_t *server);
#endif /* __SERVER_H__ */
