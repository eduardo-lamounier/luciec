#ifndef CHECKING_H
#define CHECKING_H

#include "parsing.h"

typedef struct checker checker_t;

// Returns whether there was a compilation error when checking ASTs.
bool checker_had_errors(const checker_t *checker);

// Returns the type annotated to a specific AST after checking.
type_t checker_AST_type(checker_t *checker, const expr_t *AST);


// Returns a new checker. Should be destroyed with 'checker_destroy(...)'
// when possible.
//
// Returns NULL if it isn't possible to allocate the resources.
checker_t *checker_new(expr_t *const *ASTs, size_t ASTs_amount);

// Releases all resources within the checker. The pointer passed to the
// checker becomes invalid.
void checker_destroy(checker_t *checker);


// Passes through all ASTs passed in 'checker_new(...)' and checks them.
void check_ASTs(checker_t *checker);

#endif
