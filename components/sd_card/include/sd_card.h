#ifndef SDCARD_H
#define SDCARD_H

#define MOUNT_POINT "/blog"

#define PIN_NUM_MISO  39
#define PIN_NUM_MOSI  41
#define PIN_NUM_CLK   40
#define PIN_NUM_CS    42

#define EXAMPLE_MAX_CHAR_SIZE    64

extern esp_err_t init_sd_card();
extern esp_err_t sdcard_read_file(const char *path);
extern esp_err_t sdcard_write_file(const char *path, char *data);
extern void print_file_tree(const char *dir_path, int depth);
extern bool Check_if_the_file_exists(const char* filepath);
extern void delete_directory_recursive(const char* dir_path);

extern esp_err_t sd_card_save_file(const char *path);

#ifdef __UNZIP_FUNCTION__
extern esp_err_t sdcard_unzip_file(const char *zip_filename);
extern int unzip_file(const char *zipfile);
#endif

#endif //SDCARD_H