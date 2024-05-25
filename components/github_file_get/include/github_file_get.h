#ifndef GITHUB_FILE_GET_H
#define GITHUB_FILE_GET_H

#include "esp_log.h"
#include "json_parse.h"
#include "sd_card.h"
#include "url_decode.h"
#ifdef USE_OLED
#include "oled.h"
#endif 

typedef enum
{
    NORMAL = 0,
    GET_INDEX,
    GET_ZIP
}github_file_client_req_type_e;

typedef struct
{
    char FilePath[1024];
    github_file_client_req_type_e req_type;
}github_file_client;

extern int blob_count;
extern int num_blobs;

extern void download_github_file_zip(void);
extern void get_github_file_json_index(void);

#endif