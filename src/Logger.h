#ifndef LOGGER_H
#define LOGGER_H

void LOG_MESSAGE(const bool surpress_tag, const char* format, ...);
void LOG_WARNING(const bool surpress_tag, const char* file, const char* func, const int line, const char* format, ...);
void LOG_ERROR(const bool surpress_tag, const char* file, const char* func, const int line, const char* format, ...);

#endif
