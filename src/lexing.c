#define STRING_VIEW_IMPLEMENTATION
#include "lexing.h"

#include<string.h>
#include <ctype.h>
#include <stdlib.h>

#include "logging.h"
#define DYNAMIC_ARENA_IMPLEMENTATION
#include "vendor/dynamic-arena.h"

// Stores the lexemes for all non-literals
//
// Lexemes for literals are NULL
const char *token_lexemes[] = {
  [TOKEN_LPAREN] = "(", [TOKEN_RPAREN] = ")",
  [TOKEN_LBRACE] = "{", [TOKEN_RBRACE] = "}",
  [TOKEN_COMMA] = ",", [TOKEN_SEMICOLON] = ";", [TOKEN_COLON] = ":",
  [TOKEN_PLUS] = "+", [TOKEN_MINUS] = "-", [TOKEN_STAR] = "*", [TOKEN_SLASH] = "/",

  [TOKEN_AMP] = "&", [TOKEN_PIPE] = "|",
  [TOKEN_BANG] = "!", [TOKEN_EQUAL] = "=",
  [TOKEN_LESS] = "<", [TOKEN_GREATER] = ">",

  [TOKEN_AMP_AMP] = "&&", [TOKEN_PIPE_PIPE] = "||",
  [TOKEN_BANG_EQUAL] = "!=", [TOKEN_EQUAL_EQUAL] = "==",
  [TOKEN_LESS_EQUAL] = "<=", [TOKEN_GREATER_EQUAL] = ">=",
  [TOKEN_EQUAL_GREATER] = "=>",

  // Literals skipped

  [TOKEN_NULL] = "null",
  [TOKEN_FALSE] = "false",
  [TOKEN_TRUE] = "true",
  [TOKEN_IF] = "if",
  [TOKEN_ELSE] = "else",
  [TOKEN_WHILE] = "while",
  [TOKEN_FOR] = "for",
  [TOKEN_FUNC] = "func",
  [TOKEN_RETURN] = "return",
  [TOKEN_USING] = "using",

  [TOKEN_EOF] = "<EOF>",
};

struct lexer {
  dynamic_arena_t *arena;

  bool had_errors;
  const char *source;
  size_t source_size;
  size_t current;
  size_t current_line;

  size_t read_tokens_amount;
  size_t capacity; // capacity for the tokens array
  token_t *read_tokens;
};

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
// hold the new token. Exits the program (with 'error()') if the
// reallocation fails.
//
// The current capacity of the tokens array is passed by reference
// with 'capacity_for_tokens' and will change if the array is
// reallocated.
static void add_token(lexer_t *lexer,
                      const token_t token) {
  // expands if necessary
  if(lexer->read_tokens_amount + 1 > lexer->capacity) {
    token_t *temp = lexer->read_tokens;
    lexer->capacity *= 1.5;
    lexer->read_tokens = (token_t *)dy_arena_alloc(
      lexer->arena, lexer->capacity, sizeof(token_t)
    );

    memcpy(lexer->read_tokens, temp, lexer->capacity * sizeof(token_t));

    if (lexer->read_tokens == NULL)
      error(MEMORY_ALLOCATION_ERRMSG);
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
  if(lexer->source[lexeme_start] != '\"')
    return false;

  advance(lexer);
  for(; peek(lexer) != '\"' && peek(lexer) != '\0'; advance(lexer))
    if(peek(lexer) == '\n')
      lexer->current_line++;

  if(peek(lexer) == '\0') {
    lexer->had_errors = true;
    report_at(token->line, "The string literal wasn't closed.\n");
    return true;
  }

  token->token_kind = TOKEN_STR;
  token->literal = (value_t){
      .type = TSTR,
      .data.as_str =
          str_view_new(lexeme + 1, lexer->current - lexeme_start + 1 - 2),
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

  token->token_kind = TOKEN_NUM;
  for (; isdigit(peek(lexer)); advance(lexer));

  bool is_number_decimal = false;
  if (peek(lexer) == '.' && isdigit(peek_after(lexer, 1))) {
    is_number_decimal = true;
    advance(lexer);
    for (; isdigit(peek(lexer)); advance(lexer));
  }

  string_view_t lexeme_view = str_view_new(lexeme, lexer->current - lexeme_start);

  if (is_number_decimal)
    token->literal = (value_t){
        .type = TDOUBLE,
        .data = {.as_double = str_view_todouble(lexeme_view)},
    };
  else {
    lucie_int_t as_int = str_view_toint32(lexeme_view);
    lucie_long_t as_long = str_view_toint64(lexeme_view);

    if(as_long != as_int) {
      // Overflow of the int, literal should be a long
      token->literal = (value_t) {
        .type = TLONG,
        .data = { .as_long = as_long },
      };
    } else {
      token->literal = (value_t) {
        .type = TINT,
        .data = { .as_int = as_int },
      };
    }
  }

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
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_NULL])) {
    token->token_kind = TOKEN_NULL; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_FALSE])) {
    token->token_kind = TOKEN_FALSE; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_TRUE])) {
    token->token_kind = TOKEN_TRUE; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_IF])) {
    token->token_kind = TOKEN_IF; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_ELSE])) {
    token->token_kind = TOKEN_ELSE; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_WHILE])) {
    token->token_kind = TOKEN_WHILE; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_FOR])) {
    token->token_kind = TOKEN_FOR; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_FUNC])) {
    token->token_kind = TOKEN_FUNC; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_RETURN])) {
    token->token_kind = TOKEN_RETURN; return true;
  }
  if (str_view_equals_cstr(lexeme_view, token_lexemes[TOKEN_USING])) {
    token->token_kind = TOKEN_USING; return true;
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
      token.token_kind = TOKEN_LPAREN; break;
    case ')':
      token.token_kind = TOKEN_RPAREN; break;
    case '{':
      token.token_kind = TOKEN_LBRACE; break;
    case '}':
      token.token_kind = TOKEN_RBRACE; break;
    case ',':
      token.token_kind = TOKEN_COMMA; break;
    case ';':
      token.token_kind = TOKEN_SEMICOLON; break;
    case ':':
      token.token_kind = TOKEN_COLON; break;
    case '+':
      token.token_kind = TOKEN_PLUS; break;
    case '-':
      token.token_kind = TOKEN_MINUS; break;
    case '*':
      token.token_kind = TOKEN_STAR; break;
    case '/':
      token.token_kind = TOKEN_SLASH; break;
    case '&':
      if(peek(lexer) != '&') {
        token.token_kind = TOKEN_AMP; break;
      }

      advance(lexer);
      token.token_kind = TOKEN_AMP_AMP; break;
    case '|':
      if(peek(lexer) != '|') {
        token.token_kind = TOKEN_PIPE; break;
      }

      advance(lexer);
      token.token_kind = TOKEN_PIPE_PIPE; break;
    case '!':
      if(peek(lexer) != '=') {
        token.token_kind = TOKEN_BANG; break;
      }

      advance(lexer);
      token.token_kind = TOKEN_BANG_EQUAL; break;
    case '=':
      switch(peek(lexer)) {
        case '=':
          advance(lexer);
          token.token_kind = TOKEN_EQUAL_EQUAL; break;
        case '>':
          advance(lexer);
          token.token_kind = TOKEN_EQUAL_GREATER; break;
        default:
          token.token_kind = TOKEN_EQUAL; break;
      }
      break;
    case '<':
      if(peek(lexer) != '=') {
        token.token_kind = TOKEN_LESS; break;
      }

      advance(lexer);
      token.token_kind = TOKEN_LESS_EQUAL; break;
    case '>':
      if(peek(lexer) != '=') {
        token.token_kind = TOKEN_GREATER; break;
      }

      advance(lexer);
      token.token_kind = TOKEN_GREATER_EQUAL; break;
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

      token.token_kind = TOKEN_ID;

      if(check_for_keywords(
        &token, 
        str_view_new(lexeme, lexer->current - start))
      )
        break;

      // If the lexeme doesn't match with any keyword, it's an indentifier
  }

  token.lexeme = str_view_new(lexeme, lexer->current - start);
  *token_out = token;
}

bool lexer_had_errors(const lexer_t *lexer) {
  return lexer->had_errors;
}

const token_t *lexer_tokens(const lexer_t *lexer) {
  return lexer->read_tokens;
}

size_t lexer_tokens_amount(const lexer_t *lexer) {
  return lexer->read_tokens_amount;
}

lexer_t *lexer_new(const char *source, size_t source_size) {
  assert(source != NULL);

  lexer_t *lexer = calloc(1, sizeof(lexer_t)); 

  if(lexer == NULL)
    return NULL;

  *lexer = (lexer_t) {
    .arena = dy_arena_new(256 * sizeof(token_t)),
    .source = source,
    .source_size = source_size,
    .current_line = 1,
    .read_tokens = (token_t *)malloc(2 * sizeof(token_t)),
    .capacity = 2, // MUST be 2 (or greater), otherwise `capacity *= 1.5`
                   // might not expand the array when necessary
  };

  if(lexer->arena == NULL) {
    free(lexer);
    return NULL;
  }

  return lexer;
}

void lexer_destroy(lexer_t *lexer) {
  dy_arena_destroy(&lexer->arena);
  free(lexer);
}

void lexer_scan_source(lexer_t *lexer) {
  if (lexer->read_tokens == NULL)
    error(MEMORY_ALLOCATION_ERRMSG);

  bool in_comment_block = false;
  for (; peek(lexer) != '\0';) {
    // New line
    if(peek(lexer) == '\n') {
      advance(lexer);
      lexer->current_line++;
      continue;
    }

    // Skip whitespaces:
    if(peek(lexer) == ' ' || peek(lexer) == '\t'
      || peek(lexer) == '\r') {
      advance(lexer);
      continue;
    }

    if (strcmp(peek_ptr(lexer), "*/") == 0) {
      if (!in_comment_block) {
        report_at(lexer->current_line,"'*/' doesn't have a corresponding '/*'.\n");
        lexer->had_errors = true;
      }

      in_comment_block = false;
      advance_by(lexer, 2);
      continue;
    }

    if (strcmp(peek_ptr(lexer), "//") == 0) {
      if(in_comment_block) {
        warn_at(lexer->current_line, "'//' inside a comment block. "
                "Did you mean to close the block with '*/'?\n");
        advance_by(lexer, 2);
        continue;
      }

      for(; peek(lexer) != '\n' && peek(lexer) != '\0';
        advance(lexer));
      continue;
    }

    if (strcmp(peek_ptr(lexer), "/*") == 0) {
      if (in_comment_block)
        warn_at(lexer->current_line, "'/*' inside a comment block. "
                "Did you mean to close it with '*/'?\n");

      in_comment_block = true;
      advance_by(lexer, 2);
      continue;
    }

    if(in_comment_block) {
      advance(lexer);
      continue;
    }

    token_t token;
    scan_token(lexer, &token);
    add_token(lexer, token);
  }

  token_t EOF_token = {
    .token_kind = TOKEN_EOF,
    .lexeme = str_view_from(token_lexemes[TOKEN_EOF]),
    .line = lexer->current_line
  };

  add_token(lexer, EOF_token);
}

