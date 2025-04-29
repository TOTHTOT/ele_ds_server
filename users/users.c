/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-28 15:02:36
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-29 17:14:30
 * @FilePath: \ele_ds_server\users\users.c
 * @Description: 用户管理模块, 处理用户本地数据
 */
#include "users.h"

#define LOG_TAG "users"
#define LOG_LEVEL LOG_LVL_DEBUG
#include "log.h"

#if 1

/**
 * @description: 检查用户名是否存在的回调函数
 * @param {void} *data 回调函数传入的数据
 * @param {int} argc 查询结果的列数
 * @param {char} * argv[] 查询结果的列值
 * @param {char} * azColName[] 查询结果的列名
 * @return {int} 返回 0 表示成功
 */
static int users_name_exist_cb(void *data, int argc, char **argv, char **azColName)
{
    (void)azColName; // 避免未使用参数的警告
    // 如果查询结果中有行数据（即用户名存在），返回 1，否则返回 0
    if (argc > 0 && argv[0] != NULL)
    {
        *(int *)data = 1; // 用户名存在
    }
    else
    {
        *(int *)data = 0; // 用户名不存在
    }
    return 0;
}

/**
 * @description: 检查指定用户名是否存在
 * @param {sqlite3} *db 数据库句柄
 * @param {char} *username 用户名
 * @return {bool} 存在返回true, 不存在返回false
 */
bool users_name_exist(sqlite3 *db, const char *username)
{
    if (username == NULL || db == NULL)
    {
        LOG_E("Invalid argument: username or db is NULL\n");
        return false;
    }
    bool user_exists = 0; // 用于存储结果，1 表示存在，0 表示不存在

    // 创建查询 SQL，检查指定用户名是否存在
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT 1 FROM users WHERE name = '%s' LIMIT 1;", username);

    // 执行查询，结果通过回调函数返回
    int rc = sqlite3_exec(db, sql, users_name_exist_cb, &user_exists, NULL);
    if (rc != SQLITE_OK)
    {
        LOG_E("SQL error: %s\n", sqlite3_errmsg(db));
        return -1; // 错误处理
    }

    return user_exists; // 返回 1 或 0，表示是否存在
}

/**
 * @description: 添加用户
 * @param {sqlite3} *db 数据库句柄
 * @param {char} *username 用户名
 * @param {char} *password 密码
 * @return {int32_t} 返回 0 表示成功，返回负数表示失败, 1 表示用户名已存在
 */
int32_t users_add(sqlite3 *db, const char *username, const char *password)
{
    if (username == NULL || password == NULL /* || db == NULL */)
    {
        LOG_E("Invalid argument: %s%s%s is NULL\n",
              username == NULL ? "username " : "",
              password == NULL ? "password " : "",
              db == NULL ? "db " : "");
        return -1;
    }

    // 检查用户名是否已存在
    if (users_name_exist(db, username))
    {
        LOG_E("Username already exists: %s\n", username);
        return 1; // 用户名已存在
    }

    // 创建插入 SQL
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO users(name, password) VALUES('%s', '%s');", username, password);

    // 执行插入操作
    int rc = sqlite3_exec(db, sql, 0, 0, NULL);
    if (rc != SQLITE_OK)
    {
        LOG_E("SQL error: %s\n", sqlite3_errmsg(db));
        return -3; // 错误处理
    }

    return 0; // 成功
}

/**
 * @description: 检查用户名和密码是否匹配的回调函数
 * @param {void} *privatedata 回调函数传入的数据
 * @param {int} argc 查询结果的列数
 * @param {char} * argv[] 查询结果的列值
 * @param {char} * azColName[] 查询结果的列名
 * @return {int} 返回 0 表示成功
 */
