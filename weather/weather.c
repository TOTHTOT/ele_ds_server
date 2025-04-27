#include "weather.h"
#include "common.h"
#include "log.h"
#include <cjson/cJSON.h>

static int8_t parse_7day_weather_json(const char *json_string, struct weather_info *weather, uint32_t weathersize);


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
    uint32_t total_size = size * nmemb;
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
    // INFO_PRINT("weather_json: %s", weather_json); // 打印获取到的json数据
    if (parse_7day_weather_json(weather_json, weather, weathersize) != 0) // 解析json数据
        return -3;
    return 0;
}

/**
 * @description: 解析7天天气json数据
 * @param {char} *json_string json数据
 * @param {weather_info} *weather 天气数据
 * @param {uint32_t} weathersize 天气数据大小
 * @return {int8_t} 0:成功 -1:无效参数 -2:JSON解析失败 -3:code不为200
 */
static int8_t parse_7day_weather_json(const char *json_string, struct weather_info *weather, uint32_t weathersize)
{
    if (json_string == NULL || weather == NULL || weathersize > WEATHER_DAY_MAX)
    {
        ERROR_PRINT("Invalid argument: json_string is NULL or weather is NULL or weathersize too big");
        return -1;
    }
    // 解析 JSON 字符串
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL)
    {
        ERROR_PRINT("JSON 解析失败\n");
        return -2;
    }

    // 提取 code 字段
    cJSON *code = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (code == NULL)
    {
        ERROR_PRINT("code is NULL\n");
        cJSON_Delete(root);
        return -2;
    }
    
    if (cJSON_IsString(code))
    {
        // INFO_PRINT("code: %s\n", code->valuestring);
    }

    // code 不对说明获取失败
    if (strcmp(code->valuestring, "200") != 0)
    {
        ERROR_PRINT("code is not 200: %s", code->valuestring);
        cJSON_Delete(root);
        return -3;
    }

    // 提取 updateTime 字段
    cJSON *update_time = cJSON_GetObjectItemCaseSensitive(root, "updateTime");
    if (cJSON_IsString(update_time))
    {
        // INFO_PRINT("updateTime: %s\n", update_time->valuestring);
    }

    // 提取 daily 数组
    cJSON *daily = cJSON_GetObjectItemCaseSensitive(root, "daily");
    if (cJSON_IsArray(daily))
    {
        cJSON *day;
        cJSON_ArrayForEach(day, daily)
        {
            // 提取 fxDate 字段
            cJSON *fx_date = cJSON_GetObjectItemCaseSensitive(day, "fxDate");
            if (cJSON_IsString(fx_date))
            {
                strcpy(weather->fxDate, fx_date->valuestring);
                // INFO_PRINT("fxDate: %s\n", fx_date->valuestring);
            }

            // 提取 tempMax 和 tempMin 字段
            cJSON *temp_max = cJSON_GetObjectItemCaseSensitive(day, "tempMax");
            cJSON *temp_min = cJSON_GetObjectItemCaseSensitive(day, "tempMin");
            if (cJSON_IsString(temp_max) && cJSON_IsString(temp_min))
            {
                weather->tempMax = atoi(temp_max->valuestring);
                weather->tempMin = atoi(temp_min->valuestring);
                // INFO_PRINT("tempMax: %s, tempMin: %s\n", temp_max->valuestring, temp_min->valuestring);
            }

            // 提取 textDay 和 textNight 字段
            cJSON *text_day = cJSON_GetObjectItemCaseSensitive(day, "textDay");
            cJSON *text_night = cJSON_GetObjectItemCaseSensitive(day, "textNight");
            if (cJSON_IsString(text_day) && cJSON_IsString(text_night))
            {
                strcpy(weather->textDay, text_day->valuestring);
                strcpy(weather->textNight, text_night->valuestring);
                // INFO_PRINT("textDay: %s, textNight: %s\n", text_day->valuestring, text_night->valuestring);
            }
        }
    }

    // 释放 JSON 对象
    cJSON_Delete(root);
    return 0;
}
