// $Id$
//
// A strong random number generator
//
// Copyright (C) 2006 Allen King
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Copyright (C) 1983, 1993 The Regents of the University of California.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#ifndef HEADER_SUPERTUX_MATH_RANDOM_GENERATOR_HPP
#define HEADER_SUPERTUX_MATH_RANDOM_GENERATOR_HPP

class RandomGenerator
{
private:
// Array versions of the above information to make code run faster --
// relies on fact that TYPE_i == i.
    static const int TYPE_0 = 0;   // Linear congruential
    static const int BREAK_0 = 8;
    static const int DEG_0 = 0;
    static const int SEP_0 = 0;

    static const int TYPE_1 = 1;   // x**7 + x**3 + 1
    static const int BREAK_1 = 32;
    static const int DEG_1 = 7;
    static const int SEP_1 = 3;

    static const int TYPE_2 = 2;   // x**15 + x + 1
    static const int BREAK_2 = 64;
    static const int DEG_2 = 15;
    static const int SEP_2 = 1;

    static const int TYPE_3 = 3;   // x**31 + x**3 + 1
    static const int BREAK_3 = 128;
    static const int DEG_3 = 31;
    static const int SEP_3 = 3;

    static const int TYPE_4 = 4;   // x**63 + x + 1
    static const int BREAK_4 = 256;
    static const int DEG_4 = 63;
    static const int SEP_4 = 1;

    static const int MAX_TYPES = 5;     // Max number of types above

    bool initialized;
    long degrees[MAX_TYPES];
    long seps [MAX_TYPES];
    long randtbl[DEG_3 + 1];

    long *fptr;
    long *rptr;

    long *state;
    long rand_type;
    long rand_deg;
    long rand_sep;
    long *end_ptr;
    int debug;
    static const int rand_max = 0x7fffffff;         // biggest signed Uint32

public:
    RandomGenerator();
    ~RandomGenerator();

// Documentation of user-visible calls:

     // Initialize the RNG with a 31-bit seed
    // if x is zero or absent, calls to time() will get a time-randomized seed
    // the value returned is the value of the seed used.
    int srand(int x=0);

     // generate random 31-bit numbers
    // calls to the following return a value evenly distributed between u (or
    // 0 if not specified) and v (or rand_max if not specified).  Return
    // values may include u, but never v.
    int rand();
    int rand(int v);
    int rand(int u, int v);
    double randf(double v);
    double randf(double u, double v);

    // For Squirrel wrapper, since miniswig (and even squirrel?) doesn't
    // support function overloading or doubles
    int rand1i(int v) { return rand(v); }
    int rand2i(int u, int v) { return rand(u, v); }
    float rand1f(float v)
      { return static_cast<float>(randf(static_cast<double>(v))); }
    float rand2f(float u, float v)
      { return static_cast<float>(randf(static_cast<double>(u),
                                      static_cast<double>(v))); }

//private:
    void initialize();
    void srandom(unsigned long x);
//  void srandomdev();
//  char *initstate(unsigned long seed, char *arg_state, long n);
//  char *setstate(char *arg_state);
    long random();

private:
  RandomGenerator(const RandomGenerator&);
  RandomGenerator& operator=(const RandomGenerator&);
};

extern RandomGenerator systemRandom;

#endif //__RANDOM_GENERATOR__
