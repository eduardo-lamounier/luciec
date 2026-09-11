#ifndef LEXING_H
#define LEXING_H

#include<stdlib.h>
#include<stdbool.h>
#include<stdint.h>

#include "vendor/string-view.h"

#define LIST_TOKEN_KINDS                                                       \
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
  X(TOKEN_PRINT)                                                               \
  X(TOKEN_PRINTLN)                                                             \
                                                                               \
  X(TOKEN_EOF)

typedef enum {
  #define X(tt) tt,
  LIST_TOKEN_KINDS
  #undef X
} token_kind_t;

#define LIST_LITERAL_KINDS                                                     \
  X(LITERAL_INT) X(LITERAL_UINT)                                               \
  X(LITERAL_LONG) X(LITERAL_ULONG)                                             \
  X(LITERAL_FLOAT) X(LITERAL_DOUBLE)                                           \
  X(LITERAL_CHAR)                                                              \
  X(LITERAL_BOOL)                                                              \
  X(LITERAL_STR)                                                               \
  X(LITERAL_NULL)

typedef enum {
  #define X(lk) lk,
  LIST_LITERAL_KINDS
  #undef X
} literal_kind_t;

typedef struct {
  union {
    int32_t as_int;
    uint32_t as_uint;
    int64_t as_long;
    uint64_t as_ulong;
    float as_float;
    double as_double;
    char as_char;
    bool as_bool;
    string_view_t as_str;
  } value;
  literal_kind_t kind;
} literal_t;

typedef struct {
  token_kind_t token_kind;
  string_view_t lexeme;
  literal_t literal; 
  size_t line;
} token_t;

typedef struct lexer lexer_t;

#define is_literal(t) ((t).token_kind == TOKEN_NUM || (t).token_kind == TOKEN_STR)

#define is_num_literal(lt)                                                     \
  (                                                                            \
    lt == LITERAL_INT || lt == LITERAL_UINT ||                                 \
    lt == LITERAL_LONG || lt == LITERAL_ULONG ||                               \
    lt == LITERAL_FLOAT || lt == LITERAL_DOUBLE                                \
  )

#define null_literal() (literal_t) { .kind = LITERAL_NULL }

#define boolean_literal(v) (literal_t) {                                       \
  .kind = LITERAL_BOOL,                                                        \
  .value = { .as_bool = (v) },                                                 \
}

void print_literal(literal_t literal);


const token_t *lexer_tokens(const lexer_t *lexer);

size_t lexer_tokens_amount(const lexer_t *lexer);

bool lexer_had_errors(const lexer_t *lexer);

// 'source_size' must be the source's string length
lexer_t *lexer_new(const char *source, size_t source_size);

void lexer_destroy(lexer_t *lexer);

void lexer_scan_source(lexer_t *lexer);

extern const char *literal_FMTs[];

extern const char *token_lexemes[];

#endif
