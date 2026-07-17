#ifndef PARSING_H
#define PARSING_H

#include "lexing.h"

typedef struct expr {
  enum { UNARY, BINARY, LITERAL } expr_type;
  union {
    token_t as_literal;
    struct {
      token_t operator;
      struct expr **operands;
    } as_operation;
  } expr_result;
} expr_t;

#endif
