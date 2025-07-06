/**
 * @file logger.h
 * @brief Modular and flexible logging system for STM32-based MCU projects.
 *
 * ## Features:
 * - Output to UART, SD card, or both (selectable via `#define LOG_USE_*` macros).
 * - Supports blocking and non-blocking UART (DMA or IT).
 * - Circular ring buffer for non-blocking operation.
 * - Verbosity control with log levels: ERROR, WARN, INFO, DEBUG.
 * - Runtime enable/disable of logging via `Log_Disable()`.
 * - Flush function for SD log file.
 * - User-overridable hooks for output backends.
 *
 * ## Configuration Macros:
 * - `LOG_USE_UART`: Enable/disable UART output (default: 1)
 * - `LOG_USE_SD`: Enable/disable SD card output (default: 0)
 * - `LOG_USE_IT`: Enable interrupt-based UART TX (default: 1)
 * - `LOG_USE_DMA`: Enable DMA-based UART TX (default: 0)
 * - `LOG_BUFFER_SIZE`: Buffer size for log formatting
 * - `LOG_RING_BUFFER_SIZE`: Ring buffer size for non-blocking TX
 *
 * ## Example:
 * @code
 * Log_Init();
 * Log_SetLevel(LOG_LEVEL_DEBUG);
 * Log(LOG_LEVEL_INFO, "System initialized.\n");
 * Log(LOG_LEVEL_DEBUG, "ADC value: %d\n", adc_value);
 * Log_Disable();  // Temporarily stop logging
 * Log(LOG_LEVEL_ERROR, "This won't be printed.\n");
 * Log_Flush();    // Sync SD card if enabled
 * @endcode
 */

#pragma once

#include <stdint.h>
#include <stdarg.h>

#ifdef __has_include
#  if __has_include("logger_config.h")
#    include "logger_config.h"
#  endif
#endif

/**
 * @enum LogLevel
 * @brief Defines severity levels for logging.
 */
typedef enum {
    LOG_LEVEL_ERROR = 0, /**< Critical error */
    LOG_LEVEL_WARN,      /**< Warning but not fatal */
    LOG_LEVEL_INFO,      /**< Informational message */
    LOG_LEVEL_DEBUG      /**< Debug-level message */
} LogLevel;

/**
 * @brief Initialize the logging system.
 *
 * Sets up buffers, SD card (if enabled), and clears state.
 */
void Log_Init(void);

/**
 * @brief Set the active logging level.
 *
 * Messages below this level will be ignored.
 * @param level One of the values from LogLevel.
 */
void Log_SetLevel(LogLevel level);

/**
 * @brief Log a formatted message if the level passes the threshold.
 *
 * @param level The severity of the message.
 * @param format printf-style format string.
 * @param ... Arguments to format.
 */
void Log(LogLevel level, const char* format, ...);

/**
 * @brief Flush output buffers.
 *
 * For SD logging, this calls `f_sync()` to commit to disk.
 */
void Log_Flush(void);

/**
 * @brief Disable all logging at runtime.
 *
 * Logging can be re-enabled by calling Log_SetLevel().
 */
void Log_Disable(void);

/**
 * @brief UART output hook. Can be overridden by user.
 * @param msg Null-terminated string to send via UART.
 */
__attribute__((weak)) void Log_Write_UART(const char* msg);

/**
 * @brief SD card output hook. Can be overridden by user.
 * @param msg Null-terminated string to write to file.
 */
__attribute__((weak)) void Log_Write_SD(const char* msg);