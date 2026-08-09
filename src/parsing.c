#include "parsing.h"

#include "logging.h"
#define DYNAMIC_ARENA_IMPLEMENTATION
#include "vendor/dynamic-arena.h"

#define ASTS_INITIAL_CAPACITY 2

struct parser {
  dynamic_arena_t *arena;

  const token_t *input_tokens;
  bool had_errors;
  size_t current;

  size_t ASTs_capacity;
  size_t ASTs_amount;
  expr_t **ASTs;
};

static inline const token_t *peek(const parser_t *parser) {
  return parser->input_tokens + parser->current;
}

static inline bool matches(parser_t *parser, token_type_t token_type) {
  return peek(parser)->token_type == token_type;
}

static inline void advance(parser_t *parser) {
  if(!matches(parser, TOKEN_EOF))
    parser->current++;
}

static inline void parser_report_at(parser_t *parser,
                                   size_t line, const char *fmt, ...) {
  parser->had_errors = true;
  va_list args;
  va_start(args, fmt);
  vreport_at(line, fmt, args);
  va_end(args);
}

static bool expect(parser_t *parser, token_type_t token_type) {
  if(!matches(parser, token_type)) {
    const token_t *current = peek(parser);
    report_at(current->line, "Unexpected '%s'.\n", current->lexeme);
    parser_report_at(parser, current->line, "Unexpected '%s'.\n",
                     current->lexeme);
    return false;
  }

  advance(parser);
  return true;
}

static void synchronize(parser_t *parser) {
  advance(parser);

  for(; !matches(parser, TOKEN_EOF); advance(parser)) {
    if(matches(parser, TOKEN_SEMICOLON)) {
      advance(parser);
      return;
    }

    switch(peek(parser)->token_type) {
      case TOKEN_FUNC:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_FOR:
      case TOKEN_RETURN:
        return;
      default:
        break;
    }
  }
}

static expr_t *new_grouping_expr(dynamic_arena_t *arena, expr_t *sub_expr) {
  assert(arena != NULL);
  expr_t *expr = dy_arena_alloc(arena, 1, sizeof(expr_t));

  if(expr == NULL)
    error(MEMORY_ALLOCATION_ERRMSG);

  *expr = (expr_t) {
    .expr_type = GROUPING_EXPR,
    .val = { .as_grouping = sub_expr }
  };

  return expr;
}

static expr_t *new_literal_expr(dynamic_arena_t *arena, value_t value) {
  assert(arena != NULL);
  expr_t *expr = dy_arena_alloc(arena, 1, sizeof(expr_t));

  if(expr == NULL)
    error(MEMORY_ALLOCATION_ERRMSG);

  *expr = (expr_t) {
    .expr_type = LITERAL_EXPR,
    .val = { .as_literal = value },
  };

  return expr;
}

static expr_t *new_unary_expr(dynamic_arena_t *arena, const token_t *operator,
                              expr_t *operand) {
  assert(arena != NULL && operator != NULL && operand != NULL);
  expr_t *expr = dy_arena_alloc(arena, 1, sizeof(expr_t));

  if(expr == NULL)
    error(MEMORY_ALLOCATION_ERRMSG);

  *expr = (expr_t) {
    .expr_type = UNARY_EXPR,
    .val = {
      .as_unary = {
        .operator = operator,
        .operand = operand,
      },
    },
  };

  return expr;
}

static expr_t *new_binary_expr(dynamic_arena_t *arena, const token_t *operator,
                              expr_t *left_operand,
                              expr_t *right_operand) {
  assert(arena != NULL && operator != NULL
         && left_operand != NULL && right_operand != NULL);
  expr_t *expr = dy_arena_alloc(arena, 1, sizeof(expr_t));

  if(expr == NULL)
    error(MEMORY_ALLOCATION_ERRMSG);

  *expr = (expr_t) {
    .expr_type = BINARY_EXPR,
    .val = {
      .as_binary = {
        .operator = operator,
        .operands = { left_operand, right_operand }
      }
    }
  };
  
  return expr;
}

static expr_t *primary(parser_t *parser);
static expr_t *unary(parser_t *parser);
static expr_t *factor(parser_t *parser);
static expr_t *term(parser_t *parser);
static expr_t *comparison(parser_t *parser);
static expr_t *equality(parser_t *parser);
static expr_t *expression(parser_t *parser);

static expr_t *primary(parser_t *parser) {
  if(matches(parser, TOKEN_TRUE)) {
    advance(parser);
    return new_literal_expr(parser->arena, boolean_literal(true));
  }

  if(matches(parser, TOKEN_FALSE)) {
    advance(parser);
    return new_literal_expr(parser->arena, boolean_literal(false));
  }

  if(matches(parser, TOKEN_NULL)) {
    advance(parser);
    return new_literal_expr(parser->arena, null_literal());
  }

  if(matches(parser, TOKEN_NUM) || matches(parser, TOKEN_STR)) {
    value_t literal = peek(parser)->literal;
    advance(parser);
    return new_literal_expr(parser->arena, literal);
  }

  if(matches(parser, TOKEN_LPAREN)) {
    advance(parser);
    expr_t *sub_expr = expression(parser); 

    expect(parser, TOKEN_RPAREN);

    return new_grouping_expr(parser->arena, sub_expr);
  }
 
  parser_report_at(parser, peek(parser)->line, "Expected expression.");
  return NULL;
}

static expr_t *unary(parser_t *parser) {
  if(matches(parser, TOKEN_BANG) || matches(parser, TOKEN_MINUS)) {
    const token_t *operator = peek(parser);
    advance(parser);
    expr_t *operand = unary(parser);

    if(operand == NULL)
      return NULL;

    return new_unary_expr(parser->arena, operator, operand);
  }

  return primary(parser);
}

