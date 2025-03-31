/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-25 14:44:07
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-31 17:31:25
 * @FilePath: \ele_ds_server\server\server.c
 * @Description: 电子卓搭服务器相关代码, 处理客户端的tcp连接以及服务器创建
 */
#include "server.h"
#include "../log.h"
#include "../client/client.h"
#include "main.h"
#include <cjson/cJSON.h>

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
            server->fds[i].fd = server->server_sockfd; // 0号元素存放服务器socket描述符
        }
        else
        {
            server->fds[i].fd = -1; // 初始化客户端socket为-1
        }
        server->fds[i].events = POLLIN; // 设置监听POLLIN事件
        // INFO_PRINT("fds[%d].fd = %d\n", i, server->fds[i].fd);
    }
    server->client_event_handler = cb; // 设置客户端事件回调函数
    server->client_count = 0; // 初始化客户端数量为0

    return 0;
}

/**
 * @description: 服务器消息处理函数, 发送消息给客户端
 * @param {server_t} *server
 * @param {int} fd
 * @param {ele_msg_t} *msg
 * @return {*}
 */
int32_t server_msg(server_t *server, int fd, ele_msg_t *msg)
{
    int32_t ret = 0;

    if (server == NULL)
    {
        return -1;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "msgtype", msg->msgtype);
    switch (msg->msgtype)
    {
    case ELE_SERVERMSG_MEMO:
        cJSON_AddNumberToObject(root, "len", msg->len);
        cJSON_AddStringToObject(root, "message", msg->data.memo);
        break;
    case ELE_SERVERMSG_WEATHER:
        {
            cJSON *data_array = cJSON_CreateArray();
            struct weather_info *data = msg->data.weather;
            int data_len = msg->len;
            // 结构体数组转换成 JSON
            for (int i = 0; i < data_len; i++)
            {
                cJSON *item = cJSON_CreateObject();
                cJSON_AddStringToObject(item, "fxDate", data[i].fxDate);
                cJSON_AddStringToObject(item, "sunrise", data[i].sunrise);
                cJSON_AddStringToObject(item, "sunset", data[i].sunset);
                cJSON_AddStringToObject(item, "moonrise", data[i].moonrise);
                cJSON_AddStringToObject(item, "moonset", data[i].moonset);
                cJSON_AddStringToObject(item, "moonPhase", data[i].moonPhase);
                cJSON_AddStringToObject(item, "moonPhaseIcon", data[i].moonPhaseIcon);
                cJSON_AddNumberToObject(item, "tempMax", data[i].tempMax);
                cJSON_AddNumberToObject(item, "tempMin", data[i].tempMin);
                cJSON_AddStringToObject(item, "iconDay", data[i].iconDay);
                cJSON_AddStringToObject(item, "textDay", data[i].textDay);
                cJSON_AddStringToObject(item, "iconNight", data[i].iconNight);
                cJSON_AddStringToObject(item, "textNight", data[i].textNight);
                cJSON_AddNumberToObject(item, "wind360Day", data[i].wind360Day);
                cJSON_AddStringToObject(item, "windDirDay", data[i].windDirDay);
                cJSON_AddStringToObject(item, "windScaleDay", data[i].windScaleDay);
                cJSON_AddNumberToObject(item, "windSpeedDay", data[i].windSpeedDay);
                cJSON_AddNumberToObject(item, "wind360Night", data[i].wind360Night);
                cJSON_AddStringToObject(item, "windDirNight", data[i].windDirNight);
                cJSON_AddStringToObject(item, "windScaleNight", data[i].windScaleNight);
                cJSON_AddNumberToObject(item, "windSpeedNight", data[i].windSpeedNight);
                cJSON_AddNumberToObject(item, "humidity", data[i].humidity);
                cJSON_AddNumberToObject(item, "precip", data[i].precip);
                cJSON_AddNumberToObject(item, "pressure", data[i].pressure);
                cJSON_AddNumberToObject(item, "vis", data[i].vis);
                cJSON_AddNumberToObject(item, "cloud", data[i].cloud);
                cJSON_AddNumberToObject(item, "uvIndex", data[i].uvIndex);
    
                cJSON_AddItemToArray(data_array, item);
            }
            cJSON_AddNumberToObject(root, "len", data_len);
            cJSON_AddItemToObject(root, "data", data_array);
        }
        break;
    case ELE_SERVERMSG_CLIENTUPDATE:
        break;
    default:
        WARNING_PRINT("Unknown message type: %d\n", msg->msgtype);
        return -1; // 未知消息类型, 处理失败
    }
    char *json_str = cJSON_PrintUnformatted(root);
    // printf("Sending message to client: %s\n", json_str);
    ret = write(fd, json_str, strlen(json_str)); // 发送给客户端
    if (ret < 0)
    {
        ERROR_PRINT("send failed: %d\n", ret);
        cJSON_Delete(root);
        return -2;
    }
    cJSON_Delete(root);
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
        INFO_PRINT("i = %d, fd = %d, Received message: %s", i, server->fds[i].fd, buffer);
        // 回复客户端
        char reply[MAX_MSGLEN] = {0};
        sprintf(reply, "Server received len: %ld", strlen(buffer));
        if (server->client_event_handler != NULL)
        {
            int32_t ret = server->client_event_handler(buffer, strlen(buffer));
            if (ret < 0) // 处理客户端事件
            {
                ERROR_PRINT("client_event_handler failed, ret = %d\n", ret);
            }
            else if (ret > 0)
            {
                switch (ret)
                {
                case 1:
                {
                    send_data_pack_t send_data;                      // = {0}, 这样赋值会报错初始化语法有问题 [-Wmissing-braces]
                    memset(&send_data, 0, sizeof(send_data_pack_t)); // 初始化结构体
                    get_weather(send_data.weather, WEATHER_DAY_MAX, time(NULL), 101010100);
                    // 测试消息发送
                    ele_msg_t msg = {
                        .msgtype = ELE_SERVERMSG_WEATHER,
                        .len = 7,
                        .data.weather = send_data.weather,
                    };
                    server_msg(server, server->fds[i].fd, &msg);
                    break;
                }
                default:
                    WARNING_PRINT("Unknown return value from client_event_handler: %d\n", ret);
                    break;
                }
            }
        }
        else
            WARNING_PRINT("client_event_handler is NULL\n");
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
