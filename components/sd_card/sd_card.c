#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "dirent.h"
#include <errno.h>

#include "sd_card.h"

#define MAX_PATH_LENGTH 256

static const char *TAG = "sd_card";

// 打印文件树
void print_file_tree(const char *dir_path, int depth) 
{
    DIR *dir = opendir(dir_path);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory %s", dir_path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            // 如果是文件夹，递归打印文件夹内部的文件树
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
            {
                for (int i = 0; i < depth; i++) {
                    printf("  ");
                }
                printf("%s/\n", entry->d_name);
                char subdir_path[PATH_MAX];
                snprintf(subdir_path, sizeof(subdir_path), "%s/%s", dir_path, entry->d_name);
                print_file_tree(subdir_path, depth + 1);
            }
        } else {
            // 如果是文件，打印文件名
            for (int i = 0; i < depth; i++) {
                printf("  ");
            }
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
}

bool Check_if_the_file_exists(const char* filepath)
{
    FILE *file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return true; // 文件存在
    }
    return false; // 文件不存在
}

esp_err_t init_sd_card() 
{
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = 
    {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;

    ESP_LOGI(TAG, "Start Initializing SD card...");
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = 
    {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    
    if (spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ESP_FAIL;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;
    ESP_LOGI(TAG, "End of Initializing SD card");
    ESP_LOGI(TAG, "Start Mounting filesystem...");

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) 
    {
        if (ret == ESP_FAIL) 
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } 
        else 
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // SD卡初始化完成，打印信息
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

/// @brief sdcard_write_file
/// @param path 
/// @param data 
/// @return esp_err_t
esp_err_t sdcard_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

/// @brief sdcard_read_file
/// @param path 
/// @return esp_err_t
esp_err_t sdcard_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

/// @brief 删除路径及路径下所有文件
/// @param dir_path 
void delete_directory_recursive(const char* dir_path) 
{
    DIR* dir = opendir(dir_path);
    if (dir == NULL) 
    {
        ESP_LOGE(TAG, "Failed to open directory");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (entry->d_type == DT_DIR) 
        {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
            {
                char path[PATH_MAX];
                snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
                // 递归删除
                delete_directory_recursive(path);
            }
        } 
        else 
        {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
            //删除文件
            ESP_LOGW(TAG, "Delete File:%s", path);
            unlink(path);
        }
    }

    closedir(dir);
    // 删除文件路径本身
    rmdir(dir_path);
}

/// @brief 创建临时路径
/// @param path 
/// @param dir_path 
static void extract_dirname(const char *path, char *dir_path) 
{
    // Copy the path to a temporary buffer
    strncpy(dir_path, path, MAX_PATH_LENGTH);
    dir_path[MAX_PATH_LENGTH - 1] = '\0';  // Ensure null-termination

    char *last_slash = strrchr(dir_path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';  // Terminate the string at the last slash
    } else {
        // No directory component
        dir_path[0] = '.';  // Current directory
        dir_path[1] = '\0';
    }
}

/// @brief 递归创建文件路径
/// @param path 
/// @return 
static esp_err_t create_directory_recursive(const char *path) 
{
    // 创建临时路径以防修改原始路径
    char tmp[MAX_PATH_LENGTH] = {0};
    extract_dirname(path, tmp);
    
    // Check if the directory already exists
    struct stat st;
    if (stat(tmp, &st) == 0) 
    {
        // Path already exists
        return ESP_OK;
    }

    char current[MAX_PATH_LENGTH];
    char *p = NULL;
    size_t len;

    strncpy(current, tmp, sizeof(current));
    len = strlen(current);
    if (current[len - 1] == '/') {
        current[len - 1] = '\0';
    }
    for (p = current + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(current, 0777) != 0 && errno != EEXIST) {
                return ESP_FAIL;
            }
            *p = '/';
        }
    }
    if (mkdir(current, 0777) != 0 && errno != EEXIST) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

/// @brief 按照文件路径保存文件
/// @param path 文件路径（包含文件名和文件路径） 
/// @return 
esp_err_t sd_card_save_file(const char *path) 
{
    // 检查并创建目录
    esp_err_t err = create_directory_recursive(path);
    if (err != ESP_OK) {
        return err;
    }

    // 创建并关闭文件以确保路径存在
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        ESP_LOGE(TAG, "Fail to create File Path: %s", path);
        return ESP_FAIL;
    }

    fclose(file);
    return ESP_OK;
}

#ifdef __UNZIP_FUNCTION__
wchar_t *char2wchar(char *bytes) {
    if(!bytes)
        return NULL;
#if defined(_WIN32) || defined(_MSC_VER)
    size_t size = MultiByteToWideChar(CP_UTF8, 0, bytes, -1, NULL, 0);
    if(!size)
        return NULL;
    wchar_t *res_wstr = (wchar_t *)calloc(size + 1, sizeof(wchar_t));
    if(!res_wstr)
        return NULL;
    if(!MultiByteToWideChar(CP_UTF8, 0, bytes, -1, res_wstr, size)) {
        free(res_wstr); res_wstr = NULL;
        return NULL;
    }
    return res_wstr;
#else
    wchar_t *res_wstr = (wchar_t *)calloc(2048 + 1, sizeof(wchar_t));
    if(!res_wstr)
        return NULL;
    if(mbstowcs(res_wstr, bytes, 2048) == (size_t)-1) {
        free(res_wstr); res_wstr = NULL;
        return NULL;
    }
    return res_wstr;
#endif
}

char *wchar2char(wchar_t *bytes) {
    if(!bytes)
        return NULL;
#if defined(_WIN32) || defined(_MSC_VER)
    size_t size = WideCharToMultiByte(CP_UTF8, 0, bytes, -1, NULL, 0, NULL, NULL);
    if(!size)
        return NULL;
    char *res_str = (char *)calloc(size + 1, sizeof(char));
    if(!res_str)
        return NULL;
    if(!WideCharToMultiByte(CP_UTF8, 0, bytes, -1, res_str, size, NULL, NULL)) {
        free(res_str); res_str = NULL;
        return NULL;
    }
    return res_str;
#else
    char *res_str = (char *)calloc(2048 + 1, sizeof(char));
    if(!res_str)
        return NULL;
    if(wcstombs(res_str, bytes, 2048) == (size_t)-1) {
        free(res_str); res_str = NULL;
        return NULL;
    }
    return res_str;
#endif
}

int unzip_file(const char *zipfile) 
{
    unzFile zip = unzOpen(zipfile);
    if (zip == NULL) {
        printf("Error: Failed to open zip file.\n");
        return -1;
    }

    unz_global_info global_info;
    if (unzGetGlobalInfo(zip, &global_info) != UNZ_OK) {
        printf("Error: Failed to get global info of zip file.\n");
        unzClose(zip);
        return -1;
    }

    // 创建一个目录用于存放解压后的文件
    char extract_dir[256];
    snprintf(extract_dir, sizeof(extract_dir), "/blog/%s_extracted", "zip");
    mkdir(extract_dir, 0777);

    // 解压缩每个文件到目标目录
    unzGoToFirstFile(zip);
    do {
        wchar_t filename_inzip[64];
        unz_file_info file_info;
        unzGetCurrentFileInfo(zip, &file_info, (char*)filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

        char* filename_inzip_W = wchar2char(filename_inzip);

        char extract_path[512];
        snprintf(extract_path, sizeof(extract_path), "%s/%ls", extract_dir, filename_inzip);
        // if(IsTextUnicode(filename_inzip, sizeof(filename_inzip), NULL))
        // {
        //     printf("Unicode: %s\n", filename_inzip);
        // }
        // else
        // {
        //     printf("not Unicode: %s\n", filename_inzip);
        // }
        printf("Extracting %ls\n", filename_inzip);
        printf("Extracting %s\n", filename_inzip_W);

        if (unzOpenCurrentFile(zip) != UNZ_OK) {
            printf("Error: Failed to open current file in zip.\n");
            unzClose(zip);
            return -1;
        }

        FILE *outfile = fopen(extract_path, "ab");
        if (outfile == NULL) {
            printf("Error: Failed to open file for writing.\n");
            unzCloseCurrentFile(zip);
            unzClose(zip);
            return -1;
        }

        char buf[4096];
        int read_size;
        do {
            read_size = unzReadCurrentFile(zip, buf, sizeof(buf));
            if (read_size > 0)
                fwrite(buf, 1, read_size, outfile);
        } while (read_size > 0);

        fclose(outfile);
        unzCloseCurrentFile(zip);
    } while (unzGoToNextFile(zip) == UNZ_OK);

    unzClose(zip);
    printf("Extraction complete.\n");
    return 0;
}

#endif