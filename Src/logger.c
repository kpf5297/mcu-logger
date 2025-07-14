/**
 * @file logger.c
 * @brief Implementation of the modular logging system for STM32-based MCU projects.
 */

#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#if LOG_USE_SD
#include "ff.h"  // FatFS
#endif

#ifdef __has_include
#  if __has_include("logger_config.h")
#    include "logger_config.h"
#  endif
#endif

// === Configuration Defaults ===
#ifndef LOG_USE_UART
#define LOG_USE_UART 1
#endif

#ifndef LOG_USE_SD
#define LOG_USE_SD 0
#endif

#ifndef LOG_USE_DMA
#define LOG_USE_DMA 0
#endif

#ifndef LOG_USE_IT
#define LOG_USE_IT 1
#endif

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 256
#endif

#ifndef LOG_RING_BUFFER_SIZE
#define LOG_RING_BUFFER_SIZE 1024
#endif

static LogLevel current_level = LOG_LEVEL_INFO;
static uint8_t logging_enabled = 1;

static char ring_buffer[LOG_RING_BUFFER_SIZE];
static volatile uint16_t head = 0;
static volatile uint16_t tail = 0;
static volatile bool tx_busy = false;

#ifndef LOG_ENTER_CRITICAL
#define LOG_ENTER_CRITICAL() __disable_irq()
#endif
#ifndef LOG_EXIT_CRITICAL
#define LOG_EXIT_CRITICAL() __enable_irq()
#endif

#ifndef LOG_UART_HANDLE
#define LOG_UART_HANDLE huart1  // Default
#endif

extern UART_HandleTypeDef LOG_UART_HANDLE;


#if LOG_USE_SD
static FIL log_file;
static uint8_t sd_initialized = 0;
#endif

/**
 * @brief Writes a string into the ring buffer for non-blocking UART output.
 * @param data Null-terminated string to enqueue.
 */ 
static void ring_buffer_write(const char* data) {
    while (*data) {
        LOG_ENTER_CRITICAL();
        uint16_t next = (head + 1) % LOG_RING_BUFFER_SIZE;
        if (next == tail) {
            LOG_EXIT_CRITICAL();
            break; // Buffer full
        }
        ring_buffer[head] = *data++;
        head = next;
        LOG_EXIT_CRITICAL();
    }
}

/**
 * @brief Starts or continues sending data from the ring buffer via UART.
 */
static void ring_buffer_send_next(void) {
    uint16_t len = 0;
    LOG_ENTER_CRITICAL();
    if (tx_busy || tail == head) {
        LOG_EXIT_CRITICAL();
        return; // Nothing to send or already transmitting
    }
#if LOG_USE_DMA
    len = (head >= tail) ? (head - tail) : (LOG_RING_BUFFER_SIZE - tail);
#endif
    tx_busy = true;
    LOG_EXIT_CRITICAL();

#if LOG_USE_DMA
    HAL_UART_Transmit_DMA(&LOG_UART_HANDLE, (uint8_t*)&ring_buffer[tail], len);
#elif LOG_USE_IT
    HAL_UART_Transmit_IT(&LOG_UART_HANDLE, (uint8_t*)&ring_buffer[tail], 1);
#endif
}

/**
 * @brief UART transmission complete callback handler.
 *        Should be called from HAL_UART_TxCpltCallback().
 * @param huart Pointer to the HAL UART handle.
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == LOG_UART_HANDLE.Instance) {
#if LOG_USE_DMA
        LOG_ENTER_CRITICAL();
        tail = (tail + huart->TxXferSize) % LOG_RING_BUFFER_SIZE;
        LOG_EXIT_CRITICAL();
#elif LOG_USE_IT
        LOG_ENTER_CRITICAL();
        tail = (tail + 1) % LOG_RING_BUFFER_SIZE;
        LOG_EXIT_CRITICAL();
#endif
        LOG_ENTER_CRITICAL();
        tx_busy = false;
        LOG_EXIT_CRITICAL();
        ring_buffer_send_next();
    }
}

/**
 * @brief Initializes the logging system.
 *
 * - Resets the ring buffer
 * - Enables logging
 * - Opens SD file if SD logging is enabled
 */
void Log_Init(void) {
    head = tail = 0;
    logging_enabled = 1;
#if LOG_USE_SD
    if (f_open(&log_file, "log.txt", FA_OPEN_ALWAYS | FA_WRITE) == FR_OK) {
        f_lseek(&log_file, f_size(&log_file));
        sd_initialized = 1;
    }
#endif
}

/**
 * @brief Sets the active logging verbosity level.
 * @param level Desired log level (e.g., LOG_LEVEL_DEBUG).
 */
void Log_SetLevel(LogLevel level) {
    current_level = level;
}

/**
 * @brief Disables all logging at runtime.
 */
void Log_Disable(void) {
    logging_enabled = 0;
}

/**
 * @brief Logs a formatted message based on severity level.
 *
 * - Message is dropped if below current log level or if logging is disabled.
 * - Output is routed to UART and/or SD depending on configuration.
 *
 * @param level Severity level (LOG_LEVEL_ERROR, etc.)
 * @param format `printf`-style format string.
 * @param ... Arguments to format.
 */
void Log(LogLevel level, const char* format, ...) {
    if (!logging_enabled || level > current_level) return;

    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

#if LOG_USE_UART
    Log_Write_UART(buffer);
#endif
#if LOG_USE_SD
    Log_Write_SD(buffer);
#endif
}

/**
 * @brief Forces a flush of buffered data to the SD card.
 */
void Log_Flush(void) {
#if LOG_USE_SD
    if (sd_initialized) {
        f_sync(&log_file);
    }
#endif
}

/**
 * @brief Default UART log output (weak).
 *        Can be overridden for custom UART routing or formatting.
 * @param msg Null-terminated string to send.
 */
__attribute__((weak)) void Log_Write_UART(const char* msg) {
#if LOG_USE_DMA || LOG_USE_IT
    ring_buffer_write(msg);
    ring_buffer_send_next();
#else
    HAL_UART_Transmit(&LOG_UART_HANDLE, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
#endif
}

/**
 * @brief Default SD card log output (weak).
 *        Can be overridden for filters, timestamps, or buffering.
 * @param msg Null-terminated string to append to log file.
 */
__attribute__((weak)) void Log_Write_SD(const char* msg) {
#if LOG_USE_SD
    if (sd_initialized) {
        UINT bw;
        f_write(&log_file, msg, strlen(msg), &bw);
    }
#endif
}
