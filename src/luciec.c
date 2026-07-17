#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#include "logging.h"
#include "lexing.h"

#define DEFAULT_OUTPUT_PATH "a.out"

// Options passed to the compiler
typedef struct {
  const char *source_filepath;
  const char *output_filepath;
  bool show_version;
  bool show_help;
} options_t;

// Parses the arguments passed to the compiler
//
// args MUST contain the input file's path
// as its first parameter
//
// This function throws an error using 'throw_error'
// in case of something unexpected when parsing
options_t parse_compiler_opts(char **const args, int n) {
  assert(n >= 1);
  
  // Default options
  options_t options = {
    .source_filepath = args[0],
    .output_filepath = NULL, // defaults to DEFAULT_OUTPUT_PATH if NULL
    .show_version = false,
    .show_help = false
  };
  
  if(strlen(options.source_filepath) <= strlen(".lucie") ||
    strcmp(
      options.source_filepath + strlen(options.source_filepath) - strlen(".lucie"), 
      ".lucie"
    ) != 0)
      throw_error("The input isn't a .lucie file.");

  bool waiting_for_outputfile = false;

  for(int i = 1; i < n; i++) {
    // Other arguments passed to the compiler
    
    // Check for flags:
    
    if(strcmp(args[i], "--help") == 0 || strcmp(args[i], "-H") == 0) {
      options.show_help = true;
      continue;
    }

    if(strcmp(args[i], "--version") == 0 || strcmp(args[i], "-V") == 0) {
      options.show_version = true;
      continue;
    }

    if(strcmp(args[i], "--output") == 0 || strcmp(args[i], "-O") == 0) {
      if(options.output_filepath != NULL)
        throw_error("Output file path has already been specified.");

      waiting_for_outputfile = true;
      continue;
    }

    if(args[i][0] == '-') {
      snprintf(log_msg_buff, LOG_MESSAGE_BUFFER_SIZE, "Unknown flag '%s'.", args[i]);
      throw_error(log_msg_buff);
    }

    // Check for arguments:

    if(waiting_for_outputfile) {
      options.output_filepath = args[i];
      waiting_for_outputfile = false;
    } else {
      snprintf(log_msg_buff, LOG_MESSAGE_BUFFER_SIZE, "Unexpected argument '%s'.", args[i]);
      throw_error(log_msg_buff);
    }
  }

  if(waiting_for_outputfile)
    throw_error("Output file wasn't specified.");

  if(options.output_filepath == NULL)
    options.output_filepath = DEFAULT_OUTPUT_PATH;

  return options;
}

// Retuns NULL if it isn't possible to allocate memory for the source's file 
// content
//
// The value size_out points to will hold the source's size if the allocation
// succeds.
//
// The source file must have been opened as a binary file
char *read_source(FILE *source_file, long *const size_out) {
  fseek(source_file, 0, SEEK_END);
  long source_size = ftell(source_file);

  char *source = (char*)malloc((source_size+1) * sizeof(char));

  if(source == NULL)
    return NULL;

  fseek(source_file, 0, SEEK_SET);
  fread(source, sizeof(char), source_size, source_file);
  source[source_size] = '\0';
  
  *size_out = source_size;
  return source;
}

int main(int argc, char **argv) {
  if(argc == 1)
    throw_error("No .lucie source code passed to compile.");

  options_t opts = parse_compiler_opts(argv + 1, argc - 1);

  FILE *source_file = fopen(opts.source_filepath, "rb");

  if(source_file == NULL) {
    snprintf(log_msg_buff, LOG_MESSAGE_BUFFER_SIZE,
             "It wasn't possible to read or find the file '%s'.",
             opts.source_filepath);
    throw_error(log_msg_buff);
  }

  long source_size;
  char *const source = read_source(source_file, &source_size);

  if(source == NULL)
    throw_error(MEMORY_ALLOCATION_ERRMSG);

  tokenized_source_t tokenized_source = tokenize_source(source, source_size);
 
  if(tokenized_source.had_errors) {
    puts("The compilation failed.");
    return EXIT_FAILURE;
  }
  
  // Parsing

  free(tokenized_source.read_tokens);
  free(source);
  return EXIT_SUCCESS;
}

