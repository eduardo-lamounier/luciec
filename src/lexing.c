#include "lexing.h"

#include "logging.h"
#include "utils/string_view.h"
#include <ctype.h>
#include <stdlib.h>

typedef struct {
  bool had_errors;
  const char *const source;
  size_t source_size;
  size_t current;
  size_t current_line;
  size_t read_tokens_amount;
  token_t *read_tokens;
} lexer_t;

// Gets the character in the current cursor position
static inline char peek(const lexer_t *const lexer) {
  return lexer->source[lexer->current];
}

// Gets the character in a position after the cursor
//
// The position is the current cursor position incremented by 'n'
//
// If this position passes the source's size, '\0' is returned
static inline char peek_after(const lexer_t *const lexer, const size_t n) {
  return lexer->current + n >= lexer->source_size ?
    '\0' : lexer->source[lexer->current + n];
}

// Gets the pointer to the character in the current cursor position
static inline const char *peek_ptr(const lexer_t *const lexer) {
  return lexer->source + lexer->current;
}

// Advances the current cursor position by one
static inline void advance(lexer_t *const lexer) { lexer->current++; }

// Advances the current cursor position by some specified number
static inline void advance_by(lexer_t *const lexer, const size_t n) {
  lexer->current += n;
}

// Adds the token to the vector containing all the read tokens, stored
// by the lexer.
//
// Reallocates the read tokens array if necessary to make sure it can
// hold the new token. Exits the program (with 'throw_error') if the
// reallocation fails.
//
// The current capacity of the tokens array is passed by reference
// with 'capacity_for_tokens' and will change if the array is
// reallocated.
static void add_token(lexer_t *const lexer,
                      const token_t token,
                      size_t *const capacity_for_tokens) {
  // expands if necessary
  if(lexer->read_tokens_amount + 1 > *capacity_for_tokens) {
    *capacity_for_tokens *= 1.5;
    lexer->read_tokens = (token_t *)realloc(
      lexer->read_tokens,*capacity_for_tokens * sizeof(token_t)
    );

    if (lexer->read_tokens == NULL)
      throw_error(MEMORY_ALLOCATION_ERRMSG);
  }

  lexer->read_tokens[lexer->read_tokens_amount++] = token;
}

// Checks whether the token - passed by reference - in the lexer's cursor
// current position is a string literal. Returns true if it is and false
// otherwise.
//
// If it's a string literal, the token's info is updated.
//
// The C-string to the lexeme and its start position in the source must
// be specified.
static bool check_for_string_literal(token_t *const token, const char *lexeme,
                                     const size_t lexeme_start,
                                     lexer_t *const lexer) {
  if(peek(lexer) != '\"')
    return false;

  advance(lexer);
  for(; peek(lexer) != '\"' && peek(lexer) != '\0'; advance(lexer))
    if(peek(lexer) == '\n')
      lexer->current_line++;

  if(peek(lexer) == '\0') {
    snprintf(log_msg_buff, LOG_MESSAGE_BUFFER_SIZE, 
             "The string literal wasn't closed."
             "At line %zu.", token->line);
    lexer->had_errors = true;
    report_error(log_msg_buff);
    return true;
  }

  token->token_type = TOKEN_STR;
  token->literal = (value_t){
      .type = TSTR,
      .data.as_str =
          strview_new(lexeme + 1, lexer->current - lexeme_start + 1 - 2),
  };
  advance(lexer);
  return true;
}

// Checks whether the token - passed by reference - in the lexer's cursor
// current position is a number literal. Returns true if it is and false
// otherwise.
//
// If it's a number literal, the token's info is updated.
//
// A C-string to the lexeme and its start position in the source must
// be specified
static bool check_for_number_literal(token_t *const token, char current_chr,
                                     const char *lexeme, size_t lexeme_start,
                                     lexer_t *const lexer) {
  if(!isdigit(current_chr))
    return false;

  token->token_type = TOKEN_NUM;
  for (; isdigit(peek(lexer)); advance(lexer));

  bool is_number_decimal = false;
  if (peek(lexer) == '.' && isdigit(peek_after(lexer, 1))) {
    is_number_decimal = true;
    advance(lexer);
    for (; isdigit(peek(lexer)); advance(lexer));
  }

  string_view_t lexeme_view = strview_new(lexeme, lexer->current - lexeme_start);

  if (is_number_decimal)
    token->literal = (value_t){
        .type = TDOUBLE,
        .data = {.as_double = strview_todouble(lexeme_view)},
    };
  else
    token->literal = (value_t){
        .type = TINT,
        .data = {.as_int = strview_toint(lexeme_view)},
    };

  return true;
}

