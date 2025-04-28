/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-28 15:02:42
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-28 17:16:34
 * @FilePath: \ele_ds_server\users\users.h
 * @Description: 用户管理模块, 处理用户本地数据
 */
#ifndef __USERS_H__
#define __USERS_H__

#include <stdint.h>
#include <stdbool.h>
#include <sqlite3.h>
#include "../client/client.h"

typedef struct user_info
{
    char username[USER_NAME_SIZE];   // 用户名
    char password[USER_PASSWD_SIZE]; // 密码
} user_info_t;

extern int32_t users_init(sqlite3 *db, const char *filepath);
extern bool users_name_exist(sqlite3 *db, const char *username);
extern int32_t users_add(sqlite3 *db, const char *username, const char *password);

#endif /* __USERS_H__ */