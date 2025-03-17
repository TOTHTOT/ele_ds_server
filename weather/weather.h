/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-17 14:11:11
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-20 16:03:12
 * @FilePath: \ele_ds_server\weather\weather.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __WEATHER_H__
#define __WEATHER_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <curl/curl.h>

#define WEATHER_API_DEFAULT_CITYID "101010100" // 默认城市ID
#ifndef WEATHER_API_KEY
#error "Please define WEATHER_API_KEY"
#endif
#define WEATHER_API_7D_HTTP "https://devapi.qweather.com/v7/weather/7d?key=" WEATHER_API_KEY "&location="

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

extern int32_t get_weather(struct weather_info *weather,
                           uint32_t weathersize,
                           time_t start_time,
                           uint32_t cityid);

#endif /* __WEATHER_H__ */

