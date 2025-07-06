# Logger System Usage Guide

This document explains how to use the modular logging system (`logger.h` / `logger.c`) designed for STM32 MCU projects with UART and SD card output.

---

## Features

* UART output with blocking, interrupt (IT), or DMA modes
* Optional SD card logging (FatFS required)
* Ring buffer for non-blocking UART output
* Runtime log level filtering (`ERROR`, `WARN`, `INFO`, `DEBUG`)
* Logging disable and flush support
* Easy to override hooks for custom output targets

---

## Files

* `logger.h` – Public interface and configuration
* `logger.c` – Implementation with ring buffer, UART/SD output, and FatFS integration

---

## Configuration

Set these macros **before** including `logger.h` or define them in a `logger_config.h` (optional).

```c
#define LOG_USE_UART 1       // Enable UART output (default 1)
#define LOG_USE_SD   1       // Enable SD card logging (default 0)
#define LOG_USE_IT   1       // Use UART interrupt (default 1)
#define LOG_USE_DMA  0       // Use UART DMA instead of IT (default 0)
#define LOG_BUFFER_SIZE 256  // printf-style buffer size
#define LOG_RING_BUFFER_SIZE 1024  // UART TX buffer size
```

> Only one of `LOG_USE_DMA` or `LOG_USE_IT` should be enabled.

---

## Initialization and Usage

### Initialization:

```c
#include "logger.h"

Log_Init();                // Initialize ring buffer and SD card file
Log_SetLevel(LOG_LEVEL_DEBUG);  // Set desired verbosity level
```

### Logging:

```c
Log(LOG_LEVEL_INFO, "Startup complete\n");
Log(LOG_LEVEL_DEBUG, "Sensor: %f\n", temperature);
```

### Disabling/Flushing:

```c
Log_Disable();             // Disable logging (run-time)
Log_Flush();               // Sync SD card log file
```

---

## UART Integration

You must define a UART handle externally:

```c
extern UART_HandleTypeDef huart1;
```

And call the UART complete callback:

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    // Call into logger
    Logger_HAL_UART_TxCpltCallback(huart); // or place `ring_buffer_send_next()` inline
}
```

---

## SD Card Integration

Requires:

* FatFS (`ff.h`) and disk I/O working
* Mount must be done before `Log_Init()`
* Writes to `log.txt` in append mode

Auto flushes on `Log_Flush()` using `f_sync()`:

```c
f_open(&log_file, "log.txt", FA_OPEN_ALWAYS | FA_WRITE);
f_lseek(&log_file, f_size(&log_file));
f_write(&log_file, msg, strlen(msg), &bw);
f_sync(&log_file);
```

---

## Custom Output Hooks

You can override either of these functions to customize output:

```c
__attribute__((weak)) void Log_Write_UART(const char* msg);
__attribute__((weak)) void Log_Write_SD(const char* msg);
```

For example, redirect output to USB or a BLE service.

---

## Example Session

```c
Log_Init();
Log_SetLevel(LOG_LEVEL_INFO);

Log(LOG_LEVEL_INFO, "System booting\n");
Log(LOG_LEVEL_DEBUG, "Debug info will not show at this level\n");

Log_SetLevel(LOG_LEVEL_DEBUG);
Log(LOG_LEVEL_DEBUG, "Now debug logs will show\n");

Log_Disable();
Log(LOG_LEVEL_ERROR, "This will not be printed\n");

Log_Flush(); // Save data to SD
```

---

## Tips

* You can queue logs during runtime with `LOG_USE_IT` or `LOG_USE_DMA`
* For CLI coexistence, use a shared ring buffer or alternate UARTs
* For safety, call `Log_Flush()` before shutting down or resetting the MCU

---
