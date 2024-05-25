#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"  
#include "esp_log.h" 

#include "json_parse.h"
#include "cJSON.h"

static const char *TAG = "json_parse";

Configuration global_config_json;

int get_global_config_from_json(char *filename) 
{   
    // 从 JSON 文件中读取数据并填充结构体
    read_json_file(filename, &global_config_json);

    ESP_LOGI(TAG, "ssid:%s, pwd:%s, domain:%s, token:%s, web_name:%s, web_pwd:%s, repo_owner:%s, repo_name:%s\n", 
                            global_config_json.wifi_ssid, global_config_json.wifi_password,
                            global_config_json.ddns_domain, global_config_json.ddns_token,
                            global_config_json.web_username, global_config_json.web_password,
                            global_config_json.repo_owner, global_config_json.repo_name);

    return 1;
}

// 读取 JSON 文件信息
void read_json_file(const char *filename, Configuration* config) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Error opening file %s\n", filename);
        return;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // 读取文件内容到缓冲区
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Memory allocation failed\n");
        fclose(file);
        return;
    }
    fread(buffer, 1, file_size, file);
    fclose(file);
    buffer[file_size] = '\0';

    // 解析 JSON
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        ESP_LOGE(TAG, "JSON parsing failed\n");
        free(buffer);
        return;
    }

    // 将 JSON 中的数据存储到结构体中
    strcpy(config->wifi_ssid, cJSON_GetObjectItemCaseSensitive(json, "wifi_ssid")->valuestring);
    strcpy(config->wifi_password, cJSON_GetObjectItemCaseSensitive(json, "wifi_password")->valuestring);
    strcpy(config->ddns_domain, cJSON_GetObjectItemCaseSensitive(json, "ddns_domain")->valuestring);
    strcpy(config->ddns_token, cJSON_GetObjectItemCaseSensitive(json, "ddns_token")->valuestring);
    strcpy(config->web_username, cJSON_GetObjectItemCaseSensitive(json, "web_username")->valuestring);
    strcpy(config->web_password, cJSON_GetObjectItemCaseSensitive(json, "web_password")->valuestring);
    strcpy(config->repo_type, cJSON_GetObjectItemCaseSensitive(json, "repo_type")->valuestring);
    strcpy(config->repo_owner, cJSON_GetObjectItemCaseSensitive(json, "repo_owner")->valuestring);
    strcpy(config->repo_name, cJSON_GetObjectItemCaseSensitive(json, "repo_name")->valuestring);

    // 释放内存并返回配置
    cJSON_Delete(json);
    free(buffer);
}

// 修改 JSON 文件
void modify_json_file(const char *filename, const Configuration *config) 
{
    // 创建 JSON 对象
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "wifi_ssid", config->wifi_ssid);
    cJSON_AddStringToObject(json, "wifi_password", config->wifi_password);
    cJSON_AddStringToObject(json, "ddns_domain", config->ddns_domain);
    cJSON_AddStringToObject(json, "ddns_token", config->ddns_token);
    cJSON_AddStringToObject(json, "web_username", config->web_username);
    cJSON_AddStringToObject(json, "web_password", config->web_password);
    cJSON_AddStringToObject(json, "repo_type", config->repo_type);
    cJSON_AddStringToObject(json, "repo_owner", config->repo_owner);
    cJSON_AddStringToObject(json, "repo_name", config->repo_name);

    // 将 JSON 对象转换为字符串
    char *json_str = cJSON_Print(json);

    // 将字符串写入文件
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        cJSON_Delete(json);
        free(json_str);
        return;
    }
    fprintf(file, "%s", json_str);

    // 释放内存
    fclose(file);
    cJSON_Delete(json);
    free(json_str);
}

// 修改 JSON 文件中指定属性的值
void modify_json_attribute(const char *filename, const char *attribute_name, const char *new_value) 
{
    // 读取 JSON 文件
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // 读取文件内容到缓冲区
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        fclose(file);
        return;
    }
    fread(buffer, 1, file_size, file);
    fclose(file);
    buffer[file_size] = '\0';

    // 解析 JSON
    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    if (json == NULL) {
        printf("JSON parsing failed\n");
        return;
    }

    // 修改指定属性的值
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, attribute_name);
    if (item) {
        cJSON_SetValuestring(item, new_value);
    } else {
        printf("Attribute %s not found\n", attribute_name);
    }

    // 将 JSON 对象转换为字符串
    char *json_str = cJSON_Print(json);

    // 将字符串写入文件
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        cJSON_Delete(json);
        free(json_str);
        return;
    }
    fprintf(file, "%s", json_str);

    // 释放内存
    fclose(file);
    cJSON_Delete(json);
    free(json_str);
}

// 读取 github 仓库文件目录 JSON 文件信息
char **read_github_json_info_file(const char *filename, int *num_blobs) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Error opening file %s\n", filename);
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // 读取文件内容到缓冲区
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, file_size, file);
    fclose(file);
    buffer[file_size] = '\0';

    //ESP_LOGI(TAG, "File content: %s", buffer);

    // 解析 JSON
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        ESP_LOGE(TAG, "JSON parsing failed\n");
        free(buffer);
        return NULL;
    }

    cJSON *tree = cJSON_GetObjectItemCaseSensitive(json, "tree");
    if (!cJSON_IsArray(tree)) {
        fprintf(stderr, "Error: `tree` is not an array.\n");
        cJSON_Delete(json);
        return NULL;
    }

    int blob_count = 0;
    const cJSON *item = NULL;
    cJSON_ArrayForEach(item, tree) {
        cJSON *type = cJSON_GetObjectItemCaseSensitive(item, "type");
        if (cJSON_IsString(type) && (strcmp(type->valuestring, "blob") == 0)) {
            blob_count++;
        }
    }

    char **blob_paths = malloc(blob_count * sizeof(char *));
    if (!blob_paths) {
        fprintf(stderr, "Error allocating memory for blob paths.\n");
        cJSON_Delete(json);
        return NULL;
    }

    int index = 0;
    cJSON_ArrayForEach(item, tree) {
        cJSON *type = cJSON_GetObjectItemCaseSensitive(item, "type");
        if (cJSON_IsString(type) && (strcmp(type->valuestring, "blob") == 0)) {
            cJSON *path = cJSON_GetObjectItemCaseSensitive(item, "path");
            if (cJSON_IsString(path) && (path->valuestring != NULL)) {
                blob_paths[index] = strdup(path->valuestring);
                if (!blob_paths[index]) {
                    fprintf(stderr, "Error allocating memory for path string.\n");
                    // Free previously allocated strings
                    for (int i = 0; i < index; i++) {
                        free(blob_paths[i]);
                    }
                    free(blob_paths);
                    cJSON_Delete(json);
                    return NULL;
                }
                index++;
            }
        }
    }

    cJSON_Delete(json);

    *num_blobs = blob_count;
    return blob_paths;
}