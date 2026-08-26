#include<criterion/criterion.h>
#include<criterion/assert.h>

#include "lexing.h"

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
            "Expected token of type '%s', got '%s'.",
            expected_token_type,
            actual_token_type
  );

  if(!is_literal(expected))
    return;

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
      cr_expect(str_view_equals(expected.literal.data.as_str, actual.literal.data.as_str),
                "Expected a literal value of \"" str_view_FMT "\", got \"" str_view_FMT "\".",
                str_view_ARG(expected.literal.data.as_str),
                str_view_ARG(actual.literal.data.as_str)
      );
      break;
    case TBOOL:
      cr_expect(expected.literal.data.as_bool == actual.literal.data.as_bool,
                "Expected a literal value of '%s', got '%s'.",
                (expected.literal.data.as_bool ? "true" : "false"),
                (actual.literal.data.as_bool ? "true" : "false")
      );
      break;
    default:
      cr_expect(false, "Should not get into here");
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
      .token_kind = TOKEN_FUNC,
      .lexeme = str_view_from("func"),
      .line = 1,
    },
    (token_t) {
      .token_kind = TOKEN_ID,
      .lexeme = str_view_from("main"),
      .line = 1,
    },
    (token_t) {
      .token_kind = TOKEN_LPAREN,
      .lexeme = str_view_from("("),
      .line = 1,
    },
    (token_t) {
      .token_kind = TOKEN_RPAREN,
      .lexeme = str_view_from(")"),
      .line = 1,
    },
    (token_t) {
      .token_kind = TOKEN_LBRACE,
      .lexeme = str_view_from("{"),
      .line = 1,
    },
    (token_t) {
      .token_kind = TOKEN_ID,
      .lexeme = str_view_from("x"),
      .line = 2,
    },
    (token_t) {
      .token_kind = TOKEN_COLON,
      .lexeme = str_view_from(":"),
      .line = 2,
    },
    (token_t) {
      .token_kind = TOKEN_ID,
      .lexeme = str_view_from("int"),
      .line = 2,
    },
    (token_t) {
      .token_kind = TOKEN_EQUAL,
      .lexeme = str_view_from("="),
      .line = 2,
    },
    (token_t) {
      .token_kind = TOKEN_NUM,
      .lexeme = str_view_from("2"),
      .literal = {
        .data = { .as_int = 2 },
        .type = TINT,
      },
      .line = 2,
    },
    (token_t) {
      .token_kind = TOKEN_PLUS,
      .lexeme = str_view_from("+"),
      .line = 2,
    },
    (token_t) {
      .token_kind = TOKEN_NUM,
      .lexeme = str_view_from("3"),
      .literal = {
        .data = { .as_int = 3 },
        .type = TINT,
      },
      .line = 2,
    },
    (token_t) {
      .token_kind = TOKEN_SEMICOLON,
      .lexeme = str_view_from(";"),
      .line = 2,
    },
    (token_t) {
      .token_kind = TOKEN_RBRACE,
      .lexeme = str_view_from("}"),
      .line = 3,
    },
    (token_t) {
      .token_kind = TOKEN_EOF,
      .lexeme = str_view_from(""),
      .line = 3,
    }
  };

  bool expected_error = false;
  cr_assert_eq(expected_error, res.had_errors);

  cr_assert_eq(res.read_tokens_amount, sizeof(expected_tokens) / sizeof(token_t));

  for(size_t i = 0; i < res.read_tokens_amount; i++)
    check_for_equal_tokens(expected_tokens[i], res.read_tokens[i]);
}

