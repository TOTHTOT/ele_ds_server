#include "command.h"
#include "../main.h"
#include "../server/server.h"
#include "../common/common.h"
#include "../users/users.h"
#include <string.h>


#define LOG_TAG "command"
#define LOG_LEVEL LOG_LVL_DEBUG
#include "log.h"
#define MAX_ARGS 10 // 最大参数数量

// 命令处理函数原型
typedef void (*command_func_t)(int argc, char *args[]);

/* 函数声明 */
void handle_status(int argc, char *args[]);
void handle_help(int argc, char *args[]);

// 简单的命令结构体
typedef struct
{
    const char *name;
    command_func_t func;
    const char *usage;
} command_t;

// 处理 status 命令
void handle_status(int argc, char *args[])
{
    if (argc < 2)
    {
        printf("Usage: status <clients|server>\n");
        return;
    }

    if (strcmp(args[1], "clients") == 0)
    {
        if (ele_ds_server.server.ops.connected_client != NULL)
            ele_ds_server.server.ops.connected_client(&ele_ds_server.server);
        else
            LOG_E("connected_client is null.\n");
    }
    else if (strcmp(args[1], "server") == 0)
    {
        printf("Server status: Running\n");
    }
    else
    {
        printf("Unknown status parameter: %s\n", args[1]);
    }
}

#if 0
/**
 * @description: 处理 memo send 命令
 * @param {int} argc 参数数量
 * @param {char **} args 参数列表
 * @return {void}
 */
void handle_memo_send(int argc, char **args)
{
    if (argc < 4)
    {
        printf("Usage: memo send <fd> <msg>\n");
        return;
    }

    int fd = atoi(args[2]);

    // 处理消息，支持带空格的字符串
    char raw_msg[256] = {0};
    for (int i = 3; i < argc; i++)
    {
        strcat(raw_msg, args[i]);
        if (i < argc - 1)
            strcat(raw_msg, " ");
    }
    ele_ds_server.server.ops.send_memo(&ele_ds_server.server, fd, raw_msg, strlen(raw_msg));
}
#endif /* 0 */

// 处理 memo 命令
void handle_memo(int argc, char *args[])
{
    if (argc < 2)
    {
        printf("Usage: memo <help|send>\n");
        return;
    }

    if (strcmp(args[1], "help") == 0)
    {
        printf("Usage:\n  memo send <fd> <msg>\n");
    }
    else if (strcmp(args[1], "send") == 0)
    {
        if (argc < 4)
            printf("Usage: memo send <fd> <msg>\n");
        else
        {
            int fd = atoi(args[2]);
            // 处理消息，支持带空格的字符串
            char raw_msg[256] = {0};
            for (int i = 3; i < argc; i++)
            {
                strcat(raw_msg, args[i]);
                if (i < argc - 1)
                    strcat(raw_msg, " ");
            }
            ele_ds_server.server.ops.send_memo(&ele_ds_server.server, fd, raw_msg, strlen(raw_msg));
        }
    }
    else
    {
        printf("Unknown memo parameter: %s\n", args[1]);
    }
}

// 处理 exit 命令
void handle_exit(int argc, char *args[])
{
    (void)argc; // 忽略参数数量
    (void)args; // 忽略参数列表
    printf("Exiting...\n");
    ele_ds_server.exitflag = true; // 设置退出标志
}

/**
 * @description: 处理客户端升级命令
 * @param {int} argc 参数数量
 * @param {char} *args 参数列表 fd path
 * @note: fd == MAX_CLIENTNUM + 1, 代表所有客户端都升级
 * @return {*}
 */
void handle_csupdate(int argc, char *args[])
{
    if (argc < 3)
    {
        printf("Usage: csupdate <fd> <path>\n");
        return;
    }

    int fd = atoi(args[1]);
    char *path = args[2];
    ele_ds_server.server.ops.send_file(ELE_DS_SFT_UPDATEFILE, &ele_ds_server.server, fd, path);
}

void handle_sendfile(int argc, char *args[])
{
    if (argc < 3)
    {
        printf("Usage: sendfile <fd> <path>\n");
        return;
    }

    int fd = atoi(args[1]);
    char *path = args[2];
    ele_ds_server.server.ops.send_file(ELE_DS_SFT_OTHER, &ele_ds_server.server, fd, path);
}

void handle_def_sysfile(int argc, char *args[])
{
    if (argc < 3)
    {
        printf("Usage: def_sysfile <fd> <path>\n");
        return;
    }

    int fd = atoi(args[1]);
    char *path = args[2];
    ele_ds_server.server.ops.send_file(ELE_DS_SFT_DEFAULT_SYSFILE, &ele_ds_server.server, fd, path);
}

void handle_bgimage(int argc, char *args[])
{
    if (argc < 3)
    {
        printf("Usage: bgimage <fd> <path>\n");
        return;
    }

    int fd = atoi(args[1]);
    char *path = args[2];
    ele_ds_server.server.ops.send_file(ELE_DS_SFT_BGIMAGE, &ele_ds_server.server, fd, path);
}

