#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

typedef struct _message_t
{
  int author;

  uint32_t nonce;
  uint32_t answer;

  int shutdown;
} message;

typedef struct _ring_buf_t
{
  pthread_mutex_t lock;
  pthread_cond_t empty;
  pthread_cond_t fill;

  message* slots;

  int capacity;

  int read;
  int write;
  int full;
} ring_buf;

void
ring_buf_init(ring_buf* r, int capacity);

void
ring_buf_destroy(ring_buf* r);

int
ring_buf_empty(ring_buf* r);

void
ring_buf_put(ring_buf* r, message t);

// int
// ring_buf_read(ring_buf* r, message* data);

message
ring_buf_read(ring_buf* r);

int
ring_buf_size(ring_buf* r);

void
ring_buf_lock(ring_buf* r);

void
ring_buf_unlock(ring_buf* r);
