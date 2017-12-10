#include <stdlib.h>
#include <stdint.h>
#include "utils.h"

/* Taken from
 * http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range/6852396#6852396
 */
uint32_t rand_range(uint32_t max) {
	uint64_t
	        num_bins = max+1,
	        num_rand = (uint64_t)RAND_MAX+1,
	        bin_size = num_rand / num_bins,
	        defect = num_rand % num_bins;
	int64_t x;
	do {
		x = random();
	}while(num_rand - defect <= (uint64_t) x);
	return x / bin_size;
}

/* Seeds the random number generator */
void rand_seed(uint32_t seed) {
	srand(seed);
}
