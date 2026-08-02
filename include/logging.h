#ifndef LOGGING_H
#define LOGGING_H

#include<stdarg.h>

#define MEMORY_ALLOCATION_ERRMSG "The input file exceeds the processing" \
                                 " size limits of this application.\n\n"

// Displays the formated error message with the specified
// args list.
void vreport(const char *fmt, va_list args);

// Displays the formated error message with the specified
// args list and terminates the program.
void verror(const char *fmt, va_list args);

// Displays a formated warning message with the specifiedd
// args lists.
void vwarn(const char *fmt, va_list args);

// Displays the formated error message
void report(const char *fmt, ...);

// Reports the formated error and terminates the program
void error(const char *fmt, ...);

// Displays the formated warning message
void warn(const char *fmt, ...);

#endif