// Checks whether the token - passed by reference - in the lexer's cursor
// current position matches any keyword. Returns true if it does and false
// otherwise.
//
// If it matches a keyword, the token's info is updated.
//
// A string view to the token's lexeme must be specified.
static bool check_for_keywords(token_t *const token, string_view_t lexeme_view) {
  if (strview_equals(lexeme_view, "null")) {
    token->token_type = TOKEN_NULL; return true;
  }
  if (strview_equals(lexeme_view, "false")) {
    token->token_type = TOKEN_FALSE; return true;
  }
  if (strview_equals(lexeme_view, "true")) {
    token->token_type = TOKEN_TRUE; return true;
  }
  if (strview_equals(lexeme_view, "if")) {
    token->token_type = TOKEN_IF; return true;
  }
  if (strview_equals(lexeme_view, "else")) {
    token->token_type = TOKEN_ELSE; return true;
  }
  if (strview_equals(lexeme_view, "while")) {
    token->token_type = TOKEN_WHILE; return true;
  }
  if (strview_equals(lexeme_view, "for")) {
    token->token_type = TOKEN_FOR; return true;
  }
  if (strview_equals(lexeme_view, "func")) {
    token->token_type = TOKEN_FUNC; return true;
  }
  if (strview_equals(lexeme_view, "return")) {
    token->token_type = TOKEN_RETURN; return true;
  }
  if (strview_equals(lexeme_view, "using")) {
    token->token_type = TOKEN_USING; return true;
  }

  return false;
}

// Reads the token starting from the lexer's current cursor position
//
// The value pointed by 'token_out' gets the information
// of the read token.
//
// Moves the lexer's current position when reading the source.
static void scan_token(lexer_t *const lexer, token_t *const token_out) {
  token_t token = {
      .line = lexer->current_line,
  };

  char current_chr = peek(lexer);

  const char *const lexeme = peek_ptr(lexer);
  size_t start = lexer->current;

  advance(lexer);

  switch (current_chr) {
    case '(':
      token.token_type = TOKEN_LPAREN; break;
    case ')':
      token.token_type = TOKEN_RPAREN; break;
    case '{':
      token.token_type = TOKEN_LBRACE; break;
    case '}':
      token.token_type = TOKEN_RBRACE; break;
    case ',':
      token.token_type = TOKEN_COMMA; break;
    case ';':
      token.token_type = TOKEN_SEMICOLON; break;
    case ':':
      token.token_type = TOKEN_COLON; break;
    case '+':
      token.token_type = TOKEN_PLUS; break;
    case '-':
      token.token_type = TOKEN_MINUS; break;
    case '*':
      token.token_type = TOKEN_STAR; break;
    case '/':
      token.token_type = TOKEN_SLASH; break;
    case '&':
      if(peek(lexer) != '&') {
        token.token_type = TOKEN_AMP; break;
      }

      advance(lexer);
      token.token_type = TOKEN_AMP_AMP; break;
    case '|':
      if(peek(lexer) != '|') {
        token.token_type = TOKEN_PIPE; break;
      }

      advance(lexer);
      token.token_type = TOKEN_PIPE_PIPE; break;
    case '!':
      if(peek(lexer) != '=') {
        token.token_type = TOKEN_BANG; break;
      }

      advance(lexer);
      token.token_type = TOKEN_BANG_EQUAL; break;
    case '=':
      switch(peek(lexer)) {
        case '=':
          advance(lexer);
          token.token_type = TOKEN_EQUAL_EQUAL; break;
        case '>':
          advance(lexer);
          token.token_type = TOKEN_EQUAL_GREATER; break;
        default:
          token.token_type = TOKEN_EQUAL; break;
      }
      break;
    case '<':
      if(peek(lexer) != '=') {
        token.token_type = TOKEN_LESS; break;
      }

      advance(lexer);
      token.token_type = TOKEN_LESS_EQUAL; break;
    case '>':
      if(peek(lexer) != '=') {
        token.token_type = TOKEN_GREATER; break;
      }

      advance(lexer);
      token.token_type = TOKEN_GREATER_EQUAL; break;
    default:
      if(check_for_string_literal(&token, lexeme, start, lexer))
         break;

      // Number literal
      if(check_for_number_literal(&token, current_chr, lexeme,
                                  start, lexer))
        break;

      // Indentifier
      if(isalpha(peek(lexer))) {
        advance(lexer);
        for(; isalnum(peek(lexer)); advance(lexer));
      }

      token.token_type = TOKEN_ID;

      if(check_for_keywords(
        &token, 
        strview_new(lexeme, lexer->current - start))
      )
        break;

      // If the lexeme doesn't match with any keyword, it's an indentifier
  }

  token.lexeme = strview_new(lexeme, lexer->current - start);
  *token_out = token;
}

tokenized_source_t tokenize_source(const char *const source, const size_t source_size) {
  size_t capacity = 2; // MUST be 2 (or greater), otherwise `capacity *= 1.5`
                       // might not expand the array when necessary
  lexer_t lexer = {
    .had_errors = false,
    .source = source,
    .source_size = source_size,
    .current = 0,
    .current_line = 1,
    .read_tokens = (token_t *)malloc(capacity * sizeof(token_t)),
    .read_tokens_amount = 0
  };

  if (lexer.read_tokens == NULL)
    throw_error(MEMORY_ALLOCATION_ERRMSG);

  bool in_comment_block = false;
  for (; peek(&lexer) != '\0';) {
    // New line
    if(peek(&lexer) == '\n') {
      advance(&lexer);
      lexer.current_line++;
      continue;
    }

    // Skip whitespaces:
    if(peek(&lexer) == ' ' || peek(&lexer) == '\t'
      || peek(&lexer) == '\r') {
      advance(&lexer);
      continue;
    }

    if (strcmp(peek_ptr(&lexer), "*/") == 0) {
      if (!in_comment_block) {
        snprintf(log_msg_buff, LOG_MESSAGE_BUFFER_SIZE,
                 "'*/' doesn't have a correspondent '/*'.\n"
                 "In line: %zu\n\n",
                 lexer.current_line);
        report_error(log_msg_buff);
        lexer.had_errors = true;
      }

      in_comment_block = false;
      advance_by(&lexer, 2);
      continue;
    }

    if (strcmp(peek_ptr(&lexer), "//") == 0) {
      if(in_comment_block) {
        snprintf(log_msg_buff, LOG_MESSAGE_BUFFER_SIZE,
                 "'//' inside a comment block. Did you mean to write '*/'?\n"
                 "In line: %zu\n\n", lexer.current_line);
        warn(log_msg_buff);
        advance_by(&lexer, 2);
        continue;
      }

      for(; peek(&lexer) != '\n' && peek(&lexer) != '\0';
        advance(&lexer));
      continue;
    }

    if (strcmp(peek_ptr(&lexer), "/*") == 0) {
      if (in_comment_block) {
        snprintf(log_msg_buff, LOG_MESSAGE_BUFFER_SIZE,
                 "'/*' inside a comment block. Did you mean to close it?\n"
                 "In line: %zu\n\n",
                 lexer.current_line);
        warn(log_msg_buff);
      }

      in_comment_block = true;
      advance_by(&lexer, 2);
      continue;
    }

    if(in_comment_block) {
      advance(&lexer);
      continue;
    }

    token_t token;
    scan_token(&lexer, &token);
    add_token(&lexer, token, &capacity);
  }

  token_t EOF_token = {
    .token_type = TOKEN_EOF,
    .lexeme = strview_from(""),
    .line = lexer.current_line
  };

  add_token(&lexer, EOF_token, &capacity);

  return (tokenized_source_t) {
    .read_tokens = lexer.read_tokens,
    .read_tokens_amount = lexer.read_tokens_amount,
    .had_errors = lexer.had_errors
  };
}