static expr_t *factor(parser_t *parser) {
  expr_t *AST = unary(parser);

  if(AST == NULL)
    return NULL;

  while(matches(parser, TOKEN_STAR) || matches(parser, TOKEN_SLASH)) {
    const token_t *operator = peek(parser);
    advance(parser);
    expr_t *right_operand = unary(parser);

    if(right_operand == NULL)
      return NULL;

    AST = new_binary_expr(parser->arena, operator, AST, right_operand);
  }

  return AST;
}

static expr_t *term(parser_t *parser) {
  expr_t *AST = factor(parser);

  if(AST == NULL)
    return NULL;

  while(matches(parser, TOKEN_PLUS) || matches(parser, TOKEN_MINUS)) {
    const token_t *operator = peek(parser);
    advance(parser);
    expr_t *right_operand = factor(parser);

    if(right_operand == NULL)
      return NULL;

    AST = new_binary_expr(parser->arena, operator, AST, right_operand);
  }

  return AST;
}

static expr_t *comparison(parser_t *parser) {
  expr_t *AST = term(parser);

  if(AST == NULL)
    return NULL;

  while(matches(parser, TOKEN_LESS)
        || matches(parser, TOKEN_LESS_EQUAL)
        || matches(parser, TOKEN_GREATER)
        || matches(parser, TOKEN_GREATER_EQUAL)) {
    const token_t *operator = peek(parser);
    advance(parser);
    expr_t *right_operand = term(parser);

    if(right_operand == NULL)
      return NULL;
    
    AST = new_binary_expr(parser->arena, operator, AST, right_operand);
  }

  return AST;
}

static expr_t *equality(parser_t *parser) {
  expr_t *AST = comparison(parser);

  if(AST == NULL)
    return NULL;

  while(matches(parser, TOKEN_EQUAL_EQUAL)
        || matches(parser, TOKEN_BANG_EQUAL)) {     
    const token_t *operator = peek(parser);
    advance(parser);
    expr_t *right_operand = comparison(parser);

    if(right_operand == NULL)
      return NULL;

    AST = new_binary_expr(parser->arena, operator, AST, right_operand);
  }

  return AST;
}

static expr_t *expression(parser_t *parser) {
  return equality(parser);
}

// Returns `true` if the parsing occurred successfully, returns `false`
// otherwise.
static bool parse_AST(parser_t *parser, expr_t **AST_out) {
  *AST_out = expression(parser);
  return *AST_out != NULL;
}

static void add_AST(parser_t *parser, expr_t *AST) {
  if(parser->ASTs_amount + 1 > parser->ASTs_capacity) {
    parser->ASTs_capacity *= 1.5;
    parser->ASTs= realloc(
      parser->ASTs, parser->ASTs_capacity * sizeof(expr_t*)
    );

    if(parser->ASTs == NULL)
      error(MEMORY_ALLOCATION_ERRMSG);
  }

  parser->ASTs[parser->ASTs_amount++] = AST;
}



expr_t *parser_AST(const parser_t *parser, size_t idx) {
  assert(idx < parser->ASTs_amount);
  return parser->ASTs[idx];
}

size_t parser_ASTs_amount(const parser_t *parser) {
  return parser->ASTs_amount;
}

bool parser_had_errors(const parser_t *parser) {
  return parser->had_errors;
}

parser_t *parser_new(const token_t *tokens) {
  parser_t *parser = malloc(sizeof(parser_t));

  if(parser == NULL)
    return NULL;

  *parser = (parser_t) {
    .arena = dy_arena_new(256 * sizeof(expr_t)),
    .input_tokens = tokens,
    .had_errors = false,
    .current = 0,
    .ASTs_capacity = ASTS_INITIAL_CAPACITY,
    .ASTs =
      (expr_t**)malloc(ASTS_INITIAL_CAPACITY * sizeof(token_t*)),
    .ASTs_amount = 0,
  };

  if(parser->arena == NULL) {
    if(parser->ASTs != NULL)
      free(parser->ASTs);

    free(parser);
    parser = NULL;
    return NULL;
  }

  if(parser->ASTs == NULL) {
    if(parser->arena != NULL)
      free(parser->arena);

    free(parser);
    return NULL;
  }

  return parser;
}

void parser_destroy(parser_t *parser) {
  assert(parser != NULL);
  dy_arena_destroy(&parser->arena);
  free(parser->ASTs);
  free(parser);
}

void parse_ASTs(parser_t *parser) {
  assert(parser != NULL);
 
  // TODO: Parse multiple ASTs
  expr_t *AST;
  if(parse_AST(parser, &AST))
    add_AST(parser, AST);
}

static void _show_AST(const expr_t *expr, bool put_space) {
  assert(expr != NULL);
  
  switch(expr->expr_type) {
    case GROUPING_EXPR:
      _show_AST(expr->val.as_grouping, false);
      break;
    case UNARY_EXPR:
      printf(str_view_FMT, str_view_ARG(expr->val.as_unary.operator->lexeme));
      printf("( ");
      _show_AST(expr->val.as_unary.operand, true);
      printf(")");
      break;
    case BINARY_EXPR:
      printf(str_view_FMT, str_view_ARG(expr->val.as_binary.operator->lexeme));
      printf("( ");
      _show_AST(expr->val.as_binary.operands[0], false);
      printf(", ");
      _show_AST(expr->val.as_binary.operands[1], true);
      printf(")");
      break;
    case LITERAL_EXPR:
      value_print(expr->val.as_literal);
      break;
    default:
      assert(false); // Should not get into here
  }

  if(put_space)
    printf(" ");
}

void show_AST(const expr_t *AST) {
  _show_AST(AST, false);
  printf("\n"); 
  fflush(stdout);
}