void handle_users(int argc, char *args[])
{
    (void)argc; // 忽略参数数量
    (void)args; // 忽略参数列表
    users_list(ele_ds_server.server.users_db); // 列出所有用户
}

// 创建命令表
command_t commands[] = {
    {"help", handle_help, "Show available commands"},
    {"exit", handle_exit, "Exit the program"},
    {"status", handle_status, "Show server status"},
    {"memo", handle_memo, "Handle memo actions"},
    {"csupdate", handle_csupdate, "Handle client soft update"},
    {"defsysfile", handle_def_sysfile, "Send default system file to client"},
    {"sendfile", handle_sendfile, "Send a file to client"},
    {"bgimage", handle_bgimage, "Send background image to client"},
    {"users", handle_users, "Show users info"},
};
#define CMD_COUNT (sizeof(commands) / sizeof(commands[0]))
// 处理 help 命令
void handle_help(int argc, char *args[])
{
    (void)argc; // 忽略参数数量
    (void)args; // 忽略参数列表

    // 输出 commands
    printf("Available commands:\n");
    for (size_t i = 0; i < CMD_COUNT; i++)
    {
        printf("  %s: %s\n", commands[i].name, commands[i].usage);
    }


    // printf("Available commands: help, exit, status, memo, users\n");
    // printf("Usage:\n");
    // printf("  memo <help|send>\n");
    // printf("  status <clients|server>\n");
}

const char *memo_params[] = {
    "help",
    "send",
};
const char *status_params[] = {
    "clients",
    "server",
};
// 命令和参数数量
#define CMD_COUNT (sizeof(commands) / sizeof(commands[0]))
#define MEMO_PARAM_COUNT (sizeof(memo_params) / sizeof(memo_params[0]))
#define STATUS_PARAM_COUNT (sizeof(status_params) / sizeof(status_params[0]))

// 查找命令
command_t *find_command(const char *name)
{
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        if (strcmp(commands[i].name, name) == 0)
        {
            return &commands[i];
        }
    }
    return NULL;
}

// 自动补全命令
char *command_generator(const char *text, int state)
{
    static uint32_t index, len;
    if (state == 0)
    {
        index = 0;
        len = strlen(text);
    }

    while (index < CMD_COUNT)
    {
        const char *cmd = commands[index++].name;
        if (strncmp(cmd, text, len) == 0)
        {
            return strdup(cmd);
        }
    }
    return NULL;
}

// 自动补全参数（根据不同的命令）
char *param_generator(const char *text, int state)
{
    static uint32_t index, len;
    static const char **param_list = NULL;
    static size_t param_count = 0;

    if (state == 0)
    { // 初始化
        index = 0;
        len = strlen(text);

        // 获取当前输入的命令
        char *buf = rl_line_buffer;
        if (strncmp(buf, "memo", 4) == 0)
        {
            param_list = memo_params;
            param_count = MEMO_PARAM_COUNT;
        }
        else if (strncmp(buf, "status", 6) == 0)
        {
            param_list = status_params;
            param_count = STATUS_PARAM_COUNT;
        }
        else
        {
            return NULL;
        }
    }

    while (index < param_count)
    {
        const char *param = param_list[index++];
        if (strncmp(param, text, len) == 0)
        {
            return strdup(param);
        }
    }

    return NULL;
}

// 补全入口函数
char **command_completion(const char *text, int start, int end)
{
    (void)end;

    // 处理命令补全
    if (start == 0)
    {
        return rl_completion_matches(text, command_generator);
    }

    // 处理不同命令的参数补全
    char *buf = rl_line_buffer;

    if (strncmp(buf, "memo", 4) == 0 && (buf[4] == ' ' || buf[4] == '\0'))
    {
        return rl_completion_matches(text, (rl_compentry_func_t *)param_generator);
    }
    else if (strncmp(buf, "status", 6) == 0 && (buf[6] == ' ' || buf[6] == '\0'))
    {
        return rl_completion_matches(text, (rl_compentry_func_t *)param_generator);
    }

    return NULL;
}

// 解析并执行命令
void execute_command(char *input)
{
    char *args[MAX_ARGS];
    int argc = 0;

    // 分割输入字符串
    char *token = strtok(input, " ");
    while (token != NULL && argc < MAX_ARGS)
    {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0)
    {
        return; // 空输入
    }

    // 查找命令并执行
    command_t *cmd = find_command(args[0]);
    if (cmd)
    {
        if (cmd->func != NULL)
            cmd->func(argc, args); // 执行命令处理函数
        else
            printf("Command not implemented: %s\n", cmd->name);
    }
    else
    {
        printf("Unknown command: %s\n", args[0]);
    }
}

#if 0
int main() {
    char *input;

    // 启用自动补全
    rl_attempted_completion_function = command_completion;

    while (1) {
        input = readline("cmd> ");
        if (!input) break;

        if (*input) {
            add_history(input);
            execute_command(input);
        }

        free(input);
    }

    return 0;
}
#endif
