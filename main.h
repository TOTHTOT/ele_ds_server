/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-03 09:37:31
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-03 17:15:56
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


/* 宏定义 */
#define WEATHER_API_DEFAULT_CITYID "101010100" // 默认城市ID
#ifndef WEATHER_API_KEY
#define WEATHER_API_KEY "yourkey"
#endif
#define WEATHER_API_HTTP "https://devapi.qweather.com/v7/weather/7d?key=" WEATHER_API_KEY "&location="

/* 类型定义 */
enum weather_day_index
{
    WEATHER_DAY_YESTERDAY = -1,
    WEATHER_DAY_TODAY = 0,
    WEATHER_DAY_TOMORROW,
    WEATHER_DAY_AFTER_TOMORROW,
    WEATHER_DAY_MAX
};
enum weather_type_index
{
    WEATHER_TYPE_SUNNY = 0,    // 晴
    WEATHER_TYPE_CLOUDY,       // 多云
    WEATHER_TYPE_OVERCAST,     // 阴
    WEATHER_TYPE_RAIN,         // 雨
    WEATHER_TYPE_SNOW,         // 雪
    WEATHER_TYPE_FOG,          // 雾
    WEATHER_TYPE_HAZE,         // 霾
    WEATHER_TYPE_SAND,         // 沙尘
    WEATHER_TYPE_THUNDERSTORM, // 雷阵雨
    WEATHER_TYPE_SHOWER,       // 阵雨
    WEATHER_TYPE_SLEET,        // 雨夹雪
    WEATHER_TYPE_HAIL,         // 冰雹
    WEATHER_TYPE_MAX
};
// 天气信息
struct weather_info
{
    float temperature;       // 温度
    float humidity;          // 湿度
    float wind_speed;        // 风速
    float wind_dir;          // 风向
    enum weather_type_index type; // 天气类型
};

typedef struct send_data
{
    uint8_t daystr[20];   // 显示在屏幕上的内容
    uint8_t username[10]; // 用户名, 广播的数据, 终端用户名匹配才会执行解析
    time_t time;          // 时间戳, 终端可以根据这个设定系统时间

    struct weather_info weather[WEATHER_DAY_MAX]; // 天气信息
} send_data_pack_t; // 发送到终端的数据包

struct ele_ds_server
{
    uint16_t port;      // 服务器端口
    uint32_t clientcnt; // 客户端数量
    uint32_t cityid;    // 城市ID
};
typedef struct ele_ds_server* ele_ds_server_t;

#endif /* __MAIN_H__ */
