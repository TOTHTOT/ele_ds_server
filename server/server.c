/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-25 14:44:07
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-02 11:12:50
 * @FilePath: \ele_ds_server\server\server.c
 * @Description: 电子卓搭服务器相关代码, 处理客户端的tcp连接以及服务器创建
 */
#include "server.h"
#include "../client/client.h"
#include "main.h"
#include <cjson/cJSON.h>
#include "common.h"

#define LOG_TAG "server"
#define LOG_LEVEL LOG_LVL_DEBUG
#include "log.h"

static int32_t server_show_cntclient(server_t *server);
static int32_t server_send_memo(struct server *server, int32_t fd, char *buf, uint32_t len);
static int32_t server_send_update_pack(struct server *server, int32_t fd, char *path);
static void close_client_connection(server_t *server, int index);

void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0); // 获取当前 flags
    if (flags == -1)
    {
        perror("fcntl F_GETFL failed");
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) // 设置非阻塞
    {
        perror("fcntl F_SETFL O_NONBLOCK failed");
    }
}

/**
 * @description: 初始化服务器
 * @param {uint16_t} port 服务器端口
 * @param {uint32_t} max_cltnum 最大客户端连接数
 * @return {*}
 */
// 初始化服务器
int32_t server_init(server_t *server, uint16_t port, client_event_cb cb)
{
    struct sockaddr_in server_addr;
    memset(server, 0, sizeof(server_t)); // 清空服务器结构体

    // 创建socket
    if ((server->server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket creation failed");
        return -1;
    }
    set_nonblocking(server->server_sockfd); // 设置非阻塞这样在poll里面就不会阻塞
    
    // 端口复用, 允许重用本地地址
    int opt = 1;
    setsockopt(server->server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // 绑定端口
    if (bind(server->server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind failed");
        return -1;
    }

    // 监听连接
    if (listen(server->server_sockfd, MAX_CLIENTNUM) == -1)
    {
        perror("listen failed");
        return -1;
    }

    // 设置初始的pollfd数组
    for (int i = 0; i < MAX_CLIENTNUM; i++)
    {
        if (i == 0)
        {
            server->clients.fds[i].fd = server->server_sockfd; // 0号元素存放服务器socket描述符
            strcpy(server->clients.username[i], "root"); // 服务器用户名为root
        }
        else
        {
            server->clients.fds[i].fd = -1; // 初始化客户端socket为-1
            strcpy(server->clients.username[i], ""); // 初始化用户名为空
        }
        server->clients.fds[i].events = POLLIN; // 设置监听POLLIN事件
        // LOG_I("clients.fds[%d].fd = %d\n", i, server->clients.fds[i].fd);
    }
    server->client_event_handler = cb; // 设置客户端事件回调函数
    server->client_count = 0; // 初始化客户端数量为0
    
    server->ops.connected_client = server_show_cntclient; // 设置操作函数
    server->ops.send_memo = server_send_memo; // 设置发送备忘录函数
    server->ops.update_pack_send = server_send_update_pack; // 设置发送升级包函数
    return 0;
}

/**
 * @description: 显示客户端数量
 * @param {server_t} *server 服务器
 * @return {int32_t} 0 成功; -1 失败
 */
static int32_t server_show_cntclient(server_t *server)
{
    if (server == NULL)
    {
        LOG_E("server is NULL\n");
        return -1;
    }
    printf("Connected clients: %d\n", server->client_count);
    // 从1开始, 0是服务器
    for (int i = 1; i < MAX_CLIENTNUM; i++)
    {
        if (server->clients.fds[i].fd != -1)
        {
            printf("Client[%d]: fd = %d, name = %s\n", i, ele_ds_server.server.clients.fds[i].fd, server->clients.username[i]);
        }
    }
    return 0;
}

/**
 * @description: 发送备忘录数据到客户端
 * @param {server} *server 服务器
 * @param {int32_t} fd 客户端文件描述符
 * @param {char} *buf 发送数据缓冲区
 * @param {uint32_t} len 发送数据长度
 * @return {*}
 */
static int32_t server_send_memo(struct server *server, int32_t fd, char *buf, uint32_t len)
{
    if (server == NULL || buf == NULL || len == 0)
    {
        return -1;
    }
    ele_msg_t msg = {0};
    msg.len = len; // 备忘录数据长度
    msg.packcnt = 1;
    msg.msgtype = EMT_SERVERMSG_MEMO; // 备忘录消息类型
    msg.data.memo = buf;              // 备忘录数据
    return msg_send(fd, &msg);        // 发送数据
}

/**
 * @description: 发送升级包到客户端
 * @param {server} *server 服务器
 * @param {int32_t} fd 客户端文件描述符
 * @param {char} *path 升级包路径
 * @return {int32_t} 0 成功; -1 失败; -2 打开文件失败
 */
static int32_t server_send_update_pack(struct server *server, int32_t fd, char *path)
{
    if (server == NULL || path == NULL)
    {
        return -1;
    }

    // 升级包通过json发送, 每次发送10k, 直到发送完毕
    int32_t updatefile = open(path, O_RDONLY);
    if (updatefile == -1)
    {
        LOG_E("open %s failed: %s\n", path, strerror(errno));
        return -2;
    }
    uint32_t filesize = lseek(updatefile, 0, SEEK_END);
    lseek(updatefile, 0, SEEK_SET);

    uint8_t buf[CLIENT_SOFTUPDATE_PACK_SIZE] = {0};
    int32_t ret = 0;
    uint32_t packcnt = 0; // 包序号

    ret = read(updatefile, buf, sizeof(buf)); // 读取文件数据
    ele_msg_t msg = {0};
    msg.msgtype = EMT_SERVERMSG_CLIENTUPDATE;                // 客户端升级消息类型
    msg.packcnt = ++packcnt;                                 // 包序号, 服务器发送的包序号
    msg.len = filesize;                                      // 升级包长度, 客户端根据这个长度来判断是否接收完毕
    msg.data.cs_info.len = ret;                              // 升级包长度, 客户端根据这个长度来判断是否接收完毕, 两个有点重复了
    msg.data.cs_info.version = LAST_CLIENT_SOFTWARE_VERSION; // 升级包版本号
    memcpy(msg.data.cs_info.buildinfo, "build from TOTHTOT", 19); // 测试数据
    msg.data.cs_info.crc = crc32((char *)buf, ret); // 计算crc
    msg_send(fd, &msg);                     // 发送升级包基本信息
    usleep(SERVER_SEND_DATA_INTERVAL); // 等待100ms, 避免头和数据粘连
    ret = write(fd, buf, ret); // 发送升级包数据
    if (ret < 0)
    {
        LOG_E("write failed: %s\n", strerror(errno));
        return -3;
    }
    return 0;
}

/**
 * @description: 处理服务器事件
 * @param {server_t} *server 服务器
 * @return {int8_t} 0 成功; -1 失败
 */
static int8_t server_events(server_t *server)
{
    struct sockaddr_in client_addr;
    int32_t client_sockfd;
    socklen_t addr_len = sizeof(client_addr);

    // 如果server_sockfd无效, 直接返回, 正常会在关闭服务器时置-1
    if (server->server_sockfd == -1)
    {
        return -1;
    }

    client_sockfd = accept(server->server_sockfd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_sockfd == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            // 没有新的连接, 超时导致的错误
            return 0;
        }
        else
        {
            LOG_E("fd = %d", server->server_sockfd);
            perror("accept failed");
            return -1;
        }
    }
    // 查找一个空槽位来存储新客户端
    if (server->client_count <= MAX_CLIENTNUM)
    {
        for (int i = 1; i <= MAX_CLIENTNUM; i++)
        {
            if (server->clients.fds[i].fd == -1)
            {
                server->clients.fds[i].fd = client_sockfd;
                server->clients.fds[i].events = POLLIN;
                set_nonblocking(client_sockfd);
                server->client_count++;
                LOG_I("New client connected: %d\n", client_sockfd);
                break;
            }
        }
    }
    else
    {
        LOG_I("Max client limit reached, rejecting connection\n");
        close(client_sockfd);
    }
    return 0;
}

/**
 * @description: 通过用户名查找对应的终端fd
 * @param {server_t} *server 服务器
 * @param {char} *username 用户名
 * @return {int32_t} -1 参数错误; -2 未找到对应的fd; >= 0 成功
 */
static int32_t find_fd_by_username(server_t *server, const char *username)
{
    if (server == NULL || username == NULL)
    {
        return -1; // 参数错误
    }
    for (int i = 1; i <= MAX_CLIENTNUM; i++)
    {
        if (strcmp(server->clients.username[i], username) == 0)
        {
            return server->clients.fds[i].fd;
        }
    }
    return -2; // 未找到对应的fd
}

/**
 * @description: 处理客户端消息
 * @param {server_t} *server 服务器
 * @param {uint32_t} index 客户端索引 
 * @param {ele_client_msg_t} *client_msg 客户端消息
 * @return {int32_t} 0 成功; -1 参数错误; -2 解析失败; -3 处理失败
 */
static int32_t handle_client_msg(server_t *server, uint32_t index, const ele_client_msg_t *client_msg)
{
    if (!server || !client_msg)
        return -1;

    int32_t ret = 0;
    int32_t fd = server->clients.fds[index].fd;
    
    switch (client_msg->type)
    {
    case EMT_CLIENTMSG_INFO:
        LOG_I("Received client info message\n");
        if (client_show_info(&client_msg->msg.client_info) != 0) // 显示客户端信息
        {
            LOG_E("client_show_info failed\n");
            ret = -3;
            break;
        }

        // 添加用户到数据如果不存在的话
        if (users_add(server->users_db, 
            client_msg->msg.client_info.cfg.username, 
            client_msg->msg.client_info.cfg.passwd) < 0)
        {
            LOG_E("client_add failed\n");
            ret = -3;
            break;
        }

        // 设置fd对应的用户名
        strcpy(server->clients.username[index], client_msg->msg.client_info.cfg.username);

        struct weather_info weather[WEATHER_DAY_MAX]; // 天气信息
        memset(weather, 0, sizeof(weather));          // 初始化结构体
        if (get_weather(weather, WEATHER_DAY_MAX, time(NULL), client_msg->msg.client_info.cfg.cityid) == 0)
        {
            ele_msg_t msg = {
                .msgtype = EMT_SERVERMSG_WEATHER,
                .len = sizeof(weather),
                .packcnt = 1,
                .data.weahterdays = 7,
            };
            msg_send(fd, &msg); // 发送天气数据包头
            usleep(SERVER_SEND_DATA_INTERVAL); // 等待100ms, 避免头和数据粘连
            ret = write(fd, weather, sizeof(weather)); // 发送天气数据
            if (ret < 0)
            {
                LOG_E("write failed: %s\n", strerror(errno));
                ret = -3;
            }
            ret = 0; // 避免影响外部的ret返回
        }
        else
        {
            ret = -3;
            LOG_W("get_weather failed\n");
        }
        break;
    case EMT_CLIENTMSG_CHEAT:
        LOG_I("Received client cheat message\n");
        /* 转发消息到对应客户端:
        1. 需要先检测对应客户端是否在线;
        2. 数据库内是否有这个用户; */
        // 改用户没注册
        if (users_name_exist(server->users_db, client_msg->msg.cheat.target_username) == false)
        {
            LOG_I("cannot find username %s in users table", client_msg->msg.cheat.target_username);
        }
        else
        {
            
        }
        break;
    default:
        LOG_E("deserialize from json failed\n");
        ret = -2;
        break;
    }

    return ret;
}

/**
 * @description: 处理客户端事件
 * @param {server_t} *server 服务器
 * @param {int32_t} i 客户端索引
 * @return {int8_t} 0 成功; -1 失败
 */
static int8_t client_events(server_t *server, int32_t i)
{
    if (server == NULL || i < 0 || i >= MAX_CLIENTNUM)
    {
        return -1;
    }

    char buffer[MAX_MSGLEN] = {0};
    int32_t valread = read(server->clients.fds[i].fd, buffer, sizeof(buffer));
    if (valread == 0)
    {
        // 客户端断开连接
        LOG_I("Client %d disconnected\n", server->clients.fds[i].fd);
        close_client_connection(server, i); // 关闭客户端连接
    }
    else
    {
        ele_client_msg_t client_msg = {0};
        // 处理接收到的消息
        buffer[valread] = '\0';
        LOG_I("i = %d, fd = %d, Received message: %s", i, server->clients.fds[i].fd, buffer);
        // 回复客户端
        char reply[MAX_MSGLEN] = {0};
        sprintf(reply, "Server received len: %ld", strlen(buffer));
        if (server->client_event_handler != NULL)
        {
            int32_t ret = server->client_event_handler(server->clients.fds[i].fd, buffer, strlen(buffer), &client_msg);
            if (ret < 0) // 处理客户端事件
            {
                LOG_W("client_event_handler failed, ret = %d", ret);
            }
            ret = handle_client_msg(server, i, &client_msg);
            if (ret != 0)
            {
                LOG_W("handle_client_msg failed, ret = %d", ret);
            }
        }
        else
            LOG_W("client_event_handler is NULL");
        send(server->clients.fds[i].fd, reply, strlen(reply), 0);
    }
    return 0;
}

/**
 * @description: 处理客户端连接
 * @param {server_t} *server
 * @return {*}
 */
void server_handle_clients(server_t *server)
{
    int ret = poll(server->clients.fds, server->client_count + 1, 500); // 阻塞等待事件, 超时处理避免不能退出
    if (ret == -1)
    {
        perror("poll failed");
        return;
    }
    for (int32_t i = 0; i <= MAX_CLIENTNUM; i++)
    {
        // 处理服务器事件, 检查server_sockfd是否有新的连接请求
        if (server->clients.fds[i].fd == server->server_sockfd)
        {
            server_events(server);
        }
        // 处理客户端事件
        else if (server->clients.fds[i].revents & POLLIN)
        {
            client_events(server, i);
        }
        else
        {
            if (server->clients.fds[i].revents != 0)
            {
                LOG_W("pollfd[%d].revents = %#x, fd = %d, server fd = %d\n",
                              i, server->clients.fds[i].revents, server->clients.fds[i].fd, server->server_sockfd);
            }
        }
    }
}

static void close_client_connection(server_t *server, int index)
{
    int fd = server->clients.fds[index].fd;
    server->client_count--;
    if (fd != -1)
    {
        shutdown(fd, SHUT_RDWR);
        close(fd);

        server->clients.fds[index].fd = -1;
        server->clients.fds[index].events = 0;
        server->clients.fds[index].revents = 0;
        server->clients.username[index][0] = '\0';
    }
}

/**
 * @description: 关闭服务器, 将所有fds都清空
 * @param {server_t} *server 服务器
 * @return {*}
 */
int32_t server_close(server_t *server)
{
    if (!server)
        return -1;

    // 关闭所有客户端连接（从1开始，0是服务器监听socket）
    for (int i = 1; i <= MAX_CLIENTNUM; i++)
    {
        send(server->clients.fds[i].fd, SERVER_SHUTDOWN_MSG, sizeof(SERVER_SHUTDOWN_MSG) - 1, 0);
        close_client_connection(server, i);
    }

    server->client_count = 0;

    // 关闭监听 socket
    if (server->server_sockfd != -1)
    {
        close(server->server_sockfd);
        server->server_sockfd = -1;
    }

    // 清理服务端 socket 在 fds[0] 的记录
    server->clients.fds[0].fd = -1;
    server->clients.fds[0].events = 0;
    server->clients.fds[0].revents = 0;
    server->clients.username[0][0] = '\0';

    printf("Server stopped.\n");
    return 0;
}

#if 0
void signal_handler(int sig)
{
    LOG_I("Signal %d received, exiting...\n", sig);
    exit(0);
}
// 服务器运行主函数
void server_run(server_t *server)
{
    LOG_I("Server running\n");
    while (1)
    {
        server_handle_clients(server);
    }
}

int main()
{
    server_t server = {0};
    if (server_init(&server, 24680) == -1)
    {
        fprintf(stderr, "Server initialization failed\n");
        return -1;
    }
    LOG_I("Server initialized\n");
    server_run(&server);
    return 0;
}
#endif /* test */
