#include "Logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h> 

void LOG_MESSAGE(const bool surpress_tag, const char* format, ...)
{
	va_list argptr;
	if (!surpress_tag)
	{
    	fprintf(stdout, "MESSAGE ");
	}
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
}

void LOG_WARNING(const bool surpress_tag, const char* file, const char* func, const int line, const char* format, ...)
{
	va_list argptr;
	if (!surpress_tag)
	{
    	fprintf(stdout, "WARNING %s::%s::%i    ", file, func, line);
	}
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
}

void LOG_ERROR(const bool surpress_tag, const char* file, const char* func, const int line, const char* format, ...)
{
	va_list argptr;
	if (!surpress_tag)
	{
    	fprintf(stdout, "ERROR %s::%s::%i    ", file, func, line);
	}
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    exit(EXIT_FAILURE);
}
