/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-03 09:35:51
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-26 13:50:03
 * @FilePath: \ele_ds_server\main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "main.h"
#include "log.h"
#include "common.h"
#include "weather.h"
#include "./client/client.h"
#include <errno.h>
#include <signal.h>
#include <unistd.h>

/* 全局变量 */
ele_ds_server_t ele_ds_server = {0};

/* 函数声明 */
static void *server_thread(void *arg);


/**
 * @description: 测试相关函数
 * @return {*}
 */
void test_func()
{
    int32_t ret = 0;
    send_data_pack_t send_data;                      // = {0}, 这样赋值会报错初始化语法有问题 [-Wmissing-braces]
    memset(&send_data, 0, sizeof(send_data_pack_t)); // 初始化结构体
    get_weather(send_data.weather, WEATHER_DAY_MAX, time(NULL), 101010100);

    ret = server_init(&ele_ds_server.server, SERVER_PORT, client_event_handler);
    if (ret != 0)
    {
        ERROR_PRINT("Server initialization failed\n");
        return;
    }
    else
    {
        pthread_create(&ele_ds_server.server_thread, NULL, server_thread, &ele_ds_server.server);
        INFO_PRINT("Server initialized\n");
    }
}

static void *server_thread(void *arg)
{
    server_t *server = (server_t *)arg;
    INFO_PRINT("Server thread started\n");
    while (ele_ds_server.exitflag == false)
    {
        server_handle_clients(server);
        usleep(100 * 1000);
    }
    INFO_PRINT("Server thread exited\n");
    return NULL;
}


void signal_handler(int signo)
{
    switch (signo)
    {
    case SIGINT:
        INFO_PRINT("Signal %d received, exiting...\n", signo);
        ele_ds_server.exitflag = true;
        break;
    
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    // 注册信号
    signal(SIGINT, signal_handler);
#ifdef OPENTEST
    test_func();
#endif /* OPENTEST */
    while (ele_ds_server.exitflag == false)
    {
        usleep(100 * 1000);
    }
    server_close(&ele_ds_server.server);
    pthread_join(ele_ds_server.server_thread, NULL);
    return 0;
}