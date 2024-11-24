/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Vasilenko Aleksei <awarder64@gmail.com>
 */


#ifndef SRC_LOG_LOG_H_
#define SRC_LOG_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#include "log_conf.h"
#include "printf.h"

#define LOG_RETVAL_ERROR                (-1)

/* Log levels */
#define LOG_LEVEL_NO_LOGS               (-1)
#define LOG_LEVEL_ERROR                 0
#define LOG_LEVEL_WARNING               1
#define LOG_LEVEL_INFO                  2
#define LOG_LEVEL_DEBUG                 3
#define LOG_LEVEL_VERBOSE               4

#if (LOG_LEVEL_GLOBAL < LOG_LEVEL_ERROR)
#define LOGE(tag, ...)                  do{}while(0)
#else
#define LOGE(tag, ...)                  log_write(LOG_LEVEL_ERROR, tag, __VA_ARGS__)
#endif

#if (LOG_LEVEL_GLOBAL < LOG_LEVEL_WARNING)
#define LOGW(tag, ...)                  do{}while(0)
#else
#define LOGW(tag, ...)                  log_write(LOG_LEVEL_WARNING, tag, __VA_ARGS__)
#endif

#if (LOG_LEVEL_GLOBAL < LOG_LEVEL_INFO)
#define LOGI(tag, ...)                  do{}while(0)
#else
#define LOGI(tag, ...)                  log_write(LOG_LEVEL_INFO, tag, __VA_ARGS__)
#endif

#if (LOG_LEVEL_GLOBAL < LOG_LEVEL_DEBUG)
#define LOGD(tag, ...)                  do{}while(0)
#else
#define LOGD(tag, ...)                  log_write(LOG_LEVEL_DEBUG, tag, __VA_ARGS__)
#endif

#if (LOG_LEVEL_GLOBAL < LOG_LEVEL_VERBOSE)
#define LOGV(tag, ...)                  do{}while(0)
#else
#define LOGV(tag, ...)                  log_write(LOG_LEVEL_VERBOSE, tag, __VA_ARGS__)
#endif

struct tag_info_t {
    const char *tag_name;
    int8_t log_level;
};

struct tags_storage_t {
    struct tag_info_t tags_array[MAX_AVAILABLE_TAGS_NUM];
    int tags_num;
};

void log_write(int8_t log_level, const char *tag, const char *format, ...);
void log_init();

void log_print_arr(char* arr, uint32_t len);
uint32_t log_get_timestamp();
void log_unlock();
void log_lock();
const char* get_active_task_name();
void log_platform_init();

#ifdef __cplusplus
}
#endif

#endif /* SRC_LOG_LOG_H_ */
