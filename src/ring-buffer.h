#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

typedef struct _message_t
{
  int author;
  uint8_t data;

  int shutdown;
} message;

typedef struct _ring_buf_t
{
  message* slots;

  int capacity;

  int read;
  int write;
  int full;
} ring_buf;

ring_buf
ring_buf_init(int capacity);

void
ring_buf_destroy(ring_buf* r);

int
ring_buf_empty(ring_buf* r);

void
ring_buf_put(ring_buf* r, message t);

int
ring_buf_read(ring_buf* r, message* data);

int
ring_buf_size(ring_buf* r);
