#include "utils/string_view.h"

#include<string.h>
#include<stdlib.h>

string_view_t strview_new(const char *str, const size_t n) {
  return (string_view_t) {
    .data = str,
    .len = n,
  };
}

string_view_t strview_from(const char *str) {
  return strview_new(str, strlen(str));
}

string_view_t strview_fromsubs(const char *str, size_t l, size_t r) {
  return strview_new(str + l, r - l);
}

int strview_toint(string_view_t strv) {
  if(strv.data == NULL || strv.len == 0)
    return 0;

  char temp[strv.len+1];

  memcpy(temp, strv.data, strv.len);
  temp[strv.len] = '\0';

  return atoi(temp);
}

double strview_todouble(string_view_t strv) {
  if(strv.data == NULL || strv.len == 0)
    return 0.0;

  char temp[strv.len+1];

  memcpy(temp, strv.data, strv.len);
  temp[strv.len] = '\0';

  return atof(temp);
}

bool strview_equals_view(string_view_t a, string_view_t b) {
  if(a.len != b.len)
    return false;

  for(size_t i = 0; i < a.len; i++)
    if(a.data[i] != b.data[i])
      return false;

  return true;
}

bool strview_equals_cstr(string_view_t a, const char *b) {
  if(strlen(b) != a.len)
    return false;

  for(size_t i = 0; i < a.len; i++)
    if(a.data[i] != b[i])
      return false;

  return true;
}

