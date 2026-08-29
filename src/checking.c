#include "checking.h"

#include "logging.h"
#include "types.h"
#include "vendor/dynamic-arena.h"

typedef struct AST_type {
  const expr_t *AST;
  type_t type;
  struct AST_type *next;
} AST_type_t;

struct checker {
  dynamic_arena_t *arena;

  expr_t *const *input_ASTs;
  size_t input_ASTs_amount;
  bool had_errors;
  size_t current;

  // Linked list for all types annotaded for ASTs:
  AST_type_t *AST_types;
};

// Returns, by reference, the type set to an AST.
//
// Returns NULL if the type wasn't annotaded for the specified AST
static type_t *get_type_for_AST(checker_t *checker, const expr_t *AST) {
  AST_type_t *node = checker->AST_types;

  while(node != NULL) {
    if(node->AST == AST)
      return &node->type;

    node = node->next;
  }

  return NULL;
}

// Sets a type to an AST.
//
// Returns NULL if it isn't possible to allocate memory for the type annotation
//
// Should NOT be used if a type has ALREADY been set to the specified AST.
static type_t *add_type_for_AST(checker_t *checker,
                                const expr_t *AST, type_t type) {
  static AST_type_t *tail = NULL;

  AST_type_t *node =
    dy_arena_alloc(checker->arena, 1, sizeof(AST_type_t));
  
  if(node == NULL)
    error(MEMORY_ALLOCATION_ERRMSG);

  node->AST = AST;
  node->type = type;
  node->next = NULL;
  
  if(tail == NULL) { 
    checker->AST_types = node;
    tail = node;
    return &node->type;
  }
  
  tail->next = node;
  tail = node;
  return &node->type;
}

type_t checker_AST_type(checker_t *checker, const expr_t *AST) {
  return *get_type_for_AST(checker, AST);
}

static inline expr_t *peek(const checker_t *checker) {
  return checker->input_ASTs[checker->current];
}

static inline void advance(checker_t *checker) {
  checker->current++;
}

// Reports while updating the checker's inner error state.
static inline void checker_report_at(checker_t *checker,
                                     size_t line, const char *fmt, ...) {
  checker->had_errors = true;
  va_list args;
  va_start(args, fmt);
  vreport_at(line, fmt, args);
  va_end(args);
}

// Reports an unary operator not defined for some specific type.
static inline void undef_unary_op(checker_t *checker, const token_t *op, 
                                  type_t type) {
  checker_report_at(checker, op->line,
                    "Undefined operator '%s' for operand of type '%s'.",
                    op->lexeme, type_names[type]);
}

// Reports a binary operator not defined for some specific types.
static inline void undef_binary_op(checker_t *checker, const token_t *op,
                                   const type_t types[2]) {
  checker_report_at(checker, 1,
                    "Undefined operator '" str_view_FMT
                    "' for operands of type '%s' and '%s'.",
                    str_view_ARG(op->lexeme),
                    type_names[types[0]], type_names[types[1]]);
}

bool checker_had_errors(const checker_t *checker) {
  return checker->had_errors;
}

checker_t *checker_new(expr_t *const *ASTs, size_t ASTs_amount) {
  checker_t *checker = calloc(1, sizeof(checker_t));

  if(checker == NULL)
    return NULL;

  checker->arena = dy_arena_new(256 * sizeof(AST_type_t));

  if(checker->arena == NULL) {
    free(checker);
    return NULL;
  }

  checker->input_ASTs = ASTs;
  checker->input_ASTs_amount = ASTs_amount; 

  return checker;
}

void checker_destroy(checker_t *checker) {
  free(checker);
}

// Returns whether the minus unary operator is defined for the AST's operand's
// type.
//
// If it isn't (when returning `false`), reports an error; otherwise, the
// type pointed by 'type_out' is updated.
static bool check_minus_op(checker_t *checker, expr_t *AST, type_t *type_out) {
  assert(checker != NULL && AST != NULL && type_out != NULL);

  const type_t operand_type =
    checker_AST_type(checker, AST->val.as_unary.operand);

  switch (operand_type) {
    case TINT:
    case TUINT:
    case TLONG:
    case TULONG:
    case TFLOAT:
    case TDOUBLE:
      *type_out = operand_type;
      return true;
    default:
      break;
  }

  const token_t *op = AST->val.as_unary.operator;
  undef_unary_op(checker, op, operand_type);
  return false;
}

// Returns whether the bang unary operator is defined for the ASTs operand's
// type.
//
// If it isn't (when returning `false`), reports an error; otherwise, the
// type pointed by 'type_out' is updated.
static bool check_bang_op(checker_t *checker, expr_t *AST, type_t *type_out) {
  assert(checker != NULL && AST != NULL && type_out != NULL);

  const type_t operand_type =
    checker_AST_type(checker, AST->val.as_unary.operand);

  switch(operand_type) {
    case TBOOL:
      *type_out = TBOOL;
      return true;
    default:
      break;
  }

  const token_t *op = AST->val.as_unary.operator;
  undef_unary_op(checker, op, operand_type);
  return false;
}

// Gets a common numeric type for two types.
//
// 'type_out' is an optional out parameter.
//
// Returns whether it exists a common type, and reports if it doesn't.
static bool get_common_num_type(type_t type1, type_t type2, type_t *type_out) {
  assert(is_type_num(type1));
  assert(is_type_num(type2));

  // ensures 'type' points to something:
  type_t dummy; 
  type_t *type = &dummy;
  if(type_out != NULL)
    type = type_out;


  if(type1 == TDOUBLE || type2 == TDOUBLE) {
    *type = TDOUBLE; return true;
  }

  if(type1 == TFLOAT || type2 == TFLOAT) {
    *type = TFLOAT; return true;
  }

  switch(type1) {
    case TINT:
      switch(type2) {
        case TINT:
          *type = TINT; return true;
        case TUINT:
        case TLONG:
          *type = TLONG; return true;
        default:
          break;
      }
      break;
    case TUINT:
      switch(type2) {
        case TINT:
          *type = TLONG; return true;
        case TUINT:
          *type = TUINT; return true;
        case TLONG:
          *type = TLONG; return true;
        case TULONG:
          *type = TULONG; return true;
        default:
          break;
      }
      break;
    case TLONG:
      switch(type2) {
        case TINT:
        case TUINT:
        case TLONG:
          *type = TLONG; return true;
        default:
          break;
      }
      break;
    case TULONG:
      switch(type2) {
        case TUINT:
        case TULONG:
          *type = TULONG; return true;
        default:
          break;
      }
    default:
      break;
  }

  return false;
}

// Returns whether the arithmetic binary operators are defined for the AST's
// operands's type.
//
// If it isn't (when returning `false`), reports an error; otherwise, the
// type pointed by 'type_out' is updated. 
static bool check_arithmetic_op(checker_t *checker, expr_t *AST, type_t *type_out) {
  assert(checker != NULL && AST != NULL && type_out != NULL);

  const type_t operand_types[2] = {
    checker_AST_type(checker, AST->val.as_binary.operands[0]),
    checker_AST_type(checker, AST->val.as_binary.operands[1]),
  };  
  
  // TODO: Add support for string concatenation if the operator is '+'
  
  if(
    is_type_num(operand_types[0]) && is_type_num(operand_types[1]) &&
    get_common_num_type(operand_types[0], operand_types[1], type_out)
  )
    return true;

  const token_t *op = AST->val.as_binary.operator;
  undef_binary_op(checker, op, operand_types);
  return false;
}

// Returns whether the comparsion binary operators are defined for the AST's
// operands's type.
//
// If it isn't (when returning `false`), reports an error; otherwise, the
// type pointed by 'type_out' is updated.
static bool check_comparsion_op(checker_t *checker, expr_t *AST, type_t *type_out) {
  assert(checker != NULL && AST != NULL && type_out != NULL);

  const type_t operand_types[2] = {
    checker_AST_type(checker, AST->val.as_binary.operands[0]),
    checker_AST_type(checker, AST->val.as_binary.operands[1]),
  };

  if(
    is_type_num(operand_types[0]) && is_type_num(operand_types[1]) &&
    get_common_num_type(operand_types[0], operand_types[1], NULL)) {
    *type_out = TBOOL; return true;
  }

  const token_t *op = AST->val.as_binary.operator;
  undef_binary_op(checker, op, operand_types);
  return false;
}

// Returns whether the equality binary operators are defined for the AST's
// operands's type.
//
// If it isn't (when returning `false`), reports an error; otherwise, the
// type pointed by 'type_out' is updated.
static bool check_equality_op(checker_t *checker, expr_t *AST, type_t *type_out) {
  assert(checker != NULL && AST != NULL && type_out != NULL);

  const type_t operand_types[2] = {
    checker_AST_type(checker, AST->val.as_binary.operands[0]),
    checker_AST_type(checker, AST->val.as_binary.operands[1]),
  };

  if(operand_types[0] == operand_types[1]) {
    *type_out = TBOOL; return true;
  }

  if(is_type_num(operand_types[0]) && is_type_num(operand_types[1]))
    if(get_common_num_type(operand_types[0], operand_types[1], NULL)) {
      *type_out = TBOOL; return true;
    }

  const token_t *op = AST->val.as_binary.operator;
  undef_binary_op(checker, op, operand_types);
  return false;
}

// Verifies the AST and all inner ASTs.
static bool check_AST(checker_t *checker, expr_t *AST) {
  assert(checker != NULL && AST != NULL);

  type_t type;
  
  switch(AST->expr_kind) {
    case EXPR_LITERAL:
      type = AST->val.as_literal.data.type;
      break;
    case EXPR_GROUPING:
      if(!check_AST(checker, AST->val.as_grouping.inner_expr))
        return false;

      type = checker_AST_type(checker, AST->val.as_grouping.inner_expr);
      break;
    case EXPR_UNARY:
      if(!check_AST(checker, AST->val.as_unary.operand))
        return false;
      
      switch(AST->val.as_unary.operator->token_kind) {
        case TOKEN_MINUS:
          if(!check_minus_op(checker, AST->val.as_unary.operand, &type))
            return false;
          break;
        case TOKEN_BANG:
          if(!check_bang_op(checker, AST->val.as_unary.operand, &type))
            return false;
          break;
        default:
          unreachable();
      }
      break;
    case EXPR_BINARY:
      if(!check_AST(checker, AST->val.as_binary.operands[0]))
        return false;
      if(!check_AST(checker, AST->val.as_binary.operands[1]))
        return false;

      switch(AST->val.as_binary.operator->token_kind) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_STAR:
        case TOKEN_SLASH:
          if(!check_arithmetic_op(checker, AST, &type))
            return false;
          break;
        case TOKEN_GREATER:
        case TOKEN_LESS:
        case TOKEN_GREATER_EQUAL:
        case TOKEN_LESS_EQUAL:
          if(!check_comparsion_op(checker, AST, &type))
            return false;
          break;
        case TOKEN_EQUAL_EQUAL:
        case TOKEN_BANG_EQUAL:
          if(!check_equality_op(checker, AST, &type))
            return false;
          break;
        default:
          unreachable();
      }
      break;
  }

  add_type_for_AST(checker, AST, type);
  return true;
}

void check_ASTs(checker_t *checker) {
  for(; checker->current < checker->input_ASTs_amount; advance(checker)) {
    (void)check_AST(checker, peek(checker));
  }
}

