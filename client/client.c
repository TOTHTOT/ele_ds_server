/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-25 14:34:45
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-02 11:16:42
 * @FilePath: \ele_ds_server\client\client.c
 * @Description: 用于处理终端发上来的消息
 */
#include "client.h"
#include <cjson/cJSON.h>
#include <time.h>
#include "../weather/weather.h"
#include "../common/common.h"

#define LOG_TAG "client"
#define LOG_LEVEL LOG_LVL_DEBUG
#include "log.h"
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
    cJSON_AddStringToObject(config, "passwd", client_info->cfg.passwd);
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
 * @description: 解析客户端的info信息
 * @param {cJSON} *root JSON 根节点
 * @param {ele_client_info_t} *client_info 客户端信息结构体
 * @return {int32_t} 0:成功 < 0:失败
 */
static int32_t client_analysis_infomsg(cJSON *root, ele_client_info_t *client_info)
{
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
    else
    {
        LOG_E("Failed to parse sensor data\n");
        return -1; // 解析失败
    }

    // 解析客户端配置
    cJSON *config = cJSON_GetObjectItem(root, "cfg");
    if (config)
    {
        
        strncpy(client_info->cfg.username, cJSON_GetObjectItem(config, "username")->valuestring, USER_NAME_SIZE - 1);
        client_info->cfg.username[USER_NAME_SIZE - 1] = '\0';
        strncpy(client_info->cfg.passwd, cJSON_GetObjectItem(config, "passwd")->valuestring, USER_PASSWD_SIZE - 1);
        client_info->cfg.passwd[USER_PASSWD_SIZE - 1] = '\0';
        strncpy(client_info->cfg.cityname, cJSON_GetObjectItem(config, "cityname")->valuestring, CITY_NAME_SIZE - 1);
        client_info->cfg.cityname[CITY_NAME_SIZE - 1] = '\0';
        client_info->cfg.cityid = cJSON_GetObjectItem(config, "cityid")->valueint;
        client_info->cfg.cntserver_interval = cJSON_GetObjectItem(config, "cntserver_interval")->valueint;
        client_info->cfg.version = cJSON_GetObjectItem(config, "version")->valueint;
        client_info->cfg.battery = cJSON_GetObjectItem(config, "battery")->valueint;
    }
    else
    {
        LOG_E("Failed to parse config data\n");
        return -2; // 解析失败
    }
    
    return 0;
}

/**
 * @description: 解析客户端的聊天信息
 * @param {cJSON} *root JSON 根节点
 * @param {ele_client_cheat_t} *client_cheat 客户端聊天信息结构体
 * @return {int32_t} 0:成功 < 0:失败
 */
static int32_t client_analysis_cheatmsg(cJSON *root, ele_client_cheat_t *client_cheat)
{
    // 解析聊天信息
    cJSON *cheat = cJSON_GetObjectItem(root, "cheat");
    if (cheat)
    {
        strncpy(client_cheat->username, cJSON_GetObjectItem(cheat, "username")->valuestring, USER_NAME_SIZE - 1);
        client_cheat->username[USER_NAME_SIZE - 1] = '\0';
        strncpy(client_cheat->target_username, cJSON_GetObjectItem(cheat, "target_username")->valuestring, USER_NAME_SIZE - 1);
        client_cheat->target_username[USER_NAME_SIZE - 1] = '\0';
        strncpy(client_cheat->msg, cJSON_GetObjectItem(cheat, "msg")->valuestring, CLIENT_CHEAT_CONTENT_SIZE - 1);
        client_cheat->msg[CLIENT_CHEAT_CONTENT_SIZE - 1] = '\0';
    }
    else
    {
        LOG_E("Failed to parse cheat data\n");
        return -1; // 解析失败
    }

    return 0;
}
/**
 * @description: 将 JSON 字符串反序列化为结构体数据, 一般服务器使用这个函数
 * @param {char} *json_str JSON 字符串
 * @param {ele_client_msg_t} *client_msg 客户端信息结构体
 * @return {int} >= 0 成功, 返回消息类型 -1:失败
 */
