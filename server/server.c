/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-25 14:44:07
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-26 11:15:14
 * @FilePath: \ele_ds_server\server\server.c
 * @Description: 电子卓搭服务器相关代码, 处理客户端的tcp连接以及服务器创建
 */
#include "server.h"
#include "../log.h"

/**
 * @description: 初始化服务器
 * @param {uint16_t} port 服务器端口
 * @param {uint32_t} max_cltnum 最大客户端连接数
 * @return {*}
 */
// 初始化服务器
int server_init(server_t *server)
{
    struct sockaddr_in server_addr;

    // 创建socket
    if ((server->server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket creation failed");
        return -1;
    }

    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

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
 * @description: 处理客户端连接
 * @param {server_t} *server
 * @return {*}
 */
void server_handle_clients(server_t *server)
{
    struct pollfd *curfd;
    int client_sockfd;
    char buffer[MAX_MSGLEN];
    int valread;

    int ret = poll(server->fds, server->client_count + 1, -1); // 阻塞等待事件
    if (ret == -1)
    {
        perror("poll failed");
        return;
    }
    for (int32_t i = 0; i <= MAX_CLIENTNUM; i++)
    {
        // 处理服务器事件, 检查server_sockfd是否有新的连接请求
        if (server->fds[i].fd == server->server_sockfd && server->fds[i].revents & POLLIN)
        {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            client_sockfd = accept(server->server_sockfd, (struct sockaddr *)&client_addr, &addr_len);
            if (client_sockfd == -1)
            {
                perror("accept failed");
                return;
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
        }
        // 处理客户端事件
        else if (server->fds[i].revents & POLLIN)
        {
            valread = read(server->fds[i].fd, buffer, sizeof(buffer));
            if (valread == 0)
            {
                // 客户端断开连接
                INFO_PRINT("Client %d disconnected\n", server->fds[i].fd);
                close(server->fds[i].fd);
                server->fds[i].fd = -1; // 将该客户端从pollfd数组中移除
                server->client_count--;
            }
            else
            {
                // 处理接收到的消息
                buffer[valread] = '\0';
                INFO_PRINT("Received message: %s\n", buffer);
                // 回复客户端
                char reply[MAX_MSGLEN] = {0};
                sprintf(reply, "Server received: %s", buffer);
                send(server->fds[i].fd, reply, strlen(reply), 0);
            }
        }
        else
        {
            if (server->fds[i].revents != 0)
            {
                ERROR_PRINT("Error: pollfd[%d].revents = %#x\n", i, server->fds[i].revents);
            }
        }
    }
}

#if 1
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
    if (server_init(&server) == -1)
    {
        fprintf(stderr, "Server initialization failed\n");
        return -1;
    }
    INFO_PRINT("Server initialized\n");
    server_run(&server);
    return 0;
}
#endif /* test */
