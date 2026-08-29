#ifndef PARSING_H
#define PARSING_H

#include "lexing.h"

typedef struct expr expr_t;

typedef enum {
  EXPR_UNARY,
  EXPR_BINARY,
  EXPR_LITERAL,
  EXPR_GROUPING,
} expr_kind_t;

typedef struct {
  const token_t *operator;
  expr_t *operand;
} unary_expr_t;

typedef struct {
  const token_t *operator;
  struct expr *operands[2];
} binary_expr_t;

typedef struct {
  value_t data;
} literal_expr_t;

typedef struct {
  expr_t *inner_expr;
} group_expr_t;

struct expr { 
  union {
    unary_expr_t as_unary;
    binary_expr_t as_binary;
    literal_expr_t as_literal;
    group_expr_t as_grouping;
  } val;
  expr_kind_t expr_kind;
};

typedef struct parser parser_t;


// Returns all the parsed ASTs.
expr_t *const *parser_ASTs(const parser_t *parser);

// Returns a parsed AST at a specific index.
expr_t *parser_AST(const parser_t *parser, size_t idx);

// Returns the amount of parsed ASTs.
size_t parser_ASTs_amount(const parser_t *parser);

// Returns whether a compilation error occurred in parsing.
bool parser_had_errors(const parser_t *parser);


// Creates a new parser. Its resources have to be released with
// 'parser_destroy(...)'.
parser_t *parser_new(const token_t *tokens);

// Releases all resources within a parser. The pointer to the parser becomes
// invalid.
void parser_destroy(parser_t *parser);


// Prints an AST to the screen.
void show_AST(const expr_t *AST);

// Passes through all specified tokens and returns the ASTs.
void parse_ASTs(parser_t *parser);

#endif
