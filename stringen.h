#ifndef STRING_EN_H
#define STRING_EN_H

#include <Arduino.h>

// Define debug levels
#define Gr_ERROR 0
#define Gr_WARNING 1
#define Gr_INFO 2
#define Gr_VERBOSE 3

// Set the current debug level (change this as needed)
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_VERBOSE
#endif

// Define macros for debugging
#ifdef DEBUG_ENABLED
#define DEBUG_PRINT(level, ...) if (level <= DEBUG_LEVEL) { Serial.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(level, ...) if (level <= DEBUG_LEVEL) { Serial.println(__VA_ARGS__); }
#define DEBUG_PRINTF(level, ...) if (level <= DEBUG_LEVEL) { Serial.printf(__VA_ARGS__); }
#else
#define DEBUG_PRINT(level, ...)
#define DEBUG_PRINTLN(level, ...)
#define DEBUG_PRINTF(level, ...)
#endif

#endif // DEBUG_UTILS_H

