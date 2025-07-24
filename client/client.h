/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-25 14:34:53
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-30 10:24:07
 * @FilePath: \ele_ds_server\client\client.h
 * @Description: 用于处理终端发上来的消息
 */
#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* 宏定义 */
#define USER_NAME_SIZE 21 // 用户名最大长度, 实际20字节, 最后一个字节是\0
#define USER_PASSWD_SIZE 21 // 密码最大长度
#define CITY_NAME_SIZE 21 // 城市名字最大长度
#define CLIENT_CHEAT_CONTENT_SIZE 256 // 备忘录消息最大长度

/* 此部分内容需要和服务器同步 开始*/
typedef enum
{
    EMT_CLIENTMSG_NONE = -1,   // 客户端消息类型无效
    EMT_CLIENTMSG_SUCCESS = 0, // 客户端成功
    EMT_CLIENTMSG_FAIL,        // 客户端失败
    EMT_CLIENTMSG_INFO,        // 客户端信息
    EMT_CLIENTMSG_CHEAT,       // 客户端发消息给某个客户端

    // 消息类型区分服务器和客户端
    EMT_SERVERMSG_MEMO = 0x80,  // 服务器备忘录消息
    EMT_SERVERMSG_WEATHER, // 服务器天气消息
    EMT_SERVERMSG_CLIENTUPDATE, // 服务器客户端升级消息
    EMT_SERVERMSG_BACKGROUND_IMG, // 服务器客户端屏幕背景图片
    EMT_SERVERMSG_DEFAULT_SYSFILE, // 服务器客户端默认系统文件
    EMT_SERVERMSG_OTHER_FILE, // 其他文件
    EMT_SERVERMSG_MAX, // 服务器消息最大消息类型, 使用时减去
} ele_msg_type_t; // 消息类型, 和上位机同步
#define MAX_SERVERMSG_NUM (EMT_SERVERMSG_MAX - EMT_SERVERMSG_MEMO)
#define MIN_SERVERMSG_NUM (EMT_SERVERMSG_MEMO)


typedef struct
{
    uint8_t temperature;  // 温度
    uint8_t humidity;     // 湿度
    uint32_t pressure;    // 气压
    uint16_t tvoc;         // VOC
    uint16_t co2;          // CO2
} ele_client_sensor_data_t; // 客户端传感器数据结构体, 没用到 采用json发送

typedef struct
{
    char username[USER_NAME_SIZE];
    char passwd[USER_PASSWD_SIZE];
    char cityname[CITY_NAME_SIZE];
    char location[CITY_NAME_SIZE]; // 城市ID 和 cityname 对应, 查询天气使用
    uint32_t version; // 客户端版本号
    uint8_t battery; // 电池电量
} ele_client_devinfo_t; // 客户端配置结构体, 没用到 采用json发送

typedef struct
{
    ele_client_sensor_data_t sensor_data; // 客户端本地传感器采集到的数据
    ele_client_devinfo_t cfg;                 // 客户端配置
} ele_client_info_t;                      // 客户端连接到服务器要发送过来的消息, 没用到 采用json发送

typedef struct
{
    char username[USER_NAME_SIZE];        // 用户名
    char target_username[USER_NAME_SIZE]; // 目标用户名
    char msg[CLIENT_CHEAT_CONTENT_SIZE];  // 消息内容
} ele_client_cheat_t; // 客户端间聊天信息结构体

#pragma pack(1)
typedef struct client_software_updateinfo
{
    uint32_t crc; // 发送的文件 crc
    uint32_t len; // 文件长度
    uint32_t version; // 文件版本号, 升级包用的
    char info[32]; // 文件信息, 包含文件名称
} send_file_info_t; // 客户端升级包结构体
#pragma pack()

typedef struct
{
    union
    {
        char *memo;                           // 备忘录消息
        int8_t weatherdays;                   // 天气消息, 天数
        send_file_info_t file_info; // 客户端升级包信息
        ele_client_cheat_t cheat;             // 客户端间聊天信息
        union
        {
            ele_client_info_t client_info; // 客户端信息
            char *client_info_str;        // 客户端设备信息
        }client_info;
        uint32_t crc; // 文件CRC
    } data;
    uint32_t len;           // 消息长度
    uint32_t packcnt;       // 消息包序号
    ele_msg_type_t msgtype; // 消息类型

} ele_msg_t; // 消息结构体, 和服务器同步

/* 此部分内容需要和服务器同步 结束 */

/* 函数 */
extern char *client_serialize_to_json(const ele_client_info_t *client_info);                            // 将结构体数据序列化为 JSON 字符串
extern int32_t client_deserialize_from_json(const char *json_str, ele_msg_t *client_msg);        // 将 JSON 字符串反序列化为结构体数据
extern int8_t client_show_info(const ele_client_info_t *client_info);                                   // 显示客户端信息
extern int32_t client_event_handler(int32_t fd, char *buf, uint32_t len, ele_msg_t *client_msg); // 处理客户端事件
extern int32_t msg_send(int fd, ele_msg_t *msg);
#endif /* __CLIENT_H__ */
