#include<stdio.h>
#include<criterion/criterion.h>
#include<criterion/assert.h>

#include "utils/string_view.h"
#include "lexing.h"

void check_for_equal_tokens(token_t expected, token_t actual) {
  cr_expect(
    strview_equals(expected.lexeme, actual.lexeme),
    "Expected lexeme \"" STRVIEW_Fmt "\", got \"" STRVIEW_Fmt "\".",
    STRVIEW_Arg(expected.lexeme), STRVIEW_Arg(actual.lexeme)
  );

  cr_expect(expected.line == actual.line,
            "Expected token to be at line %zu, got line %zu.",
            expected.line, actual.line);

  const char *expected_token_type, *actual_token_type;
  switch(expected.token_type) {
    #define X(tt) case tt: expected_token_type = #tt; break;
    LIST_TOKEN_TYPES
    #undef X
  } 
  switch(actual.token_type) {
    #define X(tt) case tt: actual_token_type = #tt; break;
    LIST_TOKEN_TYPES
    #undef X
  }

  cr_assert(expected.token_type == actual.token_type,
            "Expected token of type '%s', got '%s'.",
            expected_token_type,
            actual_token_type
  );

  if (IS_LITERAL(expected)) {
    cr_expect(expected.literal.type == actual.literal.type);
    switch (expected.literal.type) {
      case TINT:
        cr_expect(expected.literal.data.as_int == actual.literal.data.as_int,
                  "Expected a literal value of %d, got %d.",
                  expected.literal.data.as_int, actual.literal.data.as_int
        );
        break;
      case TDOUBLE:
        cr_expect(expected.literal.data.as_double == actual.literal.data.as_double,
                  "Expected a literal value of %f, got %f.",
                  expected.literal.data.as_double, actual.literal.data.as_double
        );
        break;
      case TFLOAT:
        cr_expect(expected.literal.data.as_float == actual.literal.data.as_float,
                  "Expected a literal value of %f, got %f.",
                  expected.literal.data.as_float, actual.literal.data.as_float
        );
        break;
      case TCHAR:
        cr_expect(expected.literal.data.as_char == actual.literal.data.as_char,
                  "Expected a literal value of '%c', got '%c'.",
                  expected.literal.data.as_char, actual.literal.data.as_char
        );
        break;
      case TSTR:
        cr_expect(strview_equals(expected.literal.data.as_str, actual.literal.data.as_str),
                  "Expected a literal value of \"" STRVIEW_Fmt "\", got \"" STRVIEW_Fmt "\".",
                  STRVIEW_Arg(expected.literal.data.as_str),
                  STRVIEW_Arg(actual.literal.data.as_str)
        );
        break;
      case TBOOL:
        cr_expect(expected.literal.data.as_bool == actual.literal.data.as_bool,
                  "Expected a literal value of '%s', got '%s'.",
                  (expected.literal.data.as_bool ? "true" : "false"),
                  (actual.literal.data.as_bool ? "true" : "false")
        );
        break;
      }
  }
}

Test(lexer_testing, source_code1) {
  char *source = "func main() {\n"
                 "  x: int = 2 + 3;\n"
                 "}";

  tokenized_source_t res =
    tokenize_source(source, strlen(source));

  token_t expected_tokens[] = {
    (token_t) {
      .token_type = TOKEN_FUNC,
      .lexeme = strview_from("func"),
      .line = 1,
    },
    (token_t) {
      .token_type = TOKEN_ID,
      .lexeme = strview_from("main"),
      .line = 1,
    },
    (token_t) {
      .token_type = TOKEN_LPAREN,
      .lexeme = strview_from("("),
      .line = 1,
    },
    (token_t) {
      .token_type = TOKEN_RPAREN,
      .lexeme = strview_from(")"),
      .line = 1,
    },
    (token_t) {
      .token_type = TOKEN_LBRACE,
      .lexeme = strview_from("{"),
      .line = 1,
    },
    (token_t) {
      .token_type = TOKEN_ID,
      .lexeme = strview_from("x"),
      .line = 2,
    },
    (token_t) {
      .token_type = TOKEN_COLON,
      .lexeme = strview_from(":"),
      .line = 2,
    },
    (token_t) {
      .token_type = TOKEN_ID,
      .lexeme = strview_from("int"),
      .line = 2,
    },
    (token_t) {
      .token_type = TOKEN_EQUAL,
      .lexeme = strview_from("="),
      .line = 2,
    },
    (token_t) {
      .token_type = TOKEN_NUM,
      .lexeme = strview_from("2"),
      .literal = {
        .data = { .as_int = 2 },
        .type = TINT,
      },
      .line = 2,
    },
    (token_t) {
      .token_type = TOKEN_PLUS,
      .lexeme = strview_from("+"),
      .line = 2,
    },
    (token_t) {
      .token_type = TOKEN_NUM,
      .lexeme = strview_from("3"),
      .literal = {
        .data = { .as_int = 3 },
        .type = TINT,
      },
      .line = 2,
    },
    (token_t) {
      .token_type = TOKEN_SEMICOLON,
      .lexeme = strview_from(";"),
      .line = 2,
    },
    (token_t) {
      .token_type = TOKEN_RBRACE,
      .lexeme = strview_from("}"),
      .line = 3,
    },
    (token_t) {
      .token_type = TOKEN_EOF,
      .lexeme = strview_from(""),
      .line = 3,
    }
  };

  bool expected_error = false;
  cr_assert_eq(expected_error, res.had_errors);

  cr_assert_eq(res.read_tokens_amount, sizeof(expected_tokens) / sizeof(token_t));

  for(size_t i = 0; i < res.read_tokens_amount; i++)
    check_for_equal_tokens(expected_tokens[i], res.read_tokens[i]);
}

