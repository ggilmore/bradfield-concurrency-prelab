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

ring_buf
ring_buf_init(int capacity)
{
  ring_buf out;

  out.capacity = capacity;
  out.slots = malloc(out.capacity * sizeof(message));
  out.full = FALSE;

  out.write = 0;
  out.read = 0;

  return out;
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

int
ring_buf_read(ring_buf* r, message* data)
{
  int ok = -1;

  if (ring_buf_empty(r)) {
    *data = r->slots[r->read];
    retreat_pointer(r);

    ok = 0;
  }

  return ok;
}

static void
advance_pointer(ring_buf* r)
{
  if (r->full) {
    //   TODO: Right now this just simply overwrites everything. Block?
    r->read = (r->read + 1) % r->capacity;
  }

  r->write = (r->write + 1) % r->capacity;
  r->full = (r->read == r->write);
}

static void
retreat_pointer(ring_buf* r)
{
  r->full = FALSE;

  r->read = (r->read + 1) % r->capacity;
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
