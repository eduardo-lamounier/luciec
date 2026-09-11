#include<criterion/criterion.h>
#include<criterion/assert.h>

#include "lexing.h"
#include "logging.h"

void check_for_equal_tokens(token_t expected, token_t actual) {
  cr_expect(
    str_view_equals(expected.lexeme, actual.lexeme),
    "Expected lexeme \"" str_view_FMT "\", got \"" str_view_FMT "\".",
    str_view_ARG(expected.lexeme), str_view_ARG(actual.lexeme)
  );

  cr_expect(expected.line == actual.line,
            "Expected token to be at line %zu, got line %zu.",
            expected.line, actual.line);

  const char *expected_token_type, *actual_token_type;
  switch(expected.token_kind) {
    #define X(tt) case tt: expected_token_type = #tt; break;
    LIST_TOKEN_KINDS
    #undef X
  } 
  switch(actual.token_kind) {
    #define X(tt) case tt: actual_token_type = #tt; break;
    LIST_TOKEN_KINDS
    #undef X
  }

  cr_assert(expected.token_kind == actual.token_kind,
            "Expected token of kind '%s', got '%s'.",
            expected_token_type,
            actual_token_type
  );

  if(!is_literal(expected))
    return;

  cr_expect(expected.literal.kind == actual.literal.kind);

  switch (expected.literal.kind) {
    case LITERAL_INT:
      cr_expect(expected.literal.value.as_int == actual.literal.value.as_int,
                "Expected a literal value of %d, got %d.",
                expected.literal.value.as_int, actual.literal.value.as_int
      );
      break;
    case LITERAL_DOUBLE:
      cr_expect(expected.literal.value.as_double == actual.literal.value.as_double,
                "Expected a literal value of %lf, got %lf.",
                expected.literal.value.as_double, actual.literal.value.as_double
      );
      break;
    case LITERAL_FLOAT:
      cr_expect(expected.literal.value.as_float == actual.literal.value.as_float,
                "Expected a literal value of %f, got %f.",
                expected.literal.value.as_float, actual.literal.value.as_float
      );
      break;
    case LITERAL_CHAR:
      cr_expect(expected.literal.value.as_char == actual.literal.value.as_char,
                "Expected a literal value of '%c', got '%c'.",
                expected.literal.value.as_char, actual.literal.value.as_char
      );
      break;
    case LITERAL_STR:
      cr_expect(str_view_equals(expected.literal.value.as_str, actual.literal.value.as_str),
                "Expected a literal value of \"" str_view_FMT "\", got \"" str_view_FMT "\".",
                str_view_ARG(expected.literal.value.as_str),
                str_view_ARG(actual.literal.value.as_str)
      );
      break;
    case LITERAL_BOOL:
      cr_expect(expected.literal.value.as_bool == actual.literal.value.as_bool,
                "Expected a literal value of '%s', got '%s'.",
                (expected.literal.value.as_bool ? "true" : "false"),
                (actual.literal.value.as_bool ? "true" : "false")
      );
      break;
    default:
      unreachable();
  } 
}

#define tok(tk, l) (token_t) {                                                 \
    .token_kind = (tk),                                                        \
    .lexeme = str_view_from(token_lexemes[tk]),                                \
    .line = (l),                                                               \
  }

#define id_tok(lex, l) (token_t) {                                             \
    .token_kind = TOKEN_ID,                                                    \
    .lexeme = str_view_from(lex),                                              \
    .line = (l),                                                               \
  }

#define num_tok(v, lex, l) (token_t) {                                         \
    .token_kind = TOKEN_NUM,                                                   \
    .lexeme = str_view_from(lex),                                              \
    .literal = {                                                               \
      .value = { .as_int = (v) },                                              \
      .kind = LITERAL_INT,                                                     \
    },                                                                         \
    .line = (l),                                                               \
  }

Test(lexer_testing, source_code1) {
  char *source = "func main() {\n"
                 "  x: int = 2 + 3;\n"
                 "}";

  lexer_t *lexer = lexer_new(source, strlen(source));
 
  lexer_scan_source(lexer);

  token_t expected_tokens[] = {
    tok(TOKEN_FUNC, 1),
    id_tok("main", 1),
    tok(TOKEN_LPAREN, 1),
    tok(TOKEN_RPAREN, 1),
    tok(TOKEN_LBRACE, 1),
    id_tok("x", 2) ,
    tok(TOKEN_COLON, 2),
    id_tok("int", 2),
    tok(TOKEN_EQUAL, 2),
    num_tok(2, "2", 2),
    tok(TOKEN_PLUS, 2),
    num_tok(3, "3", 2),
    tok(TOKEN_SEMICOLON, 2),
    tok(TOKEN_RBRACE, 3),
    tok(TOKEN_EOF, 3),
  };

  bool expected_error = false;
  cr_assert_eq(expected_error, lexer_had_errors(lexer));

  const token_t *tokens = lexer_tokens(lexer);
  size_t tokens_amount = lexer_tokens_amount(lexer);

  cr_assert_eq(tokens_amount, sizeof(expected_tokens) / sizeof(token_t));

  for(size_t i = 0; i < tokens_amount; i++)
    check_for_equal_tokens(expected_tokens[i], tokens[i]);
}

