/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-17 13:28:57
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-09 14:57:09
 * @FilePath: \ele_ds_server\common\common.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "common.h"
#include "log.h"
#include <curl/curl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
        LOG_E("Invalid argument: url is NULL or urlsize is 0 or data is NULL or datasize is 0");
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
            LOG_E("curl_easy_perform() failed: %s", curl_easy_strerror(res));
            return -EIO;
        }
        // 清理 libcurl
        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_E("curl_easy_init() failed");
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

static unsigned char base64_dec_table[256] = {0x80};
__attribute__((constructor)) // 构造函数, 在main函数之前执行
void base64_init(void)
{
    base64_dec_table['A'] = 0; base64_dec_table['B'] = 1; base64_dec_table['C'] = 2;
    base64_dec_table['D'] = 3; base64_dec_table['E'] = 4; base64_dec_table['F'] = 5;
    base64_dec_table['G'] = 6; base64_dec_table['H'] = 7; base64_dec_table['I'] = 8;
    base64_dec_table['J'] = 9; base64_dec_table['K'] = 10; base64_dec_table['L'] = 11;
    base64_dec_table['M'] = 12; base64_dec_table['N'] = 13; base64_dec_table['O'] = 14;
    base64_dec_table['P'] = 15; base64_dec_table['Q'] = 16; base64_dec_table['R'] = 17;
    base64_dec_table['S'] = 18; base64_dec_table['T'] = 19; base64_dec_table['U'] = 20;
    base64_dec_table['V'] = 21; base64_dec_table['W'] = 22; base64_dec_table['X'] = 23;
    base64_dec_table['Y'] = 24; base64_dec_table['Z'] = 25;
    base64_dec_table['a'] = 26; base64_dec_table['b'] = 27; base64_dec_table['c'] = 28;
    base64_dec_table['d'] = 29; base64_dec_table['e'] = 30; base64_dec_table['f'] = 31;
    base64_dec_table['g'] = 32; base64_dec_table['h'] = 33; base64_dec_table['i'] = 34;
    base64_dec_table['j'] = 35; base64_dec_table['k'] = 36; base64_dec_table['l'] = 37;
    base64_dec_table['m'] = 38; base64_dec_table['n'] = 39; base64_dec_table['o'] = 40;
    base64_dec_table['p'] = 41; base64_dec_table['q'] = 42; base64_dec_table['r'] = 43;
    base64_dec_table['s'] = 44; base64_dec_table['t'] = 45; base64_dec_table['u'] = 46;
    base64_dec_table['v'] = 47; base64_dec_table['w'] = 48; base64_dec_table['x'] = 49;
    base64_dec_table['y'] = 50; base64_dec_table['z'] = 51;
    base64_dec_table['0'] = 52; base64_dec_table['1'] = 53; base64_dec_table['2'] = 54;
    base64_dec_table['3'] = 55; base64_dec_table['4'] = 56; base64_dec_table['5'] = 57;
    base64_dec_table['6'] = 58; base64_dec_table['7'] = 59; base64_dec_table['8'] = 60;
    base64_dec_table['9'] = 61; base64_dec_table['+'] = 62; base64_dec_table['/'] = 63;
    base64_dec_table['='] = 0;
}
/**
 * @description: Base64 解码
 * @param {char} *input 输入数据
 * @param {size_t} length 输入数据长度
 * @param {unsigned char} *output 输出数据缓冲区, 要比input小33%
 * @param {size_t} *output_length 输出数据长度
 * @return {int32_t} 0: 成功, -EINVAL: 输入参数无效
 */
int32_t base64_decode(const char *input, size_t length, unsigned char *output, size_t *output_length)
{
    if (!input || !output || !output_length)
        return -EINVAL;

    size_t i = 0, j = 0;
    unsigned char quad[4];
    size_t quad_index = 0;

    while (i < length)
    {
        unsigned char c = input[i++];

        if (c == '\0') break;  // 防止奇怪的输入

        if (base64_dec_table[c] & 0x80)
            continue; // 跳过非法字符，比如换行、空格

        quad[quad_index++] = base64_dec_table[c];

        if (quad_index == 4)
        {
            output[j++] = (quad[0] << 2) | (quad[1] >> 4);
            if (input[i - 2] != '=')
                output[j++] = (quad[1] << 4) | (quad[2] >> 2);
            if (input[i - 1] != '=')
                output[j++] = (quad[2] << 6) | quad[3];
            quad_index = 0;
        }
    }

    *output_length = j;
    return 0;
}

#if (USE_SELF_CRC == 1)
/**
 * @description: CRC32校验
 * @param {char} *data 数据
 * @param {size_t} len 数据长度
 * @return {uint32_t} CRC32校验值
 */
uint32_t crc32(const char *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; ++i)
    {
        crc ^= (uint8_t)data[i];
        for (int j = 0; j < 8; ++j)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc = crc >> 1;
        }
    }
    return ~crc;
}


#define CRC_BUFFER_SIZE 512  // 每次读取文件的块大小
static unsigned crc32_table[256] = {
    0u, 1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
249268274u, 2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
498536548u, 1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
325883990u, 1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
997073096u, 1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
901097722u, 1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
651767980u, 1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
671266974u, 1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
};

static uint32_t soft_crc32(const uint8_t *buf, size_t len, uint32_t crc_init)
{
    uint32_t crc = crc_init ^ 0xFFFFFFFF;
    while (len--)
        crc = crc32_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFF;
}


/**
 * @description: 计算文件的CRC32值
 * @param {int} argc 参数数量, 程序内调用传入2
 * @param {char} * argv[] 参数数组, 程序内调用传入文件路径 argv[0]为程序名, argv[1]为文件路径
 * @return {uint32_t} CRC32值
 */
uint32_t crcfile(int argc, char **argv)
{
    if (argc != 2)
    {
        LOG_E("Usage: crcfile <filename>\n");
        return 0;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        LOG_E("Failed to open file: %s\n", filename);
        return 0;
    }

    uint8_t buffer[CRC_BUFFER_SIZE] = {0};
    ssize_t read_bytes;
    uint32_t crc_result = 0;

    while ((read_bytes = read(fd, buffer, CRC_BUFFER_SIZE)) > 0)
    {
        crc_result = soft_crc32(buffer, read_bytes, crc_result);
    }
    close(fd);
    return crc_result;
}
#endif
