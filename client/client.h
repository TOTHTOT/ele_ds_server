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


/* 函数 */
extern char *client_serialize_to_json(const ele_client_info_t *client_info); // 将结构体数据序列化为 JSON 字符串
extern int client_deserialize_from_json(const char *json_str, ele_client_info_t *client_info); // 将 JSON 字符串反序列化为结构体数据

#endif /* __CLIENT_H__ */
