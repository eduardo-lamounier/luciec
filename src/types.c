#include "types.h"

void value_print(value_t value) {
  switch(value.type) {
    case TINT:
      printf("" lucie_int_FMT, value.data.as_int); break;
    case TUINT:
      printf(lucie_uint_FMT, value.data.as_uint); break;
    case TFLOAT:
      printf(lucie_float_FMT, value.data.as_float); break;
    case TDOUBLE:
      printf(lucie_double_FMT, value.data.as_double); break;
    case TLONG:
      printf(lucie_long_FMT, value.data.as_long); break;
    case TULONG:
      printf(lucie_ulong_FMT, value.data.as_ulong); break;
    case TCHAR:
      printf(lucie_char_FMT, value.data.as_char); break;
    case TBOOL:
      printf(value.data.as_bool ? "true" : "false"); break;
    case TSTR:
      printf("\"" str_view_FMT "\"", str_view_ARG(value.data.as_str)); break;
    case TNULL:
      printf("null"); break;
  };
}
