#ifndef RANDOM_H_INCLUDE
#define RANDOM_H_INCLUDE

#include <cstdint>
#include <limits>
#include <random>
#include <thread>
#include <cassert>

class RandomBase {
public:
    // The interface for STL.
    using result_type = std::uint64_t;

    constexpr static result_type min() {
        return std::numeric_limits<result_type>::min();
    }

    constexpr static result_type max() {
        return std::numeric_limits<result_type>::max();
    }

    virtual result_type operator()() = 0;
};

/*
 * xorshift64star Pseudo-Random Number Generator
 * This class is based on original code written and dedicated
 * to the public domain by Sebastiano Vigna (2014).
 * It has the following characteristics:
 *
 *  -  Outputs 64-bit numbers
 *  -  Passes Dieharder and SmallCrush test batteries
 *  -  Does not require warm-up, no zeroland to escape
 *  -  Internal state is a single 64-bit integer
 *  -  Period is 2^64 - 1
 *  -  Speed: 1.60 ns/call (Core i7 @3.40GHz)
 *
 * For further analysis see
 *   <http://vigna.di.unimi.it/ftp/papers/xorshift.pdf>
 */
class PRNG : public RandomBase {
public:
    PRNG(std::uint64_t seed) : s_(seed) { assert(seed); }

    static PRNG &get() {
        std::random_device rd_;
        static thread_local PRNG rng(rd_());
        return rng;
    }

    inline std::uint64_t rand64() {
        s_ ^= s_ >> 12;
        s_ ^= s_ << 25;
        s_ ^= s_ >> 27;
        return s_ * 2685821657736338717LL;
    }

    virtual std::uint64_t operator()() { return rand64(); }

private:
    std::uint64_t s_;
};

#endif
