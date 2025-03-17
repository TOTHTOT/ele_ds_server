#include "weather.h"
#include "common.h"
#include "log.h"

/**
 * @description: 回调函数，用于处理获取天气的json数据
 * @param {void} *ptr 数据指针
 * @param {size_t} size 数据大小
 * @param {size_t} nmemb 数据块数
 * @param {void} *userdata 用户数据, 可以回传数据给调用者
 * @return {*}
 */
size_t get_weather_cb(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t total_size = size * nmemb;
    // print_hex(ptr, total_size); // 打印数据到控制台
    if (total_size > 4096)
    {
        ERROR_PRINT("Data too big: %d", total_size);
        return -1;
    }
    if (userdata == NULL)
    {
        ERROR_PRINT("Invalid argument: userdata is NULL");
        return -2;
    }
    memcpy(userdata, ptr, total_size); // 将数据拷贝到userdata
    // INFO_PRINT("len = %d, data = %s", (int)total_size, (char *)ptr); // 打印数据到控制台
    return total_size;
}

/**
 * @description: 
 * @param {weather_info} *weather
 * @param {uint32_t} weathersize
 * @param {time_t} start_time
 * @param {char} *city
 * @return {*}
 */
int32_t get_weather(struct weather_info *weather,
                           uint32_t weathersize,
                           time_t start_time,
                           uint32_t cityid)
{
    if (weather == NULL || weathersize > WEATHER_DAY_MAX || cityid == 0)
    {
        ERROR_PRINT("Invalid argument: weather is NULL or weathersize too big"); // 记录错误日志
        return -1; // Invalid argument
    }
    (void)start_time; // 防止编译器报错
    int32_t ret = 0;
    char day7_api[sizeof(WEATHER_API_7D_HTTP) + 10] = {0}; // uint32_t 最大值为10位数
    sprintf(day7_api, "%s%d", WEATHER_API_7D_HTTP, cityid);
    INFO_PRINT("api: %s", day7_api); 

    // 通过api获取原始json数据
    char weather_json[4096] = {0};
    ret = get_data_byurl(day7_api, sizeof(day7_api), weather_json,sizeof(weather_json), get_weather_cb);
    if (ret != 0)
    {
        ERROR_PRINT("get_data_byurl() failed: %d", ret);
        return -2;
    }
    INFO_PRINT("weather_json: %s", weather_json); // 打印获取到的json数据
    return 0;
}