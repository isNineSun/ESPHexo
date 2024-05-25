#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// 将十六进制字符转换为对应的数字
static int hex_to_int(char c) 
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1; // 非法的十六进制字符
}

// 解码 URL 编码的字符串
char* url_decode(const char* url) 
{
    size_t len = strlen(url);
    char* decoded_url = malloc(len + 1); // 分配足够的内存空间来存放解码后的字符串
    if (decoded_url == NULL) {
        return NULL; // 内存分配失败
    }

    size_t decoded_len = 0; // 解码后的字符串长度
    for (size_t i = 0; i < len; ++i) {
        if (url[i] == '%' && i + 2 < len) 
        {   // 如果遇到 '%' 符号
            // 将 '%' 后面的两个字符解码为一个字符
            int high = hex_to_int(url[i + 1]);
            int low = hex_to_int(url[i + 2]);
            if (high != -1 && low != -1) {
                decoded_url[decoded_len++] = (char)((high << 4) + low);
                i += 2; // 跳过两个十六进制字符
            } else {
                // 非法的十六进制字符
                decoded_url[decoded_len++] = url[i];
            }
        } else if (url[i] == '+') { // 如果遇到 '+' 符号，将其解码为空格
            decoded_url[decoded_len++] = ' ';
        } else {
            decoded_url[decoded_len++] = url[i];
        }
    }
    decoded_url[decoded_len] = '\0'; // 添加字符串结束符

    return decoded_url;
}

char* url_encode(const char* str) 
{
    // 计算最大可能长度：每个字符最多编码成 %XX（3字符），加上字符串终止符
    size_t len = strlen(str);
    char* buf = malloc(len * 3 + 1); 
    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    char* pbuf = buf;
    while (*str) {
        // 保留字母数字、-、_、.、~、/ 不编码
        if (isalnum((unsigned char)*str) || *str == '-' || *str == '_' || *str == '.' || 
            *str == '~' || *str == '/') {
            *pbuf++ = *str;
        } else {
            // 对其他字符进行编码
            pbuf += sprintf(pbuf, "%%%02X", (unsigned char)*str);
        }
        str++;
    }
    *pbuf = '\0';
    return buf;
}
