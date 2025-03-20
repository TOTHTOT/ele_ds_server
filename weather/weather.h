/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-17 14:11:11
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-20 16:51:11
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
    WEATHER_DAY_MAX = 7
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
    char fxDate[11];        // 日期 (如 "2025-03-20")
    char sunrise[6];        // 日出时间 (如 "06:19")
    char sunset[6];         // 日落时间 (如 "18:26")
    char moonrise[6];       // 月出时间 (允许为空)
    char moonset[6];        // 月落时间 (如 "08:53")
    char moonPhase[16];     // 月相描述 (如 "亏凸月")
    char moonPhaseIcon[4];  // 月相图标 ID (如 "805")
    int8_t tempMax;         // 最高温度 (如 "24")
    int8_t tempMin;         // 最低温度 (如 "10")
    char iconDay[4];        // 白天图标 ID (如 "100")
    char textDay[16];       // 白天天气描述 (如 "晴")
    char iconNight[4];      // 夜间图标 ID (如 "150")
    char textNight[16];     // 夜间天气描述 (如 "晴")
    uint16_t wind360Day;    // 白天风向角度 (如 "315")
    char windDirDay[16];    // 白天风向 (如 "西北风")
    char windScaleDay[8];   // 白天风力等级 (如 "1-3")
    uint8_t windSpeedDay;   // 白天风速 (如 "3")
    uint16_t wind360Night;  // 夜间风向角度 (如 "270")
    char windDirNight[16];  // 夜间风向 (如 "西风")
    char windScaleNight[8]; // 夜间风力等级 (如 "1-3")
    uint8_t windSpeedNight; // 夜间风速 (如 "3")
    uint8_t humidity;       // 湿度 (如 "21")
    float precip;           // 降水量 (如 "0.0")
    uint16_t pressure;      // 气压 (如 "1009")
    uint8_t vis;            // 能见度 (如 "25")
    uint8_t cloud;          // 云量 (如 "0")
    uint8_t uvIndex;        // 紫外线指数 (如 "5")
};

extern int32_t get_weather(struct weather_info *weather,
                           uint32_t weathersize,
                           time_t start_time,
                           uint32_t cityid);

#endif /* __WEATHER_H__ */
