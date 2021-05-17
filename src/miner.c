#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ring-buffer.h"
#include "sha-256.h"

#define NUM_MINERS 8
#define DIFFICULTY 2 // number of bytes of leading 0's

void
ThreadCreate(pthread_t* thread,
             const pthread_attr_t* attr,
             void* (*start_routine)(void*),
             void* arg);

void
ThreadJoin(pthread_t thread, void** value_ptr);

void
Wait(pthread_cond_t* c, pthread_mutex_t* mu);

void
Signal(pthread_cond_t* c);

message
get_message(ring_buf* r);

void
send_message(ring_buf* r, message msg);

void
_print_hash(uint8_t hash[32])
{
  for (int i = 0; i < 32; i++)
    printf("%02x", hash[i]);
}

int
successful(uint8_t hash[32])
{
  for (int i = 0; i < DIFFICULTY; i++)
    if (hash[i] != 0)
      return 0;
  return 1;
}

// TODO instead of just printing, communicate the successful
// hash value back once found
void
hash_and_print(uint32_t nonce)
{
  uint8_t hash[32];
  int attempts = 0;
  uint32_t x, sum;

  do {
    attempts++;
    x = rand();
    sum = nonce + x;
    calc_sha_256(hash, (const void*)(&sum), 32);
  } while (!successful(hash));

  printf("Found summand to be %u, since sha256(%u + %u) = ", x, nonce, x);
  _print_hash(hash);
  printf(", after %d attempts.\n", attempts);
}

uint32_t
hash(uint32_t nonce)
{
  uint8_t hash[32];
  int attempts = 0;
  uint32_t x, sum;

  do {
    attempts++;
    // printf("(attempt: %d)\n", attempts);
    x = rand();
    sum = nonce + x;
    calc_sha_256(hash, (const void*)(&sum), 32);
  } while (!successful(hash));

  return x;
}

typedef struct _miner_t
{
  ring_buf* challenges;
  ring_buf* responses;
  int author;
} miner;

void*
mine(void* arg)
{
  miner* a = (miner*)arg;
  ring_buf* challenges = a->challenges;
  ring_buf* responses = a->responses;
  int author = a->author;

  while (1) {
    message challenge = get_message(challenges);
    // printf("miner '%d': got challenge %d\n", author, challenge.nonce);

    message response;
    response.answer = hash(challenge.nonce);

    response.nonce = challenge.nonce;
    response.author = author;
    // printf("miner '%d': calculated response %d for challenge %d\n",
    //        author,
    //        response.answer,
    //        response.nonce);
    send_message(responses, response);
    // printf("miner '%d': sent msg response %d for challenge %d\n",
    //        author,
    //        response.answer,
    //        response.nonce);
  }
}

message
get_message(ring_buf* r /*, message* msg*/)
{
  message msg;

  ring_buf_lock(r);
  while (ring_buf_empty(r)) {
    Wait(&r->fill, &r->lock);
  }

  //   if (ring_buf_read(r, msg) != 0) {
  //     perror("miner: unexpected read of empty buffer");
  //     exit(1);
  //   }
  msg = ring_buf_read(r);
  Signal(&r->empty);

  ring_buf_unlock(r);
  return msg;
}

void
send_message(ring_buf* r, message msg)
{

  ring_buf_lock(r);
  while (r->full) {
    Wait(&r->empty, &r->lock);
  }

  ring_buf_put(r, msg);
  Signal(&r->fill);

  ring_buf_unlock(r);
}

void
coordinate()
{
  int winners[NUM_MINERS];
  int i;

  ring_buf* challenges = malloc(sizeof(ring_buf));
  ring_buf* responses = malloc(sizeof(ring_buf));

  ring_buf_init(challenges, NUM_MINERS);
  ring_buf_init(responses, NUM_MINERS);

  for (i = 0; i < NUM_MINERS; i++) {
    winners[i] = 0;

    miner* m = malloc(sizeof(miner));
    m->author = i;
    m->challenges = challenges;
    m->responses = responses;

    pthread_t thread_id;
    ThreadCreate(&thread_id, NULL, mine, m);
  }

  while (1) {
    srand(time(NULL));
    uint32_t nonce = rand();

    message c;
    c.nonce = nonce;
    printf("coordinator: new nonce '%d'\n", nonce);

    for (i = 0; i < NUM_MINERS; i++) {
      send_message(challenges, c);
      //   printf("coordinator: sent message to miner '%d'\n", c.author);
    }

    int num_responses = 0;
    int winning_response = 0;
    while (num_responses < NUM_MINERS) {
      message resp = get_message(responses /*, resp*/);
      //   printf("coordinator: got response %d from miner '%d' for challenge
      //   %d\n",
      //          resp.answer,
      //          resp.author,
      //          resp.nonce);
      if (resp.nonce == nonce) {
        num_responses++;
        if (!winning_response) {
          winning_response = 1;
          winners[resp.author]++;

          printf("winning miner: %d\n", resp.author);
        }
      }
    }

    printf("STATS:\n");
    for (i = 0; i < NUM_MINERS; i++) {
      printf("- miner '%d': %d\n", i, winners[i]);
    }

    printf("\nstarting next round...\n");
  }
}

int
main()
{
  // TODO instead of just running this on one nonce, generate
  // a sequence of random nonces and have miners hash the nonce
  // separately in threads
  //   srand(time(NULL));
  //   uint32_t nonce = rand();

  //   printf("Mining nonce: %d\n", nonce);

  //   hash_and_print(nonce);

  coordinate();

  return 0;
}

void
ThreadCreate(pthread_t* thread,
             const pthread_attr_t* attr,
             void* (*start_routine)(void*),
             void* arg)
{
  int result = pthread_create(thread, attr, start_routine, arg);
  if (result != 0) {
    perror("failed to create thread");
    exit(1);
  }
}

void
ThreadJoin(pthread_t thread, void** value_ptr)
{
  int result = pthread_join(thread, value_ptr);
  if (result != 0) {
    perror("failed to join thread");
    exit(1);
  }
}

void
Wait(pthread_cond_t* c, pthread_mutex_t* mu)
{
  int result = pthread_cond_wait(c, mu);
  if (result != 0) {
    perror("failed to wait for condition");
    exit(1);
  }
}
void
Signal(pthread_cond_t* c)
{
  int result = pthread_cond_signal(c);
  if (result != 0) {
    perror("failed to signal condition");
    exit(1);
  }
}