int32_t client_deserialize_from_json(const char *json_str, ele_client_msg_t *client_msg)
{
    if (json_str == NULL || client_msg == NULL)
    {
        LOG_E("Invalid argument: json_str or client_msg is NULL\n");
        return -1; // 参数无效
    }
    int32_t ret = 0;
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL)
        return -2;

    // 解析消息类型
    cJSON *type = cJSON_GetObjectItem(root, "type");
    if (type && cJSON_IsNumber(type))
    {
        client_msg->type = type->valueint;
        LOG_D("Parsed type: %d\n", client_msg->type);
    }
    else
    {
        cJSON_Delete(root);
        LOG_E("Failed to parse type\n");
        return -3; // 缺少或无效的类型字段
    }

    // 默认返回消息类型, 出错的话返回负数
    ret = client_msg->type;
    // 解析消息类型
    switch (client_msg->type)
    {
    case ELE_CLIENTMSG_INFO:
        if (client_analysis_infomsg(root, &client_msg->msg.client_info) != 0)
        {
            LOG_E("Failed to parse client info\n");
            ret = -4; // 解析失败
        }
        break;
    case ELE_CLIENTMSG_CHEAT:
        if (client_analysis_cheatmsg(root, &client_msg->msg.cheat) != 0)
        {
            LOG_E("Failed to parse client cheat\n");
            ret = -5; // 解析失败
        }
        break;
    default:
        LOG_E("Invalid type: %d\n", client_msg->type);
        ret = -6; // 无效的类型
        break;
    }

    cJSON_Delete(root);
    return ret;
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
        printf("Username: %s\n", client_info->cfg.username);
        printf("Password: %s\n", client_info->cfg.passwd);
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
        LOG_E("Failed to parse JSON\n");
        return -1; // 解析失败
    }
    return 0;
}

/**
 * @description: 服务器消息处理函数, 发送消息给客户端
 * @param {int} fd
 * @param {ele_msg_t} *msg
 * @return {*}
 */
int32_t msg_send(int fd, ele_msg_t *msg)
{
    int32_t ret = 0;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "msgtype", msg->msgtype);
    cJSON_AddNumberToObject(root, "packcnt", msg->packcnt);
    // 添加packinfo, 包含实际消息内容, 下位机根据msgtype选择union内容解析
    cJSON *packinfo = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "packinfo", packinfo); // 添加到根节点
    cJSON_AddNumberToObject(packinfo, "len", msg->len);

    switch (msg->msgtype)
    {
    case ELE_SERVERMSG_MEMO:
        cJSON_AddStringToObject(packinfo, "message", msg->data.memo);
        break;
    case ELE_SERVERMSG_WEATHER:
        cJSON_AddNumberToObject(packinfo, "weahterdays", msg->data.weahterdays);
        break;
    case ELE_SERVERMSG_CLIENTUPDATE:
        cJSON_AddNumberToObject(packinfo, "cscrc", msg->data.cs_info.crc);
        cJSON_AddNumberToObject(packinfo, "version", msg->data.cs_info.version);
        cJSON_AddStringToObject(packinfo, "buildinfo", msg->data.cs_info.buildinfo);
        break;
    default:
        LOG_W("Unknown message type: %d\n", msg->msgtype);
        return -1; // 未知消息类型, 处理失败
    }
    char *json_str = cJSON_PrintUnformatted(root);
    LOG_I("Sending message to client: %s", json_str);
    ret = write(fd, json_str, strlen(json_str)); // 发送给客户端
    if (ret < 0)
    {
        LOG_E("send failed: %d\n", ret);
        cJSON_Delete(root);
        return -2;
    }
    cJSON_Delete(root);
    return 0;
}

/**
 * @description: 客户端事件处理函数
 * @param {char} *buf 接收缓冲区
 * @param {uint32_t} len 接收数据长度
 * @return {*}
 */
int32_t client_event_handler(int32_t fd, char *buf, uint32_t len, ele_client_msg_t *client_msg)
{
    if (buf == NULL || len == 0 || client_msg == NULL)
    {
        LOG_E("buf is NULL or len is %d\n", len);
        return -1; // 无效参数
    }

    int32_t ret = 0;
    (void)len; // 防止编译器报错
    (void)fd;
    
    ret = client_deserialize_from_json(buf, client_msg);
    if (ret < 0)
    {
        LOG_E("deserialize from json failed\n");
        return -2;
    }

    ret = 0;
    return ret;
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

