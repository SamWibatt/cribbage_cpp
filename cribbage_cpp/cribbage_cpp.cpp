#include "cribbage_cpp.h"

namespace cribbage_cpp {
    //was static, dunno if C++11 needs it in a namespace
    uint32_t next = 1;


    int32_t do_random(uint32_t *ctx)
    {
        // * Compute x = (7^5 * x) mod (2^31 - 1)
        // * wihout overflowing 31 bits:
        // *      (2^31 - 1) = 127773 * (7^5) + 2836
        // * From "Random number generators: good ones are hard to find",
        // * Park and Miller, Communications of the ACM, vol. 31, no. 10,
        // * October 1988, p. 1195.
        int32_t hi, lo, x;

        x = *ctx;
        // Can't be initialized with 0, so use another value.
        if (x == 0)
            x = 123459876L;
        hi = x / 127773L;
        lo = x % 127773L;
        x = 16807L * lo - 2836L * hi;
        if (x < 0)
            x += 0x7fffffffL;
        return ((*ctx = x) % ((uint32_t)RANDOM_MAX + 1));
    }

    int32_t my_random(void)
    {
        return do_random(&next);
    }

    void my_srandom(uint32_t seed)
    {
        next = seed;
    }

    // then this is from  @Ryan Reich:
    // at https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range
    // Assumes 0 <= max <= RAND_MAX
    // Returns in the closed interval [0, max]
    int32_t random_at_most(int32_t max) {
        uint32_t
            // max <= RAND_MAX < ULONG_MAX, so this is okay.
            num_bins = (uint32_t) max + 1,
            num_rand = (uint32_t) RANDOM_MAX + 1,
            bin_size = num_rand / num_bins,
            defect   = num_rand % num_bins;
        int32_t x;
        do {
            x = my_random();
        }
        // This is carefully written not to overflow
        while (num_rand - defect <= (uint32_t)x);
        // Truncated division is intentional
        return x/bin_size;
    }
}
