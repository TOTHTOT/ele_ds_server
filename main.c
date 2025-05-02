/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-03 09:35:51
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-02 10:36:15
 * @FilePath: \ele_ds_server\main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "main.h"
#include "common.h"
#include "weather.h"
#include "./client/client.h"
#include "./server/server.h"
#include "./command/command.h"
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>

#define LOG_TAG "main"
#define LOG_LEVEL LOG_LVL_DEBUG
#include "log.h"
/* 全局变量 */
ele_ds_server_t ele_ds_server = {0};

/* 函数声明 */
static void *server_thread(void *arg);

/**
 * @description: 测试相关函数
 * @return {*}
 */
void test_func()
{
    int32_t ret = 0;
#if 0
    send_data_pack_t send_data;                      // = {0}, 这样赋值会报错初始化语法有问题 [-Wmissing-braces]
    memset(&send_data, 0, sizeof(send_data_pack_t)); // 初始化结构体
    get_weather(send_data.weather, WEATHER_DAY_MAX, time(NULL), 101010100);
#endif
#ifdef OPEN_TEST_DECODE
    char inptut[] = "LyoNCiAqIEBBdXRob3I6IFRPVEhUT1QgMzc1ODU4ODMrVE9USFRPVEB1c2Vycy5ub3JlcGx5LmdpdGh1Yi5jb20NCiAqIEBEYXRlOiAyMDI1LTAzLTAzIDA5OjM1OjUxDQogKiBATGFzdEVkaXRvcnM6IFRPVEhUT1QgMzc1ODU4ODMrVE9USFRPVEB1c2Vycy5ub3JlcGx5LmdpdGh1Yi5jb20NCiAqIEBMYXN0RWRpdFRpbWU6IDIwMjUtMDQtMjcgMTA6Mzc6NDcNCiAqIEBGaWxlUGF0aDogXGVsZV9kc19zZXJ2ZXJcbWFpbi5jDQogKiBARGVzY3JpcHRpb246IOi/meaYr+m7mOiupOiuvue9rizor7forr7nva5gY3VzdG9tTWFkZWAsIOaJk+W8gGtvcm9GaWxlSGVhZGVy5p+l55yL6YWN572uIOi/m+ihjOiuvue9rjogaHR0cHM6Ly9naXRodWIuY29tL09CS29ybzEva29ybzFGaWxlSGVhZGVyL3dpa2kvJUU5JTg1JThEJUU3JUJEJUFFDQogKi8NCiNpbmNsdWRlICJtYWluLmgiDQojaW5jbHVkZSAibG9nLmgiDQojaW5jbHVkZSAiY29tbW9uLmgiDQojaW5jbHVkZSAid2VhdGhlci5oIg0KI2luY2x1ZGUgIi4vY2xpZW50L2NsaWVudC5oIg0KI2luY2x1ZGUgIi4vc2VydmVyL3NlcnZlci5oIg0KI2luY2x1ZGUgIi4vY29tbWFuZC9jb21tYW5kLmgiDQojaW5jbHVkZSA8ZXJybm8uaD4NCiNpbmNsdWRlIDxzaWduYWwuaD4NCiNpbmNsdWRlIDx1bmlzdGQuaD4NCiNpbmNsdWRlIDxleGVjaW5mby5oPg0KDQovKiDlhajlsYDlj5jph48gKi8NCmVsZV9kc19zZXJ2ZXJfdCBlbGVfZHNfc2VydmVyID0gezB9Ow0KDQovKiDlh73mlbDlo7DmmI4gKi8NCnN0YXRpYyB2b2lkICpzZXJ2ZXJfdGhyZWFkKHZvaWQgKmFyZyk7DQoNCi8qKg0KICogQGRlc2NyaXB0aW9uOiDmtYvor5Xnm7jlhbPlh73mlbANCiAqIEByZXR1cm4geyp9DQogKi8NCnZvaWQgdGVzdF9mdW5jKCkNCnsNCiAgICBpbnQzMl90IHJldCA9IDA7DQojaWYgMA0KICAgIHNlbmRfZGF0YV9wYWNrX3Qgc2VuZF9kYXRhOyAgICAgICAgICAgICAgICAgICAgICAvLyA9IHswfSwg6L+Z5qC36LWL5YC85Lya5oql6ZSZ5Yid5aeL5YyW6K+t5rOV5pyJ6Zeu6aKYIFstV21pc3NpbmctYnJhY2VzXQ0KICAgIG1lbXNldCgmc2VuZF9kYXRhLCAwLCBzaXplb2Yoc2VuZF9kYXRhX3BhY2tfdCkpOyAvLyDliJ3lp4vljJbnu5PmnoTkvZMNCiAgICBnZXRfd2VhdGhlcihzZW5kX2RhdGEud2VhdGhlciwgV0VBVEhFUl9EQVlfTUFYLCB0aW1lKE5VTEwpLCAxMDEwMTAxMDApOw0KI2VuZGlmDQogICAgcmV0ID0gc2VydmVyX2luaXQoJmVsZV9kc19zZXJ2ZXIuc2VydmVyLCBTRVJWRVJfUE9SVCwgY2xpZW50X2V2ZW50X2hhbmRsZXIpOw0KICAgIGlmIChyZXQgIT0gMCkNCiAgICB7DQogICAgICAgIEVSUk9SX1BSSU5UKCJTZXJ2ZXIgaW5pdGlhbGl6YXRpb24gZmFpbGVkXG4iKTsNCiAgICAgICAgcmV0dXJuOw0KICAgIH0NCiAgICBlbHNlDQogICAgew0KICAgICAgICBwdGhyZWFkX2NyZWF0ZSgmZWxlX2RzX3NlcnZlci5zZXJ2ZXJfdGhyZWFkLCBOVUxMLCBzZXJ2ZXJfdGhyZWFkLCAmZWxlX2RzX3NlcnZlci5zZXJ2ZXIpOw0KICAgICAgICBJTkZPX1BSSU5UKCJTZXJ2ZXIgaW5pdGlhbGl6ZWRcbiIpOw0KICAgIH0NCn0NCg0Kc3RhdGljIHZvaWQgKnNlcnZlcl90aHJlYWQodm9pZCAqYXJnKQ0Kew0KICAgIHNlcnZlcl90ICpzZXJ2ZXIgPSAoc2VydmVyX3QgKilhcmc7DQogICAgSU5GT19QUklOVCgiU2VydmVyIHRocmVhZCBzdGFydGVkXG4iKTsNCiAgICB3aGlsZSAoZWxlX2RzX3NlcnZlci5leGl0ZmxhZyA9PSBmYWxzZSkNCiAgICB7DQogICAgICAgIHNlcnZlcl9oYW5kbGVfY2xpZW50cyhzZXJ2ZXIpOw0KICAgICAgICB1c2xlZXAoMTAwICogMTAwMCk7DQogICAgfQ0KICAgIElORk9fUFJJTlQoIlNlcnZlciB0aHJlYWQgZXhpdGVkXG4iKTsNCiAgICByZXR1cm4gTlVMTDsNCn0NCg0KDQp2b2lkIHNpZ25hbF9oYW5kbGVyKGludCBzaWdubykNCnsNCiAgICBzd2l0Y2ggKHNpZ25vKQ0KICAgIHsNCiAgICBjYXNlIFNJR0lOVDoNCiAgICAgICAgSU5GT19QUklOVCgiU2lnbmFsICVkIHJlY2VpdmVkLCBleGl0aW5nLi4uXG4iLCBzaWdubyk7DQogICAgICAgIGVsZV9kc18=";
    uint8_t output[sizeof(inptut) * 2] = {0};
    size_t output_len = 0;
    base64_decode(inptut, strlen(inptut), output, &output_len);
    LOG_I("base64_decode: %s\n", output);
#endif /* OPEN_TEST_DECODE */

    ret = server_init(&ele_ds_server.server, SERVER_PORT, client_event_handler);
    if (ret != 0)
    {
        LOG_E("Server initialization failed\n");
        return;
    }
    else
    {
        pthread_create(&ele_ds_server.server_thread, NULL, server_thread, &ele_ds_server.server);
        LOG_I("Server initialized\n");
    }
}

