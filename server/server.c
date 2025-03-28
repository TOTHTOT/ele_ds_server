/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-25 14:44:07
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-28 15:35:35
 * @FilePath: \ele_ds_server\server\server.c
 * @Description: 电子卓搭服务器相关代码, 处理客户端的tcp连接以及服务器创建
 */
#include "server.h"
#include "../log.h"
#include "../client/client.h"

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
int32_t server_init(server_t *server, uint16_t port)
{
    struct sockaddr_in server_addr;

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
            server->fds[i].fd = server->server_sockfd; // 0号元素存放服务器socket描述符
        }
        else
        {
            server->fds[i].fd = -1;         // 初始化客户端socket为-1
        }
        server->fds[i].events = POLLIN; // 设置监听POLLIN事件
        // INFO_PRINT("fds[%d].fd = %d\n", i, server->fds[i].fd);
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
            ERROR_PRINT("fd = %d", server->server_sockfd);
            perror("accept failed");
            return -1;
        }
    }
    // 查找一个空槽位来存储新客户端
    if (server->client_count <= MAX_CLIENTNUM)
    {
        for (int i = 1; i <= MAX_CLIENTNUM; i++)
        {
            if (server->fds[i].fd == -1)
            {
                server->fds[i].fd = client_sockfd;
                server->fds[i].events = POLLIN;
                set_nonblocking(client_sockfd);
                server->client_count++;
                INFO_PRINT("New client connected: %d\n", client_sockfd);
                break;
            }
        }
    }
    else
    {
        INFO_PRINT("Max client limit reached, rejecting connection\n");
        close(client_sockfd);
    }
    return 0;
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
    int32_t valread = read(server->fds[i].fd, buffer, sizeof(buffer));
    if (valread == 0)
    {
        // 客户端断开连接
        INFO_PRINT("Client %d disconnected\n", server->fds[i].fd);
        close(server->fds[i].fd);
        // 将该客户端从pollfd数组中移除, 并清空对应事件
        server->fds[i].fd = -1;
        server->fds[i].events = 0;
        server->fds[i].revents = 0;
        server->client_count--;
    }
    else
    {
        // 处理接收到的消息
        buffer[valread] = '\0';
        INFO_PRINT("i = %d, fd = %d, Received message: %s\n", i,  server->fds[i].fd, buffer);
        // 回复客户端
        char reply[MAX_MSGLEN] = {0};
        sprintf(reply, "Server received len: %d", strlen(buffer));
        ele_client_info_t client_info = {0};
        if (client_deserialize_from_json(buffer, &client_info) == 0) // 解析客户端发送过来的数据
        {
            if (client_show_info(&client_info) != 0) // 显示客户端信息
            {
                ERROR_PRINT("client_show_info failed\n");
            }
        }
        else
            ERROR_PRINT("client_deserialize_from_json failed\n");
        send(server->fds[i].fd, reply, strlen(reply), 0);
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
    int ret = poll(server->fds, server->client_count + 1, 500); // 阻塞等待事件, 超时处理避免不能退出
    if (ret == -1)
    {
        perror("poll failed");
        return;
    }
    for (int32_t i = 0; i <= MAX_CLIENTNUM; i++)
    {
        // 处理服务器事件, 检查server_sockfd是否有新的连接请求
        if (server->fds[i].fd == server->server_sockfd)
        {
            server_events(server);
        }
        // 处理客户端事件
        else if (server->fds[i].revents & POLLIN)
        {
            client_events(server, i);
        }
        else
        {
            if (server->fds[i].revents != 0)
            {
                WARNING_PRINT("pollfd[%d].revents = %#x, fd = %d, server fd = %d\n",
                              i, server->fds[i].revents, server->fds[i].fd, server->server_sockfd);
            }
        }
    }
}

/**
 * @description: 关闭服务器, 将所有fds都清空
 * @param {server_t} *server 服务器
 * @return {*}
 */
int32_t server_close(server_t *server)
{
    // 关闭所有客户端连接
    for (int i = 1; i <= MAX_CLIENTNUM; i++)
    {
        if (server->fds[i].fd != -1)
        {
            // printf("Shutting down client %d\n", server->fds[i].fd);
            send(server->fds[i].fd, "SERVER_SHUTDOWN", strlen("SERVER_SHUTDOWN"), 0);
            shutdown(server->fds[i].fd, SHUT_RDWR); // 关闭读写
            close(server->fds[i].fd);
            server->fds[i].fd = -1;
            server->fds[i].events = 0;
            server->fds[i].revents = 0;
        }
    }
    server->client_count = 0;
    // 关闭监听 socket
    close(server->server_sockfd);
    // usleep(1000*1000);
    server->server_sockfd = -1; // 关闭后置-1
    server->fds[0].fd = -1;
    server->fds[0].events = 0;
    server->fds[0].revents = 0;
    printf("Server stopped.\n");
    return 0;
}

#if 0
void signal_handler(int sig)
{
    INFO_PRINT("Signal %d received, exiting...\n", sig);
    exit(0);
}
// 服务器运行主函数
void server_run(server_t *server)
{
    INFO_PRINT("Server running\n");
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
    INFO_PRINT("Server initialized\n");
    server_run(&server);
    return 0;
}
#endif /* test */
