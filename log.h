#ifndef __LOG_H__
#define __LOG_H__

#define DEBUG_ON 1
#define INFO_OUTPUT 3
#define WARNING_OUTPUT 2
#define DEBUG_OUTPUT 1
#define ERROR_OUTPUT 0
#define DEBUG_LEVEL INFO_OUTPUT

#ifdef DEBUG_ON
#define INFO_PRINT(fmt, ...)                                                                   \
    do                                                                                         \
    {                                                                                          \
        if (DEBUG_LEVEL >= INFO_OUTPUT)                                                        \
        {                                                                                      \
            printf("Info %s,%s,%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        }                                                                                      \
    } while (0)

#define WARNING_PRINT(fmt, ...)                                                                   \
    do                                                                                            \
    {                                                                                             \
        if (DEBUG_LEVEL >= WARNING_OUTPUT)                                                        \
        {                                                                                         \
            printf("Warning %s,%s,%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        }                                                                                         \
    } while (0)

#define DEBUG_PRINT(fmt, ...)                                                                   \
    do                                                                                          \
    {                                                                                           \
        if (DEBUG_LEVEL >= DEBUG_OUTPUT)                                                        \
        {                                                                                       \
            printf("Debug %s,%s,%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        }                                                                                       \
    } while (0)

#define ERROR_PRINT(fmt, ...)                                                                   \
    do                                                                                          \
    {                                                                                           \
        if (DEBUG_LEVEL >= ERROR_OUTPUT)                                                        \
        {                                                                                       \
            printf("Error %s,%s,%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        }                                                                                       \
    } while (0)

#else

#define INFO_PRINT(fmt, ...) \
    do                       \
    {                        \
    } while (0)

#define WARNING_PRINT(fmt, ...) \
    do                          \
    {                           \
    } while (0)

#define DEBUG_PRINT(fmt, ...) \
    do                        \
    {                         \
    } while (0)

#define ERROR_PRINT(fmt, ...) \
    do                        \
    {                         \
    } while (0)

#endif

#endif /* __LOG_H__ */
