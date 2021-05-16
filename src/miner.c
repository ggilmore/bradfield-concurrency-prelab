#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ring-buffer.h"
#include "sha-256.h"

#define DIFFICULTY 2 // number of bytes of leading 0's

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

int
main()
{
  // TODO instead of just running this on one nonce, generate
  // a sequence of random nonces and have miners hash the nonce
  // separately in threads
  srand(time(NULL));
  uint32_t nonce = rand();

  printf("Mining nonce: %d\n", nonce);

  hash_and_print(nonce);

  return 0;
}
