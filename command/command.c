#include "command.h"
#include "../main.h"
#include "../log.h"
#include "../server/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 10 // 最大参数数量

// 预定义命令列表
const char *commands[] = {
    "help",
    "exit",
    "status",
    "memo",
    "users",
};

// 预定义参数
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
        const char *cmd = commands[index++];
        if (strncmp(cmd, text, len) == 0)
        {
            return strdup(cmd);
        }
    }
    return NULL;
}

// 自动补全参数（根据不同的命令）
char *param_generator(const char *text, int state) {
    static uint32_t index, len;
    static const char **param_list = NULL;
    static size_t param_count = 0;

    if (state == 0) {  // 初始化
        index = 0;
        len = strlen(text);
        
        // 获取当前输入的命令
        char *buf = rl_line_buffer;
        if (strncmp(buf, "memo", 4) == 0) {
            param_list = memo_params;
            param_count = MEMO_PARAM_COUNT;
        } else if (strncmp(buf, "status", 6) == 0) {
            param_list = status_params;
            param_count = STATUS_PARAM_COUNT;
        } else {
            return NULL;
        }
    }

    while (index < param_count) {
        const char *param = param_list[index++];
        if (strncmp(param, text, len) == 0) {
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

// 解析命令
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

    // 解析命令
    if (strcmp(args[0], "exit") == 0)
    {
        printf("Exiting...\n");
        exit(0);
    }
    else if (strcmp(args[0], "help") == 0)
    {
        printf("Available commands: help, exit, status, memo, users\n");
        printf("Usage:\n");
        printf("  memo <help|send>\n");
        printf("  status <clients|server>\n");
    }
    else if (strcmp(args[0], "status") == 0)
    {
        if (argc < 2)
        {
            printf("Usage: status <clients|server>\n");
        }
        else if (strcmp(args[1], "clients") == 0)
        {
            if (ele_ds_server.server.ops.connected_client != NULL)
                ele_ds_server.server.ops.connected_client(&ele_ds_server.server);
            else
                ERROR_PRINT("No connected clients.\n");
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
    else if (strcmp(args[0], "memo") == 0)
    {
        if (argc < 2)
        {
            printf("Usage: memo <help|send>\n");
        }
        else if (strcmp(args[1], "help") == 0)
        {
            printf("Usage:\n");
            printf("  memo send <fd> <msg>\n");
        }
        else if (strcmp(args[1], "send") == 0)
        {
            printf("Sending memo...\n");
        }
        else
        {
            printf("Unknown memo parameter: %s\n", args[1]);
        }
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
