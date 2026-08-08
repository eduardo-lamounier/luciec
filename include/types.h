#ifndef TYPES_H
#define TYPES_H

#include<stdbool.h>
#include<stdint.h>
#include<inttypes.h>

#include "vendor/string-view.h"

typedef int32_t lucie_int_t;
typedef uint32_t lucie_uint_t;
typedef int64_t lucie_long_t;
typedef uint64_t lucie_ulong_t;
typedef float lucie_float_t;
typedef double lucie_double_t;
typedef char lucie_char_t;
typedef bool lucie_bool_t;
typedef string_view_t lucie_str_t;

#define lucie_int_FMT "%" PRId32
#define lucie_uint_FMT "%" PRIu32
#define lucie_long_FMT "%" PRId64
#define lucie_ulong_FMT "%" PRIu64
#define lucie_float_FMT "%f"
#define lucie_double_FMT "%lf"
#define lucie_char_FMT "%c"
#define lucie_str_FMT str_view_FMT

#define LIST_TYPES                                                             \
  X(TINT) X(TUINT)                                                             \
  X(TLONG) X(TULONG)                                                           \
  X(TFLOAT) X(TDOUBLE)                                                         \
  X(TCHAR)                                                                     \
  X(TBOOL)                                                                     \
  X(TSTR)                                                                      \
  X(TNULL)

typedef enum {
  #define X(t) t,
  LIST_TYPES
  #undef X
} type_t;

typedef struct {
  union {
    lucie_int_t as_int;
    lucie_uint_t as_uint;
    lucie_long_t as_long;
    lucie_ulong_t as_ulong;
    lucie_float_t as_float;
    lucie_double_t as_double;
    lucie_char_t as_char;
    lucie_bool_t as_bool;
    lucie_str_t as_str;
  } data;
  type_t type;
} value_t;

#define is_type_num(t)                                                 \
  (                                                                    \
     t == TINT   || t == TUINT                                         \
  || t == TLONG  || t == TULONG                                        \
  || t == TFLOAT || t == TDOUBLE                                       \
  )

void value_print(value_t value);

#endif
