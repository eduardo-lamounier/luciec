#include "platform.h"

#include<string.h>
#include<assert.h>

const char *file_name_from_path(const char *path) {
  assert(path != NULL);

  // Substring starting in the last slash
  const char *last_slash = strrchr(path, '/');

  // No slash in the path: the entry's name is the path itself
  if(last_slash == NULL)
    return path; 

  // The string after the last slash is the entry's name
  return last_slash + 1;
}

