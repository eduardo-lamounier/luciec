#ifndef LOGGING_H
#define LOGGING_H

#include<stdlib.h>
#include<stdarg.h>
#include<assert.h>

#define MEMORY_ALLOCATION_ERRMSG "The input file exceeds the processing" \
                                 " size limits of this application.\n\n"

#define unreachable() do {                                                     \
    printf("Should not reach here.");                                          \
    assert(false);                                                             \
  } while(0)

// Displays the formated error message with the specified
// args list.
void vreport(const char *fmt, va_list args);

// Displays the formated error message with the specified
// args list and terminates the program.
void verror(const char *fmt, va_list args);

// Displays a formated warning message with the specifiedd
// args lists.
void vwarn(const char *fmt, va_list args);

// Displays the formated error message, with the specified args list, in the
// format:
// ERROR: At line [x]: <specified message>
//
void vreport_at(size_t line, const char *fmt, va_list args);

// Displays the formated error message, with the specified args list, in the
// format:
// ERROR: At line [x]: <specified message>
//
// Terminates the program afterwards.
void verror_at(size_t line, const char *fmt, va_list args);

// Displays the formated warning message, with the specified args list, in the
// format:
// WARNING: At line [x]: <specified-message>
//
void vwarn_at(size_t line, const char *fmt, va_list args);

// Displays the formated error message
void report(const char *fmt, ...);

// Reports the formated error and terminates the program
void error(const char *fmt, ...);

// Displays the formated warning message
void warn(const char *fmt, ...);

// Displays the formated error message in the format:
// ERROR: At line [x]: <specified message>
//
void report_at(size_t line, const char *fmt, ...);

// Displays the formated error message in the format:
// ERROR: At line [x]: <specified-message>
//
// Terminates the program afterwards.
void error_at(size_t line, const char *fmt, ...);

// Displays the formated warning message in the format:
// WARNING: At line [x]: <specified-message>
//
void warn_at(size_t line, const char *fmt, ...);

#endif
