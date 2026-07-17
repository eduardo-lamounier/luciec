#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include<stdio.h>
#include<stdbool.h>

#define STRVIEW_Fmt "%.*s"
#define STRVIEW_Arg(s) (int)(s).len, (s).data

typedef struct {
  const char *data;
  size_t len;
} string_view_t;

// Returns a string view to a section of the specified
// string - which doesn't have to be null-terminated.
//
// 'str' points to the start of the string section.
//
// 'n' represents the length of the section represented
// by the string view.
//
// All of the original string's changes will reflect in
// the view, and freeing the original string makes this
// string view invalid.
string_view_t strview_new(const char *str, const size_t n);

// Creates a string view from a null-terminated string
//
// Because the length of the string is counted, it has
// to pass through all the characters. Also because of
// this, the null terminator isn't necessary for
// subsequent operations.
//
// All the original string's changes will reflect in
// the view, and freeing the original string makes this
// string view invalid.
string_view_t strview_from(const char *str);

// Returns a string view to the characters in a substring
// of the specified string that goes from index 'begin' (inclusive)
// up to index 'end' (exclusive).
//
// 'str' points to the start of the source string.
//
// All of the original string's changes will reflect in 
// the view, and freeing the original string makes this
// string view invalid.
string_view_t strview_sub(const char *str, size_t begin, size_t end);

// Converts the string view to a int.
//
// Does NOT check for a invalid string like "hello".
// In this case 0 will be returned.
int strview_toint(string_view_t strv);

// Converts the string view to a double.
//
// Does NOT check for a invalid string like "hello".
// In this case 0.0 will be returned.
double strview_todouble(string_view_t strv);

// Returns `true` if the text in the string view 'a' and in the
// null-terminated string 'b' are the same. Returns `false` otherwise.
bool strview_equals_cstr(string_view_t a, const char *b);

// Returns 'true' if the text in the string view 'a' and in the
// string view 'b' are the same. Returns 'false' otherwise.
bool strview_equals_view(string_view_t a, string_view_t b);

#define strview_equals(A, B) _Generic((B), \
  string_view_t: strview_equals_view,      \
  char*: strview_equals_cstr,              \
  const char*: strview_equals_cstr         \
)(A, B)

#endif
