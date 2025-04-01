/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-03 09:37:31
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-26 11:25:52
 * @FilePath: \ele_ds_server\main.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <syslog.h> // 添加syslog头文件
#include <time.h>
#include <pthread.h>
#include "weather/weather.h"
#include "server/server.h"

/* 宏定义 */
// #define OPENTEST

/* 类型定义 */
typedef struct send_data
{
    uint8_t daystr[20];   // 显示在屏幕上的内容
    uint8_t username[10]; // 用户名, 广播的数据, 终端用户名匹配才会执行解析
    time_t time;          // 时间戳, 终端可以根据这个设定系统时间

    struct weather_info weather[WEATHER_DAY_MAX]; // 天气信息
} send_data_pack_t; // 发送到终端的数据包

typedef struct ele_ds_server
{
    uint16_t port;      // 服务器端口
    bool exitflag;      // 退出标志 true:退出 false:运行
    server_t server;    // 服务器
    pthread_t server_thread; // 服务器线程
}ele_ds_server_t;

#endif /* __MAIN_H__ */
