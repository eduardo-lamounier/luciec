#include "logging.h"

#include<stdio.h>
#include<stdlib.h>

void report_error(const char *const error_msg) {
  printf("\033[31m");
  printf("ERROR: %s", error_msg);
  printf("\033[0m\n");
}

void throw_error(const char *const error_msg) {
  report_error(error_msg);
  exit(EXIT_FAILURE);
}

void warn(const char *const warning) {
  printf("\033[33m");
  printf("WARNING: %s", warning);
  printf("\033[0m\n");
}

char log_msg_buff[LOG_MESSAGE_BUFFER_SIZE];

