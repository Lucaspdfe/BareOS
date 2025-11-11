#pragma once

// Level identifiers
#define LOG_CRIT  0
#define LOG_ERR   1
#define LOG_WARN  2
#define LOG_INFO  3
#define LOG_DEBUG 4

void i686_DEBUG_Initialize();
void i686_DEBUG_Debugc(char c);
void i686_DEBUG_Debugs(const char* s);
void i686_DEBUG_Debugf(int level, const char* fmt, ...);
