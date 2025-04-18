/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-17 13:29:03
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-01 17:28:29
 * @FilePath: \ele_ds_server\common.h
 * @Description: 通用库, 一些工具函数
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef size_t (*curl_cb)(void *ptr, size_t size, size_t nmemb, void *userdata); // libcurl 回调函数

extern int32_t print_hex(const char *data, uint32_t size);
extern int32_t get_data_byurl(char *url,
                              uint32_t urlsize,
                              char *data,
                              uint32_t datasize,
                              curl_cb write_callback);
extern void base64_encode(const unsigned char *input, size_t length, char *output);
extern void hex_to_bytes(const char *hex, unsigned char *bytes, size_t *length);

#endif /* __COMMON_H__ */
