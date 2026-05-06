#include <stdio.h>
#include <string.h>
#include <stdint.h>

uint32_t rand(void) {
	static uint32_t x = 123456789;
	static uint32_t y = 362436069;
	static uint32_t z = 521288629;
	static uint32_t w = 88675123;

	uint32_t t;

fprintf(stderr, "state x=%08x y=%08x z=%08x w=%08x\n",
 x, y, z, w);

	t = x ^ (x << 11);
	x = y; y = z; z = w;
	return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

int main(void) {
 uint32_t x;
 int i;

 for (i = 0; i < 8; i++) {
  x = rand();
  fprintf(stderr, "rand()=%08x\n", x);
 }


 return 0;
}
