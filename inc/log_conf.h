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


#ifndef SRC_LOG_LOG_CONF_H_
#define SRC_LOG_LOG_CONF_H_

#include "log.h"

#ifdef LOG_USE_CUSTOM_CONF
#include "log_conf_custom.h"
#else

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
#define LOG_MAIN_BUF_SIZE                           2000

/* Is used in case of recursive log print call,
 * actual when LOG_USE_RTOS is set to 0
 */
#define LOG_RESERVE_BUF_SIZE                        1000

#define LOG_USE_RTOS                                1
#if LOG_USE_RTOS > 0
#define LOG_PRINT_ACTIVE_TASK                       1
#endif

/* Parameters below must not be changed */
#define LOG_DATE_PREFIX_MAX_LEN                     24
#define LOG_ALL_PREFIX_MAX_LEN                      82

#endif /* LOG_USE_CUSTOM_CONF */
#endif /* SRC_LOG_LOG_CONF_H_ */
