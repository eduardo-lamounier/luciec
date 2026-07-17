#ifndef LEXING_H
#define LEXING_H

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

#include "utils/string_view.h"

#define LIST_TOKEN_TYPES                                                       \
  X(TOKEN_LPAREN) X(TOKEN_RPAREN)                                              \
  X(TOKEN_LBRACE) X(TOKEN_RBRACE)                                              \
  X(TOKEN_COMMA) X(TOKEN_SEMICOLON) X(TOKEN_COLON)                             \
  X(TOKEN_PLUS) X(TOKEN_MINUS) X(TOKEN_STAR) X(TOKEN_SLASH)                    \
                                                                               \
  /* One character long with 2 character long variations */                    \
  X(TOKEN_AMP) X(TOKEN_PIPE)                                                   \
  X(TOKEN_BANG) X(TOKEN_EQUAL)                                                 \
  X(TOKEN_LESS) X(TOKEN_GREATER)                                               \
                                                                               \
  /* Two characters long */                                                    \
  X(TOKEN_AMP_AMP) X(TOKEN_PIPE_PIPE)                                          \
  X(TOKEN_BANG_EQUAL) X(TOKEN_EQUAL_EQUAL)                                     \
  X(TOKEN_LESS_EQUAL) X(TOKEN_GREATER_EQUAL)                                   \
  X(TOKEN_EQUAL_GREATER)                                                       \
                                                                               \
  /* Literals */                                                               \
  X(TOKEN_ID) X(TOKEN_STR) X(TOKEN_NUM)                                        \
                                                                               \
  /* Keywords */                                                               \
  X(TOKEN_NULL)                                                                \
  X(TOKEN_FALSE)                                                               \
  X(TOKEN_TRUE)                                                                \
  X(TOKEN_IF)                                                                  \
  X(TOKEN_ELSE)                                                                \
  X(TOKEN_WHILE)                                                               \
  X(TOKEN_FOR)                                                                 \
  X(TOKEN_FUNC)                                                                \
  X(TOKEN_RETURN)                                                              \
  X(TOKEN_USING)                                                               \
                                                                               \
  X(TOKEN_EOF)

typedef enum {
  #define X(tt) tt,
  LIST_TOKEN_TYPES
  #undef X
} token_type_t;

typedef struct {
  enum { TINT, TFLOAT, TDOUBLE, TCHAR, TSTR, TBOOL } type;

  union {
    int as_int;
    float as_float;
    double as_double;
    char as_char;
    bool as_bool;
    string_view_t as_str;
  } data;
} value_t;

typedef struct {
  token_type_t token_type;
  string_view_t lexeme;
  value_t literal; 
  size_t line;
} token_t;

typedef struct {
  const size_t read_tokens_amount;
  token_t *const read_tokens;
  const bool had_errors;
} tokenized_source_t;

#define IS_LITERAL(t) (t).token_type == TOKEN_NUM || (t).token_type == TOKEN_STR

// Must be used to free the token and all dynamically allocated
// fields inside
void token_clear(token_t *const token);

// The pointer 'read_tokens' (a field of the returned struct)
// references the array containing all tokens in the source code,
// and MUST BE FREED, because it's allocated dynamically
//
// 'source_size' must be the source's string length
tokenized_source_t tokenize_source(const char *const source_file,
                                   const size_t source_size);

#endif
