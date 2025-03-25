#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/* 宏定义 */
#define USER_NAME_SIZE 20 // 用户名最大长度
#define CITY_NAME_SIZE 20 // 城市名字最大长度
/* 类型定义 */
typedef struct
{
    char username[USER_NAME_SIZE];
    char cityname[CITY_NAME_SIZE];
}ele_client_info;


#endif /* __CLIENT_H__ */
