#ifndef LOGGING_H
#define LOGGING_H

#define LOG_MESSAGE_BUFFER_SIZE 1024

#define MEMORY_ALLOCATION_ERRMSG "The input file exceeds the processing" \
                                 " size limits of this application."

// Displays an error message
void report_error(const char *const error_msg);

// Reports the error and terminates the program
void throw_error(const char *const error_msg);

// Just displays a warning message
void warn(const char *const warning);

// Buffer to print formatted messages. You should write the message
// into here with 'snprintf' and, after the message gets printed,
// leave it as it is
extern char log_msg_buff[LOG_MESSAGE_BUFFER_SIZE];

#endif