static int users_list_callback(void *privatedata, int argc, char **argv, char **azColName)
{
    (void)privatedata; // 避免未使用参数的警告
    for (int i = 0; i < argc; i++)
    {
        printf("%s = %s ", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

/**
 * @description: 列出所有用户
 * @param {sqlite3} *db 数据库句柄
 * @return {int32_t} 返回 0 表示成功，返回负数表示失败
 */
int32_t users_list(sqlite3 *db)
{
    if (db == NULL)
    {
        LOG_E("Invalid argument: db is NULL\n");
        return -1;
    }
    // 查询测试
    const char *sql_select = "SELECT * FROM users;";
    int rc = sqlite3_exec(db, sql_select, users_list_callback, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        LOG_E("SQL error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -2;
    }
    return 0;
}
/**
 * @description: 初始化用户数据库
 * @param {sqlite3} **db 数据库句柄
 * @param {char} *filepath 数据库文件路径
 * @return {int32_t} 返回 0 表示成功，返回负数表示失败
 */
int32_t users_init(sqlite3 **db, const char *filepath)
{
    if (filepath == NULL)
    {
        LOG_E("Invalid argument: filepath is NULL");
        return -1;
    }

    int rc = sqlite3_open(filepath, db);
    if (rc != SQLITE_OK)
    {
        LOG_E("Cannot open database: %s", sqlite3_errmsg(*db));
        return -2;
    }

    // 创建表
    const char *sql_create = "CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY, name TEXT, password TEXT);";
    rc = sqlite3_exec(*db, sql_create, 0, 0, NULL);
    if (rc != SQLITE_OK)
    {
        LOG_E("SQL error: %s", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return -3;
    }

    return 0;
}


#else // 不把数据读到内存中, 每次使用都查表就好了
struct 
{
    user_info_t *user_info_table; // 用户信息表
    uint32_t table_size;          // 表大小
} users_cb; // 传递给回调函数的参数

static int callback(void *privatedata, int argc, char **argv, char **azColName)
{
    struct users_cb *cb = (struct users_cb *)privatedata;
    for (int i = 0; i < argc; i++)
    {
        if (i >= cb->table_size)
        {
            LOG_E("Table size exceeded\n");
            return -1;
        }
        cb->user_info_table[i].username = argv[0] ? argv[0] : "NULL";
        cb->user_info_table[i].password = argv[1] ? argv[1] : "NULL";
        LOG_I("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    return 0;
}

/**
 * @description: 初始化用户数据库
 * @param {const char} *database_path 数据库路径
 * @param {user_info_t} *user_info_table 用户信息表
 * @param {uint32_t} table_size 表大小
 * @return {*}
 */
int32_t users_init(const char *database_path, user_info_t *user_info_table, uint32_t table_size)
{
    if (database_path == NULL || user_info_table == NULL || table_size == 0)
    {
        LOG_E("Invalid argument: database_path or user_info_table is NULL or table_size is 0\n");
        return -1;
    }

    sqlite3 *db;
    char *err_msg = NULL;
    int rc;

    // 打开数据库，如果不存在则自动创建
    rc = sqlite3_open(database_path, &db);
    if (rc != SQLITE_OK)
    {
        LOG_E("Cannot open database: %s\n", sqlite3_errmsg(db));
        return -2;
    }

    // 创建表
    const char *sql_create = "CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY, name TEXT, password INTEGER);";
    rc = sqlite3_exec(db, sql_create, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        LOG_E("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return -3;
    }
    
#if 0 // 测试插入数据
    // 插入数据
    const char *sql_insert = "INSERT INTO users(name, age) VALUES('Alice', 123456), ('Bob', 123456);";
    rc = sqlite3_exec(db, sql_insert, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        LOG_E("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
#endif
    struct users_cb cb = {
        .user_info_table = user_info_table,
        .table_size = table_size,
    };
    // 查询数据
    const char *sql_select = "SELECT * FROM users;";
    rc = sqlite3_exec(db, sql_select, callback, &cb, &err_msg);
    if (rc != SQLITE_OK)
    {
        LOG_E("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);
    return 0;
}
#endif