#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* 宏定义 */
#define USER_NAME_SIZE 20 // 用户名最大长度
#define CITY_NAME_SIZE 20 // 城市名字最大长度
/* 类型定义 */
typedef enum
{
    ELE_CLIENTMSG_INFO = 0,     // 客户端信息
    ELE_CLIENTMSG_SUCCESS,      // 客户端成功
    ELE_SERVERMSG_MEMO,         // 服务器备忘录消息
    ELE_SERVERMSG_WEATHER,      // 服务器天气消息
    ELE_SERVERMSG_CLIENTUPDATE, // 服务器客户端升级消息
    ELE_MSG_MAX,                // 最大消息类型
} ele_msg_type_t;               // 消息类型
typedef struct
{
    uint8_t temperature;  // 温度
    uint8_t humidity;     // 湿度
    uint32_t pressure;    // 气压
    uint16_t tvoc;         // VOC
    uint16_t co2;          // CO2
} ele_client_sensor_data_t; // 客户端传感器数据结构体

typedef struct
{
    char username[USER_NAME_SIZE];
    char cityname[CITY_NAME_SIZE];
    uint32_t cityid;             // 城市ID 和 cityname 对应, 查询天气使用
    uint16_t cntserver_interval; // 连接服务器间隔时间
    uint32_t version;            // 客户端版本号
    uint8_t battery;             // 电池电量
} ele_client_cfg_t;              // 客户端配置结构体

typedef struct
{
    uint16_t type; // 消息类型
    ele_client_sensor_data_t sensor_data; // 客户端本地传感器采集到的数据
    ele_client_cfg_t cfg;                 // 客户端配置
} ele_client_info_t;                      // 客户端连接到服务器要发送过来的消息

typedef struct
{
    union
    {
        char *memo;                   // 备忘录消息
        struct weather_info *weather; // 天气消息, 7天天气
        uint8_t *client_update;       // 客户端升级消息, 升级包数据
    } data;
    uint32_t len;           // 消息长度
    ele_msg_type_t msgtype; // 消息类型
} ele_msg_t;                // 消息结构体
/* 函数 */
extern char *client_serialize_to_json(const ele_client_info_t *client_info); // 将结构体数据序列化为 JSON 字符串
extern int client_deserialize_from_json(const char *json_str, ele_client_info_t *client_info); // 将 JSON 字符串反序列化为结构体数据
extern int8_t client_show_info(const ele_client_info_t *client_info); // 显示客户端信息
extern int32_t client_event_handler(int32_t fd, char *buf, uint32_t len); // 处理客户端事件
extern int32_t msg_send(int fd, ele_msg_t *msg);
#endif /* __CLIENT_H__ */
