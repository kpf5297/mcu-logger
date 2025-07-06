# MCU Logger

A flexible and modular logging system for STM32-based microcontroller projects. Supports UART and SD card output, blocking or non-blocking transmission, verbosity levels, and easy integration with command line interfaces or control systems.

## Features

- Modular C-based design
- UART + SD logging support
- Non-blocking or blocking transmission
- DMA and Interrupt-based modes supported
- Circular buffer system for non-blocking mode
- Configurable verbosity levels (DEBUG, INFO, WARN, ERROR)
- Optional `log_flush()` and `log_disable()`
- Integration-friendly with other UART users (e.g., CLI or telemetry)
- Lightweight footprint for resource-constrained devices

## Doxygen Documentation

[![Doxygen](https://img.shields.io/badge/docs-Doxygen-blue.svg)](https://kpf5297.github.io/mcu-logger/)

## Integration

Include the following in your STM32 project:

```c
#include "logger.h"
````

In your initialization:

```c
Logger_Init(LOG_OUTPUT_UART | LOG_OUTPUT_SD, LOG_MODE_DMA);
Logger_SetVerbosity(LOG_LEVEL_INFO);
```

In usage:

```c
LOG_INFO("System initialized");
LOG_DEBUG("Value: %d", some_var);
LOG_WARN("Sensor timeout");
LOG_ERROR("System failure!");
```

## Configuration

Create a `logger_config.h` file in your application to override defaults.

Example:

```c
// logger_config.h
#pragma once

// user configuration overrides
#define LOG_UART_HANDLE     huart2
#define LOG_USE_SD          0
#define LOG_USE_UART        1
#define LOG_USE_IT          1
#define LOG_USE_DMA         0
#define LOG_BUFFER_SIZE     1024
#define LOG_RING_BUFFER_SIZE 2048
```

## Example

See [`Examples/STM32F4_UART_SD/main.c`](Examples/STM32F4_UART_SD/main.c) for a complete STM32F4 integration example.

## Documentation

See [`Docs/USAGE.md`](Docs/USAGE.md) for setup, buffer usage, SD logging integration and performance notes.

## License

MIT License

```