static void *server_thread(void *arg)
{
    server_t *server = (server_t *)arg;
    LOG_I("Server thread started\n");
    while (ele_ds_server.exitflag == false)
    {
        server_handle_clients(server);
        usleep(100 * 1000);
    }
    LOG_I("Server thread exited\n");
    return NULL;
}


void signal_handler(int signo)
{
    switch (signo)
    {
    case SIGINT:
        LOG_I("Signal %d received, exiting...\n", signo);
        ele_ds_server.exitflag = true;
        break;
    case SIGSEGV:
    {
        void *array[10];
        size_t size;

        size = backtrace(array, 10);
        fprintf(stderr, "Error: signal %d:\n", signo);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        ele_ds_server.exitflag = true;
        exit(1);
        break;
    }
    default:
        break;
    }
}

/**
 * @description: 初始化设备
 * @param {ele_ds_server_t} *device 设备指针
 * @param {uint16_t} port 端口号
 * @return {*}
 */
static int32_t ele_ds_server_init(ele_ds_server_t *device, uint16_t port)
{
    int32_t ret = 0;
    if (device == NULL)
    {
        LOG_E("Invalid argument: device is NULL");
        return -1;
    }
    
    device->port = port;
    device->exitflag = false;
    ret = server_init(&ele_ds_server.server, port, client_event_handler);
    if (ret != 0)
    {
        LOG_E("Server initialization failed\n");
        return -2;
    }

    ret = users_init(&device->server.users_db, "users.db");
    if (ret != 0)
    {
        LOG_E("users_init failed\n");
        return -1;
    }

    pthread_create(&ele_ds_server.server_thread, NULL, server_thread, &ele_ds_server.server);
    LOG_I("Server initialized\n");

    return 0;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    // 注册信号
    signal(SIGINT, signal_handler);
    signal(SIGSEGV, signal_handler);
#ifdef OPENTEST
    test_func();
#else
    ele_ds_server_init(&ele_ds_server, SERVER_PORT);
    rl_attempted_completion_function = command_completion;
#endif /* OPENTEST */
    while (ele_ds_server.exitflag == false)
    {
        char *input = readline("> ");  // 显示 `>` 提示符
        if (input && *input) {
            add_history(input);  // 添加到命令历史
            execute_command(input);
        }
        free(input);  // readline 需要手动释放内存
    }
    server_close(&ele_ds_server.server);
    pthread_join(ele_ds_server.server_thread, NULL);
    LOG_I("Exiting...\n");
    return 0;
}