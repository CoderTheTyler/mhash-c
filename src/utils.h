#ifndef __UTILS_H_
#define __UTILS_H_

/* Generates a random 32-bit integer in the range [0,max). */
uint32_t rand_range(uint32_t max);

/* Seeds the random number generator. */
void rand_seed(uint32_t seed);

#endif
