/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-03 09:35:51
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-17 14:18:32
 * @FilePath: \ele_ds_server\main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "main.h"
#include "log.h"
#include "common.h"
#include "weather.h"
#include <errno.h>

int main(int argc, char *argv[])
{
    send_data_pack_t send_data; // = {0}, 这样赋值会报错初始化语法有问题 [-Wmissing-braces]
    memset(&send_data, 0, sizeof(send_data_pack_t)); // 初始化结构体

    get_weather(send_data.weather, WEATHER_DAY_MAX, time(NULL), 101010100);

    return 0;
}