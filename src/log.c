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


#include "log.h"
#include "string.h"
#include "time.h"
//#include "stdio.h"
#include "stdarg.h"

static char main_buf[LOG_MAIN_BUF_SIZE] = { 0 };
static char reserve_buf[LOG_RESERVE_BUF_SIZE] = { 0 }; //used while log isn't initialized, to store here log data
static uint32_t main_buf_pos = 0;
static uint32_t reserve_buf_pos = 0;
static bool print_in_progress_main = false;
static bool print_in_progress_reserve = false;
static uint32_t data_printed_per_session = 0;

static bool log_initialized = false;
struct tags_storage_t tags_storage;

const char *log_levels_str[] = {
        "[ ERROR ]",
        "[WARNING]",
        "[ INFO  ]",
        "[ DEBUG ]",
        "[VERBOSE]"
};

static int get_tag_index(const char *tag) {
    for (int i = 0; i < tags_storage.tags_num; i++) {
        if (strcmp(tag, tags_storage.tags_array[i].tag_name) == 0) {
            return i;
        }
    }
    return LOG_RETVAL_ERROR;
}

static bool set_tag_log_level(const char *tag, int8_t log_level) {
    if (strcmp(tag, "*") == 0) {
        for (int i = 0; i < tags_storage.tags_num; i++) {
            tags_storage.tags_array[i].log_level = log_level;
        }
        return true;
    }

    int index = get_tag_index(tag);
    if (index != LOG_RETVAL_ERROR) {
        tags_storage.tags_array[index].log_level = log_level;
        return true;
    }
    if (tags_storage.tags_num < MAX_AVAILABLE_TAGS_NUM) {
        tags_storage.tags_array[tags_storage.tags_num].log_level = log_level;
        tags_storage.tags_array[tags_storage.tags_num].tag_name = tag;
        tags_storage.tags_num++;
        return true;
    }
    return false;
}

static bool is_tag_printable(const char *tag, int8_t printable_level) {
    int index = get_tag_index(tag);
    if (index == LOG_RETVAL_ERROR) {
        if (set_tag_log_level(tag, LOG_LEVEL_DEFAULT) == true) {
            if (LOG_LEVEL_DEFAULT >= printable_level) {
                return true;
            }
        }
        return false;
    }
    if (tags_storage.tags_array[index].log_level < printable_level) {
        return false;
    }
    return true;
}

static int log_form_prefix(char *arr, int len, int8_t log_level, const char *tag) {
    int written_len = 0;
#if LOG_PRINT_TIMESTAMP
    if(len - written_len > LOG_DATE_PREFIX_MAX_LEN){
        time_t timestamp = log_get_timestamp();
#if	LOG_FORMAT_TIMESTAMP
        struct tm local_time;
        localtime_r(&timestamp, &local_time);
        written_len += snprintf(&arr[written_len], len - written_len, "[%04d-%02d-%02d %02d:%02d:%02d]", local_time.tm_year + LOG_TIME_H_YEAR_START,
                local_time.tm_mon, local_time.tm_mday, local_time.tm_hour, local_time.tm_min, local_time.tm_sec);
#else
        written_len += snprintf(&arr[written_len], len - written_len, "[%9lu]", (uint32_t)timestamp);
#endif
    }
#endif

#if LOG_PRINT_LOG_LEVEL
    if (len - written_len > (int) strlen(log_levels_str[log_level])) {
        written_len += snprintf(&arr[written_len], len - written_len, "%s",
                log_levels_str[log_level]);
    }
#endif

#if LOG_PRINT_TAGNAMES
    if(tag != NULL){
        if(len - written_len > (int)strlen(tag)){
            written_len += snprintf(&arr[written_len], len - written_len, "[%s]", tag);
        }
    }
#endif

#if LOG_PRINT_ACTIVE_TASK
    if(get_active_task_name() != NULL){
        if(len - written_len > (int)strlen(get_active_task_name())){
                written_len += snprintf(&arr[written_len], len - written_len, "[%s]", get_active_task_name());
        }
    }
#endif

    if (len - written_len > 1) {
        written_len += snprintf(&arr[written_len], len - written_len, " ");
    }
    return written_len;
}

int log_printf(char *prefix, const char *fmt, va_list args) {
    int printed_len = 0;
    if (print_in_progress_main == true) {
        /* Recursive call, print to reserve buffer */
        if (print_in_progress_reserve != true) {
            print_in_progress_reserve = true;
            printed_len = snprintf(&reserve_buf[reserve_buf_pos],
                    LOG_RESERVE_BUF_SIZE - reserve_buf_pos, "%s", prefix);
            reserve_buf_pos += printed_len;
            printed_len = vsnprintf(&reserve_buf[reserve_buf_pos],
                    LOG_RESERVE_BUF_SIZE - reserve_buf_pos, fmt, args);
            reserve_buf_pos += printed_len;
            print_in_progress_reserve = false;
            return printed_len;
        }
        return 0;
    }
    print_in_progress_main = true;

    printed_len = snprintf(&main_buf[main_buf_pos],
            LOG_MAIN_BUF_SIZE - main_buf_pos, "%s", prefix);
    main_buf_pos += printed_len;
    printed_len = vsnprintf(&main_buf[main_buf_pos],
            LOG_MAIN_BUF_SIZE - main_buf_pos, fmt, args);
    main_buf_pos += printed_len;

    if (log_initialized == true) {
        print_in_progress_reserve = true;
        log_print_arr(reserve_buf, reserve_buf_pos);
        data_printed_per_session += reserve_buf_pos;
        reserve_buf_pos = 0;
        print_in_progress_reserve = false;

        log_print_arr(main_buf, main_buf_pos);
        data_printed_per_session += main_buf_pos;
        main_buf_pos = 0;
    }
    print_in_progress_main = false;
    return printed_len;
}

void log_write(int8_t log_level, const char *tag, const char *format, ...) {
    va_list list;
    va_start(list, format);

    if (tag != NULL) {
        if (is_tag_printable(tag, log_level) == false) {
            return;
        }
    }

#if LOG_USE_RTOS
    /* Call platform implemented lock function */
    log_lock();
#endif

    char prefix_buf[LOG_ALL_PREFIX_MAX_LEN] = { 0 };
    log_form_prefix(prefix_buf, LOG_ALL_PREFIX_MAX_LEN, log_level, tag);
    log_printf(prefix_buf, format, list);

#if LOG_USE_RTOS
    /* Call platform implemented unlock function */
    log_unlock();
#endif

    va_end(list);
}

void log_flush_bufs() {
#if LOG_USE_RTOS
    /* Call platform implemented lock function */
    log_lock();
#endif

    if (log_initialized == true) {
        print_in_progress_reserve = true;
        log_print_arr(reserve_buf, reserve_buf_pos);
        data_printed_per_session += reserve_buf_pos;
        reserve_buf_pos = 0;
        print_in_progress_reserve = false;

        log_print_arr(main_buf, main_buf_pos);
        data_printed_per_session += main_buf_pos;
        main_buf_pos = 0;
    }
    print_in_progress_main = false;

#if LOG_USE_RTOS
    /* Call platform implemented unlock function */
    log_unlock();
#endif
}

void log_init() {
    log_platform_init();
    log_initialized = true;
    log_flush_bufs();
}
