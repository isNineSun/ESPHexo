#ifndef JSON_PARSE_H
#define JSON_PARSE_H

// 保存 JSON 数据的结构体
typedef struct {
    char wifi_ssid[32];
    char wifi_password[64]; 
    char ddns_domain[32];
    char ddns_token[128];
    char web_username[32];
    char web_password[64];
    char blog_path[128];
    char repo_type[32];
    char repo_owner[128];
    char repo_name[128];
} Configuration;

extern Configuration global_config_json;

extern int get_global_config_from_json(char *filename);

extern void read_json_file(const char *filename, Configuration* config);

extern void modify_json_file(const char *filename, const Configuration *config);

extern void modify_json_attribute(const char *filename, const char *attribute_name, const char *new_value);

extern char **read_github_json_info_file(const char *filename, int *num_blobs);
#endif