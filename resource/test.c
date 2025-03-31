#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdint.h>
// 结构体定义
struct weather_info {
    char fxDate[11];
    char sunrise[6];
    char sunset[6];
    char moonrise[6];
    char moonset[6];
    char moonPhase[16];
    char moonPhaseIcon[4];
    int8_t tempMax;
    int8_t tempMin;
    char iconDay[4];
    char textDay[16];
    char iconNight[4];
    char textNight[16];
    uint16_t wind360Day;
    char windDirDay[16];
    char windScaleDay[8];
    uint8_t windSpeedDay;
    uint16_t wind360Night;
    char windDirNight[16];
    char windScaleNight[8];
    uint8_t windSpeedNight;
    uint8_t humidity;
    float precip;
    uint16_t pressure;
    uint8_t vis;
    uint8_t cloud;
    uint8_t uvIndex;
};

// 创建 JSON
char *create_weather_json(struct weather_info *data, int data_len, const char *msg) {
    cJSON *root = cJSON_CreateObject();
    cJSON *data_array = cJSON_CreateArray();

    // 结构体数组转换成 JSON
    for (int i = 0; i < data_len; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "fxDate", data[i].fxDate);
        cJSON_AddStringToObject(item, "sunrise", data[i].sunrise);
        cJSON_AddStringToObject(item, "sunset", data[i].sunset);
        cJSON_AddStringToObject(item, "moonrise", data[i].moonrise);
        cJSON_AddStringToObject(item, "moonset", data[i].moonset);
        cJSON_AddStringToObject(item, "moonPhase", data[i].moonPhase);
        cJSON_AddStringToObject(item, "moonPhaseIcon", data[i].moonPhaseIcon);
        cJSON_AddNumberToObject(item, "tempMax", data[i].tempMax);
        cJSON_AddNumberToObject(item, "tempMin", data[i].tempMin);
        cJSON_AddStringToObject(item, "iconDay", data[i].iconDay);
        cJSON_AddStringToObject(item, "textDay", data[i].textDay);
        cJSON_AddStringToObject(item, "iconNight", data[i].iconNight);
        cJSON_AddStringToObject(item, "textNight", data[i].textNight);
        cJSON_AddNumberToObject(item, "wind360Day", data[i].wind360Day);
        cJSON_AddStringToObject(item, "windDirDay", data[i].windDirDay);
        cJSON_AddStringToObject(item, "windScaleDay", data[i].windScaleDay);
        cJSON_AddNumberToObject(item, "windSpeedDay", data[i].windSpeedDay);
        cJSON_AddNumberToObject(item, "wind360Night", data[i].wind360Night);
        cJSON_AddStringToObject(item, "windDirNight", data[i].windDirNight);
        cJSON_AddStringToObject(item, "windScaleNight", data[i].windScaleNight);
        cJSON_AddNumberToObject(item, "windSpeedNight", data[i].windSpeedNight);
        cJSON_AddNumberToObject(item, "humidity", data[i].humidity);
        cJSON_AddNumberToObject(item, "precip", data[i].precip);
        cJSON_AddNumberToObject(item, "pressure", data[i].pressure);
        cJSON_AddNumberToObject(item, "vis", data[i].vis);
        cJSON_AddNumberToObject(item, "cloud", data[i].cloud);
        cJSON_AddNumberToObject(item, "uvIndex", data[i].uvIndex);

        cJSON_AddItemToArray(data_array, item);
    }

    cJSON_AddNumberToObject(root, "msgtype", 1);
    cJSON_AddNumberToObject(root, "len", data_len);
    cJSON_AddItemToObject(root, "data", data_array);
    cJSON_AddStringToObject(root, "message", msg);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;
}

// 测试代码
int main() {
    struct weather_info weather_data[7] = {
        {"2025-03-20", "06:19", "18:26", "", "08:53", "亏凸月", "805", 24, 10, "100", "晴", "150", "晴", 315, "西北风", "1-3", 3, 270, "西风", "1-3", 3, 21, 0.0, 1009, 25, 0, 5},
        {"2025-03-21", "06:18", "18:27", "", "08:50", "满月", "806", 25, 11, "101", "多云", "151", "多云", 320, "西风", "1-3", 2, 275, "西风", "1-3", 2, 22, 0.1, 1010, 24, 1, 6},
        {"2025-03-22", "06:17", "18:28", "", "08:48", "亏凸月", "807", 23, 9, "102", "阴", "152", "阴", 310, "东北风", "2-4", 4, 260, "东风", "2-4", 4, 20, 0.2, 1012, 23, 5, 4},
        {"2025-03-23", "06:16", "18:29", "", "08:46", "亏凸月", "808", 26, 12, "103", "小雨", "153", "小雨", 305, "南风", "3-5", 5, 255, "南风", "3-5", 5, 23, 0.5, 1013, 22, 8, 7},
        {"2025-03-24", "06:15", "18:30", "", "08:44", "新月", "809", 27, 13, "104", "晴", "154", "晴", 300, "西风", "1-3", 3, 250, "西风", "1-3", 3, 25, 0.0, 1011, 26, 0, 8},
        {"2025-03-25", "06:14", "18:31", "", "08:42", "上弦月", "810", 28, 14, "105", "多云", "155", "多云", 295, "西南风", "2-4", 4, 245, "西南风", "2-4", 4, 26, 0.1, 1010, 24, 2, 9},
        {"2025-03-26", "06:13", "18:32", "", "08:40", "盈凸月", "811", 29, 15, "106", "阴", "156", "阴", 290, "东北风", "3-5", 5, 240, "东北风", "3-5", 5, 27, 0.3, 1012, 22, 4, 10}
    };

    char *json_output = create_weather_json(weather_data, 7, "hello");
    printf("JSON Output:\n%s\n", json_output);

    free(json_output);
    return 0;
}
