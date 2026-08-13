/* Library copied from the repository: https://github.com/eduardo-lamounier/c-utils
 * Code at the commit cd16734 [branch main].
 *
 * Updating the code requires manually copying the file from this repository
 * into here.
 */

/* @author: eduardo-lamounier
 * @date: 28/07/2026 [DD/MM/YYYY]
 *
 * A header-only library containing the implementation of memory arenas that
 * grow dynamically.
 *
 * To use it, put this before including the file:
 * #define DYNAMIC_ARENA_IMPLEMENTATION
*/

#ifndef DYNAMIC_ARENA_H
#define DYNAMIC_ARENA_H

#include<stdlib.h>

#define KB(x) (   (x) * (size_t)1024 )
#define MB(x) ( KB(x) * (size_t)1024 )
#define GB(x) ( MB(x) * (size_t)1024 )

typedef struct arena dynamic_arena_t;

// Creates a new arena and returns a pointer to it.
//
// The arena is initialized with the specified amount of bytes, but if it isn't
// possible to allocate that memory, NULL is returned.
//
// The arena must be destroyed after use with 'dy_arena_destroy'.
//
// Also returns NULL if 'capacity' is zero.
dynamic_arena_t *dy_arena_new(size_t capacity);

// Releases all resources from an arena, freeing everything that was allocated
// in it.
//
// Receives a pointer to the arena's pointer (aka an arena's pointer passed by
// reference), making the pointer to the arena NULL - as the arena is now
// unavailable.
void dy_arena_destroy(dynamic_arena_t **arena);

// Doesn't destroy the arena, instead, ignores everything once allocated in it
// and acts like a brand-new empty arena with the same capacity. Still needs to
// be destroyed with 'dy_arena_destroy'.
void dy_arena_reset(dynamic_arena_t *arena);

// Allocates memory for the specified 'n' amount of elements of the specified
// size each. Returns a pointer to the beginning of the allocated memory.
//
// It's guaranteed that the allocated memory will be completely zero-filled.
//
// If the arena is full or doesn't have enough room for this amount of memory,
// tries to allocate more memory for the arena. If it isn't possible to allocate
// more memory, returns NULL.
//
// Also returns NULL if either 'n' or 'size' are zero.
//
// Also keep in mind that the pointer returned becomes invalid after reseting
// or destroying the arena.
void *dy_arena_alloc(dynamic_arena_t *arena, size_t n, size_t size);

// Returns the total amount of allocated bytes by this arena in its lifetime.
size_t dy_arena_capacity(dynamic_arena_t *arena);

#endif

#ifdef DYNAMIC_ARENA_IMPLEMENTATION

#include<assert.h>
#include<stdint.h>

#define MEMORY_ALIGNMENT 8

struct arena {
  struct arena *next_arena;
  char *data;
  size_t capacity;
  size_t offset;
};

inline static size_t aligned_offset(size_t offset) {
  return (offset + MEMORY_ALIGNMENT-1) & ~(MEMORY_ALIGNMENT-1);
}

dynamic_arena_t *dy_arena_new(size_t capacity) {
  if(capacity == 0)
    return NULL;

  dynamic_arena_t *arena = (dynamic_arena_t*)malloc(sizeof(dynamic_arena_t));

  if(arena == NULL)
    return NULL;

  arena->next_arena = NULL;
  arena->offset = 0;
  arena->capacity = capacity;
  arena->data = (char*)calloc(capacity, 1);

  if(arena->data == NULL) {
    free(arena);
    return NULL;
  }

  return arena;
}

void dy_arena_destroy(dynamic_arena_t **arena) {
  assert(arena != NULL && *arena != NULL);

  dynamic_arena_t *current = *arena;
  while(current != NULL) {
    dynamic_arena_t *next_arena = current->next_arena;
    free(current->data);
    free(current);

    current = next_arena;
  }
  *arena = NULL;
}

void dy_arena_reset(dynamic_arena_t *arena) {
  assert(arena != NULL);

  dynamic_arena_t *current = arena;
  while(current != NULL) {
    current->offset = 0;
    current = current->next_arena;
  }
}

void *dy_arena_alloc(dynamic_arena_t *arena, size_t n, size_t size) {
  assert(arena != NULL);
  assert(n != 0 && size != 0);

  dynamic_arena_t *current = arena;
  while(current != NULL) {
    current->offset = aligned_offset(current->offset);

    if(current->offset + n * size < current->capacity)
      break;

    current = current->next_arena;
  }

  if(current == NULL)
    current = dy_arena_new(arena->capacity);

  if(current == NULL)
    return NULL;

  void *addr = current->data + current->offset;
  current->offset += n * size;
  return addr;
}

size_t dy_arena_capacity(dynamic_arena_t *arena) {
  assert(arena != NULL);

  size_t count = 0;
  dynamic_arena_t *current = arena;
  while(current != NULL) {
    count += current->capacity;
    current = current->next_arena;
  }

  return count;
}

#endif
