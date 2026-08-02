#include "logging.h"

#include <stdarg.h>
#include<stdio.h>
#include<stdlib.h>

void vreport(const char *fmt, va_list args) {
  printf("\033[31m");
  printf("ERROR: ");
  vprintf(fmt, args);
  printf("\033[0m\n");
}

void verror(const char *fmt, va_list args) {
  vreport(fmt, args);
  exit(EXIT_FAILURE);
}

void vwarn(const char *fmt, va_list args) {
  printf("\033[33m");
  printf("fmt: ");
  vprintf(fmt, args);
  printf("\033[0m\n");
}

void report(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vreport(fmt, args);
  va_end(args);
}

void error(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vreport(fmt, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

void warn(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vwarn(fmt, args);
  va_end(args);
}

