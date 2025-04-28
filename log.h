#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdarg.h>

// --- 日志等级 ---
#define LOG_LVL_NONE     0
#define LOG_LVL_ERROR    1
#define LOG_LVL_WARNING  2
#define LOG_LVL_INFO     3
#define LOG_LVL_DEBUG    4

// --- 默认模块名 & 日志等级 ---
#ifndef LOG_TAG
#define LOG_TAG "unknown"
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LVL_INFO
#endif

// --- 基础内部打印函数 ---
static inline void log_output(const char *level, const char *file, const char *func, int line, const char *fmt, ...)
{
    va_list args;
    (void)file; // 暂时不使用
    printf("[%s][%s](%s:%d) ", level, LOG_TAG, func, line);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

// --- 日志输出宏 ---
#if (LOG_LEVEL >= LOG_LVL_DEBUG)
#define LOG_D(fmt, ...) log_output("D", __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_D(fmt, ...) do {} while(0)
#endif

#if (LOG_LEVEL >= LOG_LVL_INFO)
#define LOG_I(fmt, ...) log_output("I", __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...) do {} while(0)
#endif

#if (LOG_LEVEL >= LOG_LVL_WARNING)
#define LOG_W(fmt, ...) log_output("W", __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_W(fmt, ...) do {} while(0)
#endif

#if (LOG_LEVEL >= LOG_LVL_ERROR)
#define LOG_E(fmt, ...) log_output("E", __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_E(fmt, ...) do {} while(0)
#endif

#endif /* __LOG_H__ */
