#include "weather.h"
#include "common.h"
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LOG_TAG "weather"
#define LOG_LEVEL LOG_LVL_WARNING
#include "log.h"

#define WEATHER_DAY_MAX 7  // 假设最多解析7天数据

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
    if (total_size > 4096)
    {
        LOG_E("Data too big: %d", total_size);
        return -1;
    }
    if (userdata == NULL)
    {
        LOG_E("Invalid argument: userdata is NULL");
        return -2;
    }
    memcpy(userdata, ptr, total_size); // 将数据拷贝到userdata
    return total_size;
}

/**
 * @description: 获取天气数据
 * @param {weather_info} *weather
 * @param {uint32_t} weathersize
 * @param {time_t} start_time
 * @param {char} *city
 * @return {*}
 */
int32_t get_weather_ex(struct weather_info *weather,
                      uint32_t weathersize,
                      time_t start_time,
                      const void *identifier,
                      bool is_cityid)
{
    if (weather == NULL || weathersize > WEATHER_DAY_MAX ||
        (is_cityid && *(uint32_t*)identifier == 0) ||
        (!is_cityid && identifier == NULL))
    {
        LOG_E("Invalid argument: weather is NULL or weathersize too big or invalid identifier");
        return -1; // Invalid argument
    }

    (void)start_time; // Prevent compiler warning
    int32_t ret = 0;
    char *day7_api = calloc(1, sizeof(WEATHER_API_7D_HTTP) + strlen(identifier));

    if (is_cityid) {
        sprintf(day7_api, "%s%d", WEATHER_API_7D_HTTP, *(uint32_t*)identifier);
    } else {
        LOG_D("111111");
        sprintf(day7_api, "%s%s", WEATHER_API_7D_HTTP, (char*)identifier);
        LOG_D("111111");
    }

    LOG_I("api: %s", day7_api);

    // Get raw json data via api
    char weather_json[14096] = {0};
    ret = get_data_byurl(day7_api, sizeof(day7_api), weather_json, sizeof(weather_json), get_weather_cb);
    if (ret != 0) {
        LOG_E("get_data_byurl() failed: %d", ret);
        return -2;
    }

    LOG_I("weather_json: %s", weather_json); // Print received json data
    if (parse_7day_weather_json(weather_json, weather, weathersize) != 0) {
        return -3;
    }

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
    if (json_string == NULL || weather == NULL || weathersize == 0 || weathersize > WEATHER_DAY_MAX)
    {
        LOG_E("Invalid argument: json_string is NULL or weather is NULL or weathersize invalid");
        return -1;
    }

    // 解析 JSON 字符串
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL)
    {
        LOG_E("JSON 解析失败: %s", cJSON_GetErrorPtr());
        return -2;
    }

    // 提取 code 字段
    cJSON *code = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (!code || !cJSON_IsString(code) || strcmp(code->valuestring, "200") != 0)
    {
        LOG_E("API请求失败，code不为200: %s", code ? code->valuestring : "NULL");
        cJSON_Delete(root);
        return -3;
    }

    // 提取 daily 数组
    cJSON *daily = cJSON_GetObjectItemCaseSensitive(root, "daily");
    if (!daily || !cJSON_IsArray(daily))
    {
        LOG_E("未找到daily数据");
        cJSON_Delete(root);
        return -2;
    }

    cJSON_GetArraySize(daily);
    uint32_t i = 0;
    cJSON *day_item;

    cJSON_ArrayForEach(day_item, daily)
    {
        if (i >= weathersize) break; // 防止越界

        struct weather_info *day_weather = &weather[i];
        cJSON *temp_obj;

        // 日期
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "fxDate");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->fxDate, temp_obj->valuestring, sizeof(day_weather->fxDate)-1);
            day_weather->fxDate[sizeof(day_weather->fxDate)-1] = '\0';
        }

        // 日出日落
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "sunrise");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->sunrise, temp_obj->valuestring, sizeof(day_weather->sunrise)-1);
            day_weather->sunrise[sizeof(day_weather->sunrise)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "sunset");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->sunset, temp_obj->valuestring, sizeof(day_weather->sunset)-1);
            day_weather->sunset[sizeof(day_weather->sunset)-1] = '\0';
        }

        // 月相相关
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "moonrise");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->moonrise, temp_obj->valuestring, sizeof(day_weather->moonrise)-1);
            day_weather->moonrise[sizeof(day_weather->moonrise)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "moonset");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->moonset, temp_obj->valuestring, sizeof(day_weather->moonset)-1);
            day_weather->moonset[sizeof(day_weather->moonset)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "moonPhase");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->moonPhase, temp_obj->valuestring, sizeof(day_weather->moonPhase)-1);
            day_weather->moonPhase[sizeof(day_weather->moonPhase)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "moonPhaseIcon");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->moonPhaseIcon, temp_obj->valuestring, sizeof(day_weather->moonPhaseIcon)-1);
            day_weather->moonPhaseIcon[sizeof(day_weather->moonPhaseIcon)-1] = '\0';
        }

        // 温度
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "tempMax");
        day_weather->tempMax = temp_obj ? atoi(temp_obj->valuestring) : 0;
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "tempMin");
        day_weather->tempMin = temp_obj ? atoi(temp_obj->valuestring) : 0;

        // 天气图标和描述
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "iconDay");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->iconDay, temp_obj->valuestring, sizeof(day_weather->iconDay)-1);
            day_weather->iconDay[sizeof(day_weather->iconDay)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "textDay");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->textDay, temp_obj->valuestring, sizeof(day_weather->textDay)-1);
            day_weather->textDay[sizeof(day_weather->textDay)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "iconNight");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->iconNight, temp_obj->valuestring, sizeof(day_weather->iconNight)-1);
            day_weather->iconNight[sizeof(day_weather->iconNight)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "textNight");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->textNight, temp_obj->valuestring, sizeof(day_weather->textNight)-1);
            day_weather->textNight[sizeof(day_weather->textNight)-1] = '\0';
        }

        // 风向风速（白天）
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "wind360Day");
        day_weather->wind360Day = temp_obj ? (uint16_t)atoi(temp_obj->valuestring) : 0;
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "windDirDay");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->windDirDay, temp_obj->valuestring, sizeof(day_weather->windDirDay)-1);
            day_weather->windDirDay[sizeof(day_weather->windDirDay)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "windScaleDay");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->windScaleDay, temp_obj->valuestring, sizeof(day_weather->windScaleDay)-1);
            day_weather->windScaleDay[sizeof(day_weather->windScaleDay)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "windSpeedDay");
        day_weather->windSpeedDay = temp_obj ? (uint8_t)atoi(temp_obj->valuestring) : 0;

        // 风向风速（夜间）
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "wind360Night");
        day_weather->wind360Night = temp_obj ? (uint16_t)atoi(temp_obj->valuestring) : 0;
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "windDirNight");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->windDirNight, temp_obj->valuestring, sizeof(day_weather->windDirNight)-1);
            day_weather->windDirNight[sizeof(day_weather->windDirNight)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "windScaleNight");
        if (temp_obj && cJSON_IsString(temp_obj)) {
            strncpy(day_weather->windScaleNight, temp_obj->valuestring, sizeof(day_weather->windScaleNight)-1);
            day_weather->windScaleNight[sizeof(day_weather->windScaleNight)-1] = '\0';
        }
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "windSpeedNight");
        day_weather->windSpeedNight = temp_obj ? (uint8_t)atoi(temp_obj->valuestring) : 0;

        // 其他数值字段
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "humidity");
        day_weather->humidity = temp_obj ? (uint8_t)atoi(temp_obj->valuestring) : 0;
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "precip");
        day_weather->precip = temp_obj ? (float)atof(temp_obj->valuestring) : 0.0f;
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "pressure");
        day_weather->pressure = temp_obj ? (uint16_t)atoi(temp_obj->valuestring) : 0;
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "vis");
        day_weather->vis = temp_obj ? (uint8_t)atoi(temp_obj->valuestring) : 0;
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "cloud");
        day_weather->cloud = temp_obj ? (uint8_t)atoi(temp_obj->valuestring) : 0;
        temp_obj = cJSON_GetObjectItemCaseSensitive(day_item, "uvIndex");
        day_weather->uvIndex = temp_obj ? (uint8_t)atoi(temp_obj->valuestring) : 0;

        i++;
    }

    cJSON_Delete(root);
    return 0;
}