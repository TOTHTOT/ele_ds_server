#include "client.h"
#include "../log.h"
#include <cjson/cJSON.h>
#include <time.h>

// 生成 JSON
/**
 * @description: 将结构体数据序列化为 JSON 字符串, 一般客户端使用这个函数
 * @param {ele_client_info_t} *client_info 客户端信息结构体
 * @return {char *} JSON 字符串
 * @note: 需要调用者释放内存, 使用 free(json_str) 释放
 */
char *client_serialize_to_json(const ele_client_info_t *client_info)
{
    cJSON *root = cJSON_CreateObject();

    // 添加消息类型
    cJSON_AddNumberToObject(root, "type", client_info->type);

    // 传感器数据
    cJSON *sensor = cJSON_CreateObject();
    cJSON_AddNumberToObject(sensor, "temperature", client_info->sensor_data.temperature);
    cJSON_AddNumberToObject(sensor, "humidity", client_info->sensor_data.humidity);
    cJSON_AddNumberToObject(sensor, "pressure", client_info->sensor_data.pressure);
    cJSON_AddNumberToObject(sensor, "tvoc", client_info->sensor_data.tvoc);
    cJSON_AddNumberToObject(sensor, "co2", client_info->sensor_data.co2);
    cJSON_AddItemToObject(root, "sensor_data", sensor);

    // 客户端配置
    cJSON *config = cJSON_CreateObject();
    cJSON_AddStringToObject(config, "username", client_info->cfg.username);
    cJSON_AddStringToObject(config, "cityname", client_info->cfg.cityname);
    cJSON_AddNumberToObject(config, "cityid", client_info->cfg.cityid);
    cJSON_AddNumberToObject(config, "cntserver_interval", client_info->cfg.cntserver_interval);
    cJSON_AddNumberToObject(config, "version", client_info->cfg.version);
    cJSON_AddNumberToObject(config, "battery", client_info->cfg.battery);
    cJSON_AddItemToObject(root, "cfg", config);

    // 转换为 JSON 字符串
    // char *json_str = cJSON_Print(root);
    char *json_str = cJSON_PrintUnformatted(root); // 不格式化输出
    cJSON_Delete(root);
    return json_str;
}

/**
 * @description: 将 JSON 字符串反序列化为结构体数据, 一般服务器使用这个函数
 * @param {char} *json_str JSON 字符串
 * @param {ele_client_info_t} *client_info 客户端信息结构体
 * @return {int} 0:成功 -1:失败
 */
int client_deserialize_from_json(const char *json_str, ele_client_info_t *client_info)
{
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL)
        return -1;

    // 解析消息类型
    cJSON *type = cJSON_GetObjectItem(root, "type");
    if (type && cJSON_IsNumber(type)) {
        client_info->type = type->valueint;
    } else {
        cJSON_Delete(root);
        return -1; // 缺少或无效的类型字段
    }

    // 解析传感器数据
    cJSON *sensor = cJSON_GetObjectItem(root, "sensor_data");
    if (sensor)
    {
        client_info->sensor_data.temperature = cJSON_GetObjectItem(sensor, "temperature")->valueint;
        client_info->sensor_data.humidity = cJSON_GetObjectItem(sensor, "humidity")->valueint;
        client_info->sensor_data.pressure = cJSON_GetObjectItem(sensor, "pressure")->valueint;
        client_info->sensor_data.tvoc = cJSON_GetObjectItem(sensor, "tvoc")->valueint;
        client_info->sensor_data.co2 = cJSON_GetObjectItem(sensor, "co2")->valueint;
    }

    // 解析客户端配置
    cJSON *config = cJSON_GetObjectItem(root, "cfg");
    if (config)
    {
        strncpy(client_info->cfg.username, cJSON_GetObjectItem(config, "username")->valuestring, USER_NAME_SIZE);
        strncpy(client_info->cfg.cityname, cJSON_GetObjectItem(config, "cityname")->valuestring, CITY_NAME_SIZE);
        client_info->cfg.cityid = cJSON_GetObjectItem(config, "cityid")->valueint;
        client_info->cfg.cntserver_interval = cJSON_GetObjectItem(config, "cntserver_interval")->valueint;
        client_info->cfg.version = cJSON_GetObjectItem(config, "version")->valueint;
        client_info->cfg.battery = cJSON_GetObjectItem(config, "battery")->valueint;
    }

    cJSON_Delete(root);
    return 0;
}

/**
 * @description: 显示客户端信息
 * @param {ele_client_info_t} *client_info 客户端信息结构体
 * @return {int8_t} 0:成功 -1:失败
 */
int8_t client_show_info(const ele_client_info_t *client_info)
{
    if (client_info != NULL)
    {
        time_t raw_time;
        struct tm *time_info;
        time(&raw_time);
        time_info = localtime(&raw_time);
        // 获取时间字符串
        char *time_str = asctime(time_info);

        // 直接通过指针去掉换行符
        if (time_str)
        {
            time_str[strlen(time_str) - 1] = '\0';
        }
        printf("\nRecv time:%s, Parsed Data:\n", time_str);
        printf("Type: %u\n", client_info->type);
        printf("Username: %s\n", client_info->cfg.username);
        printf("City: %s\n", client_info->cfg.cityname);
        printf("City ID: %u\n", client_info->cfg.cityid);
        printf("Interval: %u sec\n", client_info->cfg.cntserver_interval);
        printf("Version: %u\n", client_info->cfg.version);
        printf("Battery: %u%%\n", client_info->cfg.battery);
        printf("Temperature: %u°C\n", client_info->sensor_data.temperature);
        printf("Humidity: %u%%\n", client_info->sensor_data.humidity);
        printf("Pressure: %u Pa\n", client_info->sensor_data.pressure);
        printf("TVOC: %u ppb\n", client_info->sensor_data.tvoc);
        printf("CO2: %u ppm\n", client_info->sensor_data.co2);
    }
    else
    {
        ERROR_PRINT("Failed to parse JSON\n");
        return -1; // 解析失败
    }
    return 0;
}

/**
 * @description: 客户端事件处理函数
 * @param {char} *buf 接收缓冲区
 * @param {uint32_t} len 接收数据长度
 * @return {*}
 */
int32_t client_event_handler(char *buf, uint32_t len)
{
    ele_client_info_t client_info = {0};
    (void)len; // 防止编译器报错
    
    if (client_deserialize_from_json(buf, &client_info) == 0) // 解析客户端发送过来的数据
    {
        if (client_show_info(&client_info) != 0) // 显示客户端信息
        {
            ERROR_PRINT("client_show_info failed\n");
            return -2;
        }
    }
    else
    {
        ERROR_PRINT("client_deserialize_from_json failed\n");
        return -1;
    }
    return 0;
}

#if 0
// gcc -o client client.c -lcjson # 编译命令
int main()
{
    ele_client_info_t client_info = {
        .type = 1,
        .sensor_data = {25, 60, 101325, 50, 400},
        .cfg = {"test_user", "Beijing", 110000, 30, 20240328, 85},
    };

    // 生成 JSON
    char *json_str = client_serialize_to_json(&client_info);
    printf("Generated JSON:\n%s\n", json_str);

    // 解析 JSON
    ele_client_info_t new_client_info;
    memset(&new_client_info, 0, sizeof(new_client_info));
    if (client_deserialize_from_json(json_str, &new_client_info) == 0)
    {
        printf("\nParsed Data:\n");
        printf("Type: %u\n", new_client_info.type);
        printf("Username: %s\n", new_client_info.cfg.username);
        printf("City: %s\n", new_client_info.cfg.cityname);
        printf("City ID: %u\n", new_client_info.cfg.cityid);
        printf("Interval: %u sec\n", new_client_info.cfg.cntserver_interval);
        printf("Version: %u\n", new_client_info.cfg.version);
        printf("Battery: %u%%\n", new_client_info.cfg.battery);
        printf("Temperature: %u°C\n", new_client_info.sensor_data.temperature);
        printf("Humidity: %u%%\n", new_client_info.sensor_data.humidity);
        printf("Pressure: %u Pa\n", new_client_info.sensor_data.pressure);
        printf("TVOC: %u ppb\n", new_client_info.sensor_data.tvoc);
        printf("CO2: %u ppm\n", new_client_info.sensor_data.co2);
    }
    else
    {
        printf("Failed to parse JSON\n");
    }

    free(json_str); // 释放 JSON 字符串
    return 0;
}
#endif

