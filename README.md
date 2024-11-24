# Logger library
Universal logger library for embedded systems

## Author
Vasilenko Alexey awarder64@gmail.com

## Getting started
To use the library you should define several functions in your code:
```c++
void log_platform_init();
void log_print_arr(char *arr, uint32_t len);
uint32_t log_get_timestamp();
```
and in case of using RTOS you should also define:
```c++
const char* get_active_task_name();
void log_lock();
void log_unlock();
```
Default logger configuration is in **log_conf.h**. If you want to use your custom configuration you should create file **log_conf_custom.h** and build your application with define **LOG_USE_CUSTOM_CONF**.


To set global log level use define **LOG_LEVEL_GLOBAL** in config file. You can also set different formatting options of logs (see all parameters in **log_conf.h**).

**LOG_LEVEL_GLOBAL** can have the values: **LOG_LEVEL_NO_LOGS**, **LOG_LEVEL_ERROR**, **LOG_LEVEL_WARNING**, **LOG_LEVEL_INFO**, **LOG_LEVEL_DEBUG** or **LOG_LEVEL_VERBOSE**.

### Included Libraries

This project reuses the `printf` implementation from [mpaland/printf](https://github.com/mpaland/printf), licensed under the MIT License.  
It is optimized for embedded systems with limited resources and provides a lightweight alternative to standard `printf` implementations.

#### Customizing the `printf` Library

If you need to customize the behavior of the `printf` library, you can include a custom configuration header file by defining the macro `PRINTF_INCLUDE_CONFIG_H` globally during compilation.

```bash
gcc -DPRINTF_INCLUDE_CONFIG_H ...
```

## Usage example
```cpp
#include "log.h"

static const char *tag = "MAIN";

void func() {
    LOGE(tag, "Test string error\n");
    LOGW(tag, "Test string warning\n");
    LOGI(tag, "Test string info\n");
    LOGD(tag, "Test string debug\n");
    LOGV(tag, "Test string verbose\n");
}
```
---
*****log_conf_custom.h*** example:**
```c
#ifndef SRC_LOG_CUSTOM_CONF_H_
#define SRC_LOG_CUSTOM_CONF_H_


/* Max printable log level, all logs with higher
 * log level will not be printed
 */
#define LOG_LEVEL_GLOBAL                            LOG_LEVEL_VERBOSE

/* Log level that assigns to all tags that haven't
 * explicitly assigned to any level
 */
#define LOG_LEVEL_DEFAULT                           LOG_LEVEL_VERBOSE

#define LOG_TIME_H_YEAR_START                       1900
#define MAX_AVAILABLE_TAGS_NUM                      16
#define LOG_PRINT_TAGNAMES                          0
#define LOG_PRINT_LOG_LEVEL                         1
#define LOG_PRINT_TIMESTAMP                         0

#if LOG_PRINT_TIMESTAMP > 0
/* LOG_FORMAT_TIMESTAMP 1 means timestamp in human
 * readable format [%04d-%02d-%02d %02d:%02d:%02d]
 */
#define LOG_FORMAT_TIMESTAMP                        1
#endif

/* Main buffer size, used also to store data when
 * log is not initialized yet
 */
#define LOG_MAIN_BUF_SIZE                           1000

/* Is used in case of recursive log print call,
 * actual when LOG_USE_RTOS is set to 0
 */
#define LOG_RESERVE_BUF_SIZE                        1000

#define LOG_USE_RTOS                                1
#if LOG_USE_RTOS > 0
#define LOG_PRINT_ACTIVE_TASK                       0
#endif

/* Parameters below must not be changed */
#define LOG_DATE_PREFIX_MAX_LEN                     24
#define LOG_ALL_PREFIX_MAX_LEN                      82

#endif /* SRC_LOG_CUSTOM_CONF_H_ */
```
---
Custom example of definition necessary functions to use the logger

***log_platform_specific.c***

```c
#include "usart_io.h"
#include "log.h"
#if LOG_USE_RTOS == 1
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LOG_UART_INCOME_BUFFER_LEN     64

static SemaphoreHandle_t               log_mutex = NULL;
static StaticSemaphore_t               log_mutex_buffer;
static uint8_t                         rx_buf[LOG_UART_INCOME_BUFFER_LEN];

const char* get_active_task_name() {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return NULL;
    }
    return pcTaskGetName(xTaskGetCurrentTaskHandle());
}

void log_lock() {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return;
    }
    if (log_mutex == NULL) {
        log_mutex = xSemaphoreCreateMutexStatic(&log_mutex_buffer);
    }
    xSemaphoreTake(log_mutex, portMAX_DELAY);
}

void log_unlock() {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return;
    }
    xSemaphoreGive(log_mutex);
}
#endif

void log_data_income_cb(uint32_t data_size) {
//    printf("%lu\n", data_size);
    (void)data_size;
}

void log_platform_init() {
    /* Place here any actions that are needed for logging, maybe open file or init some peripheral */
    uart_init(UART_1, UART_8N1, UART_BAUDRATE_115200, false,
            rx_buf, LOG_UART_INCOME_BUFFER_LEN, NULL);
}

void log_print_arr(char *arr, uint32_t len) {
    uart_write(UART_1, (uint8_t*)arr, (int)len);
}

uint32_t log_get_timestamp() {
    return HAL_GetTick() / 1000;
}

void _putchar(char character) {
    uart_write(UART_1, (uint8_t*)&character, 1);
}

```
---
*****printf_config.h*** example:**
```c
#ifndef LOG_LIB_INC_PRINTF_CONFIG_H_
#define LOG_LIB_INC_PRINTF_CONFIG_H_


// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#define PRINTF_NTOA_BUFFER_SIZE    32U

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#define PRINTF_FTOA_BUFFER_SIZE    32U

// support for the floating point type (%f)
// default: activated
#define PRINTF_DISABLE_SUPPORT_FLOAT

// support for exponential floating point notation (%e/%g)
// default: activated
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL

// define the default floating point precision
// default: 6 digits
#define PRINTF_DEFAULT_FLOAT_PRECISION  6U

// define the largest float suitable to print with %f
// default: 1e9
#define PRINTF_MAX_FLOAT  1e9

// support for the long long types (%llu or %p)
// default: activated
#define PRINTF_DISABLE_SUPPORT_LONG_LONG

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated
#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T

#endif /* LOG_LIB_INC_PRINTF_CONFIG_H_ */

```

## License

This project is licensed under the Apache License, Version 2.0.  
You may obtain a copy of the License at [LICENSE](./LICENSE).

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,  
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
See the License for the specific language governing permissions and limitations under the License.
