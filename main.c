#include "main.h"
#include "log.h"
#include <errno.h>

/**
 * @description: 
 * @param {weather_info} *weather
 * @param {uint32_t} weathersize
 * @param {time_t} start_time
 * @param {char} *city
 * @return {*}
 */
static int32_t get_weather(struct weather_info *weather,
                           uint32_t weathersize,
                           time_t start_time,
                           uint32_t cityid)
{
    if (weather == NULL)
    {
        ERROR_PRINT("Invalid argument: weather or city is NULL"); // 记录错误日志
        return -EINVAL; // Invalid argument
    }
    char day7_api[sizeof(WEATHER_API_HTTP) + 10] = {0}; // uint32_t 最大值为10位数
    sprintf(day7_api, "%s%d", WEATHER_API_HTTP, cityid); // 拼接API
    INFO_PRINT("api: %s", day7_api); // 记录调试日志
    return 0;
}

int main(int argc, char *argv[])
{
    send_data_pack_t send_data = {0};
    get_weather(send_data.weather, sizeof(send_data.weather), time(NULL), 101010100);


    closelog(); // 关闭日志系统
    return 0;
}