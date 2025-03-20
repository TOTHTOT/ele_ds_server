/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-17 13:28:57
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-20 16:04:25
 * @FilePath: \ele_ds_server\common\common.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "common.h"
#include "log.h"
#include <curl/curl.h>

int32_t print_hex(const char *data, uint32_t size)
{
    if (data == NULL || size == 0)
    {
        return -EINVAL;
    }
    // 打印行头
    for (uint32_t i = 1; i < 0x10; i++)
        printf("%2x ", i);
    printf("\n");

    for (uint32_t i = 0; i < size; i++)
    {
        printf("%2x ", (unsigned char)data[i]);
        if ((i + 1) % 15 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");

    return 0;
}

/**
 * @description: 通过url获取数据, 返回json数据
 * @param {char} *url 获取数据的url
 * @param {uint32_t} urlsize url大小
 * @param {char} *data 存放数据的缓冲区
 * @param {uint32_t} datasize 暂时没用到
 * @param {curl_cb} write_callback 回调函数, 用于处理获取到的数据
 * @return {int32_t} 0: 成功, 其他: 失败
 */
int32_t get_data_byurl(char *url,
                       uint32_t urlsize,
                       char *data,
                       uint32_t datasize,
                       curl_cb write_callback)
{
    if (url == NULL || urlsize == 0 || data == NULL || datasize == 0)
    {
        ERROR_PRINT("Invalid argument: url is NULL or urlsize is 0 or data is NULL or datasize is 0");
        return -EINVAL;
    }

    CURL *curl;
    CURLcode res;

    // 初始化 libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        // 设置 URL
        curl_easy_setopt(curl, CURLOPT_URL, url);
        // 设置接收数据的回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        // 设置接收数据的缓冲区
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
        // 数据通过了gzip压缩, 得解压缩
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        // 执行 HTTP 请求
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            ERROR_PRINT("curl_easy_perform() failed: %s", curl_easy_strerror(res));
            return -EIO;
        }
        // 清理 libcurl
        curl_easy_cleanup(curl);
    }
    else
    {
        ERROR_PRINT("curl_easy_init() failed");
        return -EIO;
    }
    return 0;
}