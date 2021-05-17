#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ring-buffer.h"

#define TRUE 1
#define FALSE 0

static void
advance_pointer(ring_buf* r);

static void
retreat_pointer(ring_buf* r);

void
ring_buf_init(ring_buf* r, int capacity)
{

  r->capacity = capacity;
  r->slots = malloc(r->capacity * sizeof(message));

  pthread_mutex_init(&r->lock, NULL);
  r->full = FALSE;
  pthread_cond_init(&r->fill, NULL);
  pthread_cond_init(&r->empty, NULL);

  r->write = 0;
  r->read = 0;
}

void
ring_buf_destroy(ring_buf* r)
{
  free(r->slots);
}

int
ring_buf_empty(ring_buf* r)
{
  return !r->full && (r->read == r->write);
}

void
ring_buf_put(ring_buf* r, message m)
{
  r->slots[r->write] = m;

  advance_pointer(r);
}

message
ring_buf_read(ring_buf* r /*, message* data*/)
{
  //   int ok = -1;
  message out;

  if (!ring_buf_empty(r)) {
    out = r->slots[r->read];

    retreat_pointer(r);

    // ok = 0;
  }

  return out;

  //   return ok;
}

static void
advance_pointer(ring_buf* r)
{
  if (r->full) {
    //   TODO: Right now this just simply overwrites everything. Block?
    if (++(r->read) == r->capacity) {
      r->read = 0;
    }
  }

  if (++(r->write) == r->capacity) {
    r->write = 0;
  }

  r->full = (r->read == r->write);
}

static void
retreat_pointer(ring_buf* r)
{
  r->full = FALSE;

  if ((++r->read) == r->capacity) {
    r->read = 0;
  }

  //   r->read = (r->read + 1) % r->capacity;
}

int
ring_buf_size(ring_buf* r)
{
  if (r->full) {
    return r->capacity;
  }

  if (r->read >= r->write) {
    return r->read - r->write;
  }

  return (r->read - r->write) + r->capacity;
}

void
ring_buf_lock(ring_buf* r)
{
  int result = pthread_mutex_lock(&r->lock);
  if (result != 0) {
    perror("failed to lock mutex");
    exit(1);
  }
}

void
ring_buf_unlock(ring_buf* r)
{

  int result = pthread_mutex_unlock(&r->lock);
  if (result != 0) {
    perror("failed to unlock mutex");
    exit(1);
  }
}
