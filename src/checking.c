#include "checking.h"

#include "logging.h"

struct checker {
  expr_t *const *input_ASTs;
  size_t input_ASTs_amount;
  bool had_errors;
  size_t current;
};

static inline expr_t *peek(const checker_t *checker) {
  return checker->input_ASTs[checker->current];
}

static inline void advance(checker_t *checker) {
  checker->current++;
}

static inline void checker_report_at(checker_t *checker,
                                     size_t line, const char *fmt, ...) {
  checker->had_errors = true;
  va_list args;
  va_start(args, fmt);
  vreport_at(line, fmt, args);
  va_end(args);
}

bool checker_had_errors(const checker_t *checker) {
  return checker->had_errors;
}

checker_t *checker_new(expr_t *const *ASTs, size_t ASTs_amount) {
  checker_t *checker = calloc(1, sizeof(checker_t));

  if(checker == NULL)
    return NULL;

  checker->input_ASTs = ASTs;
  checker->input_ASTs_amount = ASTs_amount; 

  return checker;
}

void checker_destroy(checker_t *checker) {
  free(checker);
}

void check_AST(checker_t *checker, expr_t *AST) {
  (void)checker; (void)AST;
}

void check_ASTs(checker_t *checker) {
  for(; checker->current < checker->input_ASTs_amount; advance(checker)) {
    check_AST(checker, peek(checker));
  }
}
