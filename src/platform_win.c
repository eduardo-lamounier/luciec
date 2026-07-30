#include "platform.h"

#include<stdint.h>
#include<string.h>
#include<assert.h>

const char *file_name_from_path(const char *path) {
  assert(path != NULL);

  // Windows accepts both slash characters, so it's needed to find the
  // right-most one
  const char *a = strrchr(path, '/');
  const char *b = strrchr(path, '\\');

  // No slash in the path: the entry's name is the path itself
  if(a == NULL && b == NULL)
    return path;
  
  // The string after the last slash character is the entry's name
  return a == NULL || ((b != NULL) && (uintptr_t)b > (uintptr_t)a) ? b+1 : a+1;
}

