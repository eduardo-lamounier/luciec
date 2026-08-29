#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<stdint.h>

#include "platform.h"
#include "logging.h"
#include "lexing.h"
#include "parsing.h"

#define VERSION_MESSAGE "luciec 1.0.0v"
#define HELP_MESSAGE "Usage: luciec [OPTIONS] file...\n"                       \
  "\n[--help | -H]: Show this reference."                                      \
  "\n[--version | -V]: Shows the version of luciec installed in your system."  \
  "\n[--output | -O]: Specifies path for the output executable"                \
  ""

#define NO_SOURCE_FILE_MESSAGE "No source file passed to compile."
#define HAD_ERRORS_MESSAGE "The compilation failed."

// Options passed to the compiler
typedef struct {
  const char *source_filepath;
  const char *output_filepath;
  bool show_version;
  bool show_help;
} options_t;

// Parses the arguments passed to the compiler
//
// The first argument of the program (the executable's path) should NOT be
// included.
//
// This function throws an error using 'error()'
// in case of something unexpected when parsing
options_t parse_compiler_opts(char **const args, int n) {
  assert(n >= 1);
  
  // Default options
  options_t options = {
    .source_filepath = NULL,
    .output_filepath = NULL, // defaults to the source file's name
                             // (without the .lucie extension)
    .show_version = false,
    .show_help = false
  };

  bool waiting_for_sourcefile = true;
  bool waiting_for_outputfile = false;

  for(int i = 0; i < n; i++) {
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
        error("Output file path has already been specified.");

      waiting_for_outputfile = true;
      continue;
    }

    if(args[i][0] == '-')
      error("Unknown flag '%s'.", args[i]);

    // Check for arguments:
    
    if(waiting_for_outputfile) {
      options.output_filepath = args[i];
      waiting_for_outputfile = false;
      continue;
    }

    if(waiting_for_sourcefile) {
      options.source_filepath = args[i];
      waiting_for_sourcefile = false;
      continue;
    }

    error("Unexpected argument '%s'.", args[i]);
  }

  if(options.source_filepath != NULL && (
    strlen(options.source_filepath) <= strlen(".lucie") ||
    strcmp(
      options.source_filepath + strlen(options.source_filepath) - strlen(".lucie"), 
      ".lucie"
    ) != 0)
  )
      error("The input isn't a .lucie file.");

  if(waiting_for_outputfile)
    error("Output file wasn't specified.");

  if(options.output_filepath == NULL && options.source_filepath != NULL) {
    const char *source_filename = file_name_from_path(options.source_filepath);

    size_t len = strlen(source_filename) - strlen(".lucie");
    char *output_filepath = calloc(len+1, 1);
    memmove(output_filepath, source_filename, len);

    options.output_filepath = output_filepath;
  }

  return options;
}

// Retuns NULL if it isn't possible to allocate memory for the source's file 
// content
//
// The value size_out points to will hold the source's size if the allocation
// succeds.
char *read_source(const char *filepath, long *const size_out) {
  assert(filepath != NULL && size_out != NULL);

  FILE *source_file = fopen(filepath, "rb");

  if(source_file == NULL)
    error("It wasn't possible to read or find the file '%s'.", filepath); 

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
    error(NO_SOURCE_FILE_MESSAGE);

  options_t opts = parse_compiler_opts(argv + 1, argc - 1);

  if(opts.show_version) {
    puts(VERSION_MESSAGE); return EXIT_SUCCESS;
  }

  if(opts.show_help) {
    puts(HELP_MESSAGE); return EXIT_SUCCESS;
  }
  
  // No option that would made the program terminate was passed, so we need the
  // source file to compile:
  if(opts.source_filepath == NULL)
    error(NO_SOURCE_FILE_MESSAGE);
 
  long source_size;
  char *const source = read_source(opts.source_filepath, &source_size);

  if(source == NULL)
    error(MEMORY_ALLOCATION_ERRMSG);
  
  lexer_t *lexer = lexer_new(source, source_size);

  if(lexer == NULL) {
    puts(MEMORY_ALLOCATION_ERRMSG); return EXIT_FAILURE;
  }

  lexer_scan_source(lexer);
 
  if(lexer_had_errors(lexer)) {
    puts(HAD_ERRORS_MESSAGE); return EXIT_FAILURE;
  }
  
  parser_t *parser = parser_new(lexer_tokens(lexer));

  if(parser == NULL) {
    puts(MEMORY_ALLOCATION_ERRMSG); return EXIT_FAILURE;
  }

  parse_ASTs(parser);

  if(parser_had_errors(parser)) {
    puts(HAD_ERRORS_MESSAGE); return EXIT_FAILURE;
  }

  expr_t *AST = parser_AST(parser, 0);
  show_AST(AST);

  lexer_destroy(lexer);
  parser_destroy(parser);

  free(source);
  return EXIT_SUCCESS;
}

