/* Library copied from the repository: https://github.com/eduardo-lamounier/c-utils
 * Code at the commit e6862a3 [branch main].
 *
 * Updating the code requires manually copying the file from this repository
 * into here.
 */

/* @author: eduardo-lamounier
 * @date: 25/07/2026 [DD/MM/YYYY]
 *
 * A header-only library containing the implementation of string views.
 *
 * To use it, put before including the file:
 * #define STRING_VIEW_IMPLEMENTATION
*/

#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include<stdio.h>
#include<stdbool.h>

#define str_view_FMT "%.*s"
#define str_view_ARG(v) (int)(v).length, (v).data

typedef struct {
  const char *data;
  size_t length;
} string_view_t;

// Creates a string view to an empty string literal
#define str_view_empty() str_view_from("")

// Trims spaces from left and right of the view
#define str_view_trim(v) do {                                                  \
  str_view_trim_left(v);                                                       \
  str_view_trim_right(v);                                                      \
} while(0)

// Creates a string view to the start of a string up to a specified amount of
// characters.
//
// All changes in the original string reflect on the view.
string_view_t str_view_new(const char *str, size_t n);

// Creates a string view to the entirety of a null-terminated string.
//
// All changes in the original string reflect on the view.
string_view_t str_view_from(const char *cstr);

// Returns a string view to a section of the specified input view. Goes from
// 'left' (inclusive) to right (exclusive).
string_view_t str_view_slice(string_view_t view, size_t left, size_t right);

// Removes all spaces to the left of a string view, not from the original string
// but only the view.
void str_view_trim_left(string_view_t *view);

// Removes all spaces to the right of a string view, not from the original string
// but only the view.
void str_view_trim_right(string_view_t *view);

// Converts the content of a string view to an integer.
//
// If the string is invalid (e.g. "hello"), 0 is returned.
int str_view_toint(string_view_t view);

// Converts the content of a string view to a double.
//
// If the string is invalid (e.g. "hello"), 0.0 is returned.
double str_view_todouble(string_view_t view);

// Checks whether the content of two string views are the same - in this case
// returning `true`, otherwise `false`.
bool str_view_equals(string_view_t view1, string_view_t view2);

// Checks whether the content of a string view and a null-terminated string
// are the same - in this case returning `true`, otherwise `false`.
bool str_view_equals_cstr(string_view_t view, const char *cstr);


#endif


#ifdef STRING_VIEW_IMPLEMENTATION

#include<assert.h>
#include<string.h>
#include<stdlib.h>

inline static void chop_left(string_view_t *view, size_t n) {
  assert(view != NULL && n <= view->length && view->data != NULL);
  assert(view->data != NULL);
  view->data += n;
  view->length -= n;
}

inline static void chop_right(string_view_t *view, size_t n) {
  assert(view != NULL && n <= view->length && view->data != NULL);
  view->length -= n;
}



string_view_t str_view_new(const char *str, size_t n) {
  assert(str != NULL);

  string_view_t view = {
    .data = str,
    .length = n,
  };

  return view;
}

string_view_t str_view_from(const char *cstr) {
  assert(cstr != NULL);
  return str_view_new(cstr, strlen(cstr));
}

string_view_t str_view_slice(string_view_t view, size_t left, size_t right) {
  assert(left < view.length && right <= view.length);
  assert(view.data != NULL && view.data != NULL);

  string_view_t res = str_view_new(view.data, view.length);

  chop_left(&res, left);
  chop_right(&res, view.length - right);

  return res;
}

void str_view_trim_left(string_view_t *view) {
  assert(view != NULL && view->data != NULL);
  size_t count = 0;
  for(size_t i = 0; i < view->length && view->data[i] == ' ';
      i++, count++);
  chop_left(view, count);
}

void str_view_trim_right(string_view_t *view) {
  assert(view != NULL && view->data != NULL);
  size_t count = 0;
  for(size_t i = view->length; i > 0 && view->data[i-1] == ' ';
      i--, count++);
  chop_right(view, count);
}

int str_view_toint(string_view_t view) {
  if(view.data == NULL || view.length == 0)
    return 0;

  char temp[view.length+1];
  memcpy(temp, view.data, view.length);
  temp[view.length] = '\0';

  return atoi(temp);
}

double str_view_todouble(string_view_t view) {
  if(view.data == NULL || view.length == 0)
    return 0.0;

  char temp[view.length+1];
  memcpy(temp, view.data, view.length);
  temp[view.length] = '\0';

  return atof(temp);
}

bool str_view_equals(string_view_t view1, string_view_t view2) {
  if(view1.length != view2.length)
    return false;

  return strncmp(view1.data, view2.data, view1.length) == 0;
}

bool str_view_equals_cstr(string_view_t view, const char *cstr) {
  if(view.length != strlen(cstr))
    return false;

  return strncmp(view.data, cstr, view.length) == 0;
}

#endif
