#ifndef CHECKING_H
#define CHECKING_H

#include "parsing.h"

typedef struct checker checker_t;

bool checker_had_errors(const checker_t *checker);

type_t checker_AST_type(checker_t *checker, const expr_t *AST);

checker_t *checker_new(expr_t *const *ASTs, size_t ASTs_amount);
void checker_destroy(checker_t *checker);

void check_ASTs(checker_t *checker);

#endif
