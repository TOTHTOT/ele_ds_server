/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-17 13:28:57
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-27 10:23:40
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

// Base64 编码表
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// 将 16 进制字符串转换为字节数组
void hex_to_bytes(const char *hex, unsigned char *bytes, size_t *length)
{
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0)
    {
        fprintf(stderr, "Invalid hex string length\n");
        return;
    }
    *length = hex_len / 2;
    for (size_t i = 0; i < *length; ++i)
    {
        sscanf(hex + 2 * i, "%2hhx", &bytes[i]);
    }
}

/**
 * @description: Base64 编码
 * @param {unsigned char} *input 输入数据
 * @param {size_t} length 输入数据长度
 * @param {char} *output 输出数据缓冲区, 要比input大33%
 * @param {size_t} output_size 输出数据缓冲区大小
 * @return {int32_t} 0: 成功, -ENOMEM: 输出缓冲区不够大
 */
int32_t base64_encode(const unsigned char *input, size_t length, char *output, size_t output_size)
{
    size_t i = 0, j = 0;
    unsigned char triple[3];
    unsigned char quartet[4];

    // 如果output不够大直接返回
    if (length == 0 || output_size < ((length + 2) / 3) * 4 + 1)
    {
        return -ENOMEM;
    }
    while (length--)
    {
        triple[i++] = *(input++);
        if (i == 3)
        {
            quartet[0] = (triple[0] >> 2) & 0x3F;
            quartet[1] = ((triple[0] << 4) | (triple[1] >> 4)) & 0x3F;
            quartet[2] = ((triple[1] << 2) | (triple[2] >> 6)) & 0x3F;
            quartet[3] = triple[2] & 0x3F;

            for (i = 0; i < 4; ++i)
                output[j++] = base64_table[quartet[i]];

            i = 0;
        }
    }

    if (i)
    {
        for (size_t k = i; k < 3; ++k)
            triple[k] = '\0';

        quartet[0] = (triple[0] >> 2) & 0x3F;
        quartet[1] = ((triple[0] << 4) | (triple[1] >> 4)) & 0x3F;
        quartet[2] = ((triple[1] << 2) | (triple[2] >> 6)) & 0x3F;
        quartet[3] = triple[2] & 0x3F;

        for (size_t k = 0; k < i + 1; ++k)
            output[j++] = base64_table[quartet[k]];

        while (i++ < 3)
            output[j++] = '=';
    }
    output[j] = '\0';

    return 0;
}
