#ifndef CRIBBAGE_CPP_H_INCLUDED
#define CRIBBAGE_CPP_H_INCLUDED

#include <cstdint>

namespace cribbage_cpp {

    //random number generator
    const uint32_t RANDOM_MAX (0x7FFFFFFF);
    int32_t do_random(uint32_t *ctx);
    int32_t my_random(void);
    void my_srandom(uint32_t seed);
    int32_t random_at_most(int32_t max);
}


#endif // CRIBBAGE_CPP_H_INCLUDED
