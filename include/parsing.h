#ifndef PARSING_H
#define PARSING_H

#include "lexing.h"

typedef struct expr { 
  union {
    struct {
      const token_t *operator;
      struct expr *operand;
    } as_unary;
    struct {
      const token_t *operator;
      struct expr *operands[2];
    } as_binary;
    value_t as_literal;
    struct expr *as_grouping;
  } val;
  enum {
    UNARY_EXPR,
    BINARY_EXPR,
    LITERAL_EXPR,
    GROUPING_EXPR,
  } expr_type;
} expr_t;

typedef struct parser parser_t;

expr_t *parser_AST(const parser_t *parser, size_t idx);

size_t parser_ASTs_amount(const parser_t *parser);

bool parser_had_errors(const parser_t *parser);


parser_t *parser_new(const token_t *tokens);

void parser_destroy(parser_t *parser);


void show_AST(const expr_t *AST);

void parse_ASTs(parser_t *parser);

#endif
