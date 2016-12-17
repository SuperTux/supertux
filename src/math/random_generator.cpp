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

// Transliterated into C++ Allen King 060417, from sources on
//          http://www.jbox.dk/sanos/source/lib/random.c.html

#include <assert.h>
#include <stdexcept>
#include <stdio.h>
#include <time.h>

#include "math/random_generator.hpp"

RandomGenerator graphicsRandom;               // graphic RNG
RandomGenerator gameRandom;                   // game RNG

RandomGenerator::RandomGenerator() :
  initialized(0),
  fptr(),
  rptr(),
  state(),
  rand_type(),
  rand_deg(),
  rand_sep(),
  end_ptr(),
  debug(0) // change this by hand for debug
{
  assert(sizeof(int) >= 4);
  initialize();
}

RandomGenerator::~RandomGenerator() {
}

int RandomGenerator::srand(int x)    {
  int x0 = x;
  while (x <= 0)                          // random seed of zero means
    x = time(0) % RandomGenerator::rand_max; // randomize with time

  if (debug > 0)
    printf("==== srand(%10d) (%10d) rand_max=%x =====\n",
           x, x0, RandomGenerator::rand_max);

  RandomGenerator::srandom(x);
  return x;                               // let caller know seed used
}

int RandomGenerator::rand() {
  int rv;                                  // a positive int
  while ((rv = RandomGenerator::random()) <= 0) // neg or zero causes probs
    ;
  if (debug > 0)
    printf("==== rand(): %10d =====\n", rv);
  return rv;
}

int RandomGenerator::rand(int v) {
  assert(v >= 0); // illegal arg

  // remove biases, esp. when v is large (e.g. v == (rand_max/4)*3;)
  int rv, maxV =(RandomGenerator::rand_max / v) * v;
  while ((rv = RandomGenerator::random()) >= maxV)
    ;
  return rv % v;                          // mod it down to 0..(maxV-1)
}

int RandomGenerator::rand(int u, int v) {
  assert(v > u);
  return u + RandomGenerator::rand(v-u);
}

double RandomGenerator::randf(double v) {
  float rv;
  do {
    rv = ((double)RandomGenerator::random())/RandomGenerator::rand_max * v;
  } while (rv >= v);                      // rounding might cause rv==v

  if (debug > 0)
    printf("==== rand(): %f =====\n", rv);
  return rv;
}

double RandomGenerator::randf(double u, double v) {
  return u + RandomGenerator::randf(v-u);
}

//-----------------------------------------------------------------------
//
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
//

//**#include <os.h>

//
// An improved random number generation package.  In addition to the standard
// rand()/srand() like interface, this package also has a special state info
// interface.  The initstate() routine is called with a seed, an array of
// bytes, and a count of how many bytes are being passed in; this array is
// then initialized to contain information for random number generation with
// that much state information.  Good sizes for the amount of state
// information are 32, 64, 128, and 256 bytes.  The state can be switched by
// calling the setstate() routine with the same array as was initialized
// with initstate().  By default, the package runs with 128 bytes of state
// information and generates far better random numbers than a linear
// congruential generator.  If the amount of state information is less than
// 32 bytes, a simple linear congruential R.N.G. is used.
//
// Internally, the state information is treated as an array of longs; the
// zeroeth element of the array is the type of R.N.G. being used (small
// integer); the remainder of the array is the state information for the
// R.N.G.  Thus, 32 bytes of state information will give 7 longs worth of
// state information, which will allow a degree seven polynomial.  (Note:
// the zeroeth word of state information also has some other information
// stored in it -- see setstate() for details).
//
// The random number generation technique is a linear feedback shift register
// approach, employing trinomials (since there are fewer terms to sum up that
// way).  In this approach, the least significant bit of all the numbers in
// the state table will act as a linear feedback shift register, and will
// have period 2^deg - 1 (where deg is the degree of the polynomial being
// used, assuming that the polynomial is irreducible and primitive).  The
// higher order bits will have longer periods, since their values are also
// influenced by pseudo-random carries out of the lower bits.  The total
// period of the generator is approximately deg*(2**deg - 1); thus doubling
// the amount of state information has a vast influence on the period of the
// generator.  Note: the deg*(2**deg - 1) is an approximation only good for
// large deg, when the period of the shift is the dominant factor.
// With deg equal to seven, the period is actually much longer than the
// 7*(2**7 - 1) predicted by this formula.
//
// Modified 28 December 1994 by Jacob S. Rosenberg.
//

//
// For each of the currently supported random number generators, we have a
// break value on the amount of state information (you need at least this
// many bytes of state info to support this random number generator), a degree
// for the polynomial (actually a trinomial) that the R.N.G. is based on, and
// the separation between the two lower order coefficients of the trinomial.

void RandomGenerator::initialize() {

#define NSHUFF 100      // To drop part of seed -> 1st value correlation

  //static long degrees[MAX_TYPES] = { DEG_0, DEG_1, DEG_2, DEG_3, DEG_4 };
  //static long seps [MAX_TYPES] = { SEP_0, SEP_1, SEP_2, SEP_3, SEP_4 };

  degrees[0] = DEG_0;
  degrees[1] = DEG_1;
  degrees[2] = DEG_2;
  degrees[3] = DEG_3;
  degrees[4] = DEG_4;

  seps [0] = SEP_0;
  seps [1] = SEP_1;
  seps [2] = SEP_2;
  seps [3] = SEP_3;
  seps [4] = SEP_4;

  //
  // Initially, everything is set up as if from:
  //
  //  initstate(1, randtbl, 128);
  //
  // Note that this initialization takes advantage of the fact that srandom()
  // advances the front and rear pointers 10*rand_deg times, and hence the
  // rear pointer which starts at 0 will also end up at zero; thus the zeroeth
  // element of the state information, which contains info about the current
  // position of the rear pointer is just
  //
  //  MAX_TYPES * (rptr - state) + TYPE_3 == TYPE_3.

  randtbl[ 0] =  TYPE_3;
  randtbl[ 1] =  0x991539b1;
  randtbl[ 2] =  0x16a5bce3;
  randtbl[ 3] =  0x6774a4cd;
  randtbl[ 4] =  0x3e01511e;
  randtbl[ 5] =  0x4e508aaa;
  randtbl[ 6] =  0x61048c05;
  randtbl[ 7] =  0xf5500617;
  randtbl[ 8] =  0x846b7115;
  randtbl[ 9] =  0x6a19892c;
  randtbl[10] =  0x896a97af;
  randtbl[11] =  0xdb48f936;
  randtbl[12] =  0x14898454;
  randtbl[13] =  0x37ffd106;
  randtbl[14] =  0xb58bff9c;
  randtbl[15] =  0x59e17104;
  randtbl[16] =  0xcf918a49;
  randtbl[17] =  0x09378c83;
  randtbl[18] =  0x52c7a471;
  randtbl[19] =  0x8d293ea9;
  randtbl[20] =  0x1f4fc301;
  randtbl[21] =  0xc3db71be;
  randtbl[22] =  0x39b44e1c;
  randtbl[23] =  0xf8a44ef9;
  randtbl[24] =  0x4c8b80b1;
  randtbl[25] =  0x19edc328;
  randtbl[26] =  0x87bf4bdd;
  randtbl[27] =  0xc9b240e5;
  randtbl[28] =  0xe9ee4b1b;
  randtbl[29] =  0x4382aee7;
  randtbl[30] =  0x535b6b41;
  randtbl[31] =  0xf3bec5da;

  // static long randtbl[DEG_3 + 1] =
  // {
  //   TYPE_3;
  //   0x991539b1, 0x16a5bce3, 0x6774a4cd, 0x3e01511e, 0x4e508aaa, 0x61048c05,
  //   0xf5500617, 0x846b7115, 0x6a19892c, 0x896a97af, 0xdb48f936, 0x14898454,
  //   0x37ffd106, 0xb58bff9c, 0x59e17104, 0xcf918a49, 0x09378c83, 0x52c7a471,
  //   0x8d293ea9, 0x1f4fc301, 0xc3db71be, 0x39b44e1c, 0xf8a44ef9, 0x4c8b80b1,
  //   0x19edc328, 0x87bf4bdd, 0xc9b240e5, 0xe9ee4b1b, 0x4382aee7, 0x535b6b41,
  //   0xf3bec5da
  // };

  //
  // fptr and rptr are two pointers into the state info, a front and a rear
  // pointer.  These two pointers are always rand_sep places aparts, as they
  // cycle cyclically through the state information.  (Yes, this does mean we
  // could get away with just one pointer, but the code for random() is more
  // efficient this way).  The pointers are left positioned as they would be
  // from the call
  //
  //  initstate(1, randtbl, 128);
  //
  // (The position of the rear pointer, rptr, is really 0 (as explained above
  // in the initialization of randtbl) because the state table pointer is set
  // to point to randtbl[1] (as explained below).
  //

  fptr = &randtbl[SEP_3 + 1];
  rptr = &randtbl[1];

  //
  // The following things are the pointer to the state information table, the
  // type of the current generator, the degree of the current polynomial being
  // used, and the separation between the two pointers.  Note that for efficiency
  // of random(), we remember the first location of the state information, not
  // the zeroeth.  Hence it is valid to access state[-1], which is used to
  // store the type of the R.N.G.  Also, we remember the last location, since
  // this is more efficient than indexing every time to find the address of
  // the last element to see if the front and rear pointers have wrapped.
  //

  state = &randtbl[1];
  rand_type = TYPE_3;
  rand_deg = DEG_3;
  rand_sep = SEP_3;
  end_ptr = &randtbl[DEG_3 + 1];

}

//
// Compute x = (7^5 * x) mod (2^31 - 1)
// without overflowing 31 bits:
//      (2^31 - 1) = 127773 * (7^5) + 2836
// From "Random number generators: good ones are hard to find",
// Park and Miller, Communications of the ACM, vol. 31, no. 10,
// October 1988, p. 1195.
//

__inline static long good_rand(long x)
{
  long hi, lo;

  // Can't be initialized with 0, so use another value.
  if (x == 0) x = 123459876;
  hi = x / 127773;
  lo = x % 127773;
  x = 16807 * lo - 2836 * hi;
  if (x < 0) x += 0x7fffffff;
  return x;
}

//
// srandom
//
// Initialize the random number generator based on the given seed.  If the
// type is the trivial no-state-information type, just remember the seed.
// Otherwise, initializes state[] based on the given "seed" via a linear
// congruential generator.  Then, the pointers are set to known locations
// that are exactly rand_sep places apart.  Lastly, it cycles the state
// information a given number of times to get rid of any initial dependencies
// introduced by the L.C.R.N.G.  Note that the initialization of randtbl[]
// for default usage relies on values produced by this routine.

void RandomGenerator::srandom(unsigned long x)
{
  long i, lim;

  state[0] = x;
  if (rand_type == TYPE_0)
    lim = NSHUFF;
  else
  {
    for (i = 1; i < rand_deg; i++) state[i] = good_rand(state[i - 1]);
    fptr = &state[rand_sep];
    rptr = &state[0];
    lim = 10 * rand_deg;
  }

  initialized = 1;
  for (i = 0; i < lim; i++) random();
}

#ifdef NOT_FOR_SUPERTUX     // use in supertux doesn't require these methods,
// which are not portable to as many platforms as
// SDL.  The cost is that the variability of the
// initial seed is reduced to only 32 bits of
// randomness, seemingly enough. PAK 060420
//
// srandomdev
//
// Many programs choose the seed value in a totally predictable manner.
// This often causes problems.  We seed the generator using the much more
// secure random() interface.  Note that this particular seeding
// procedure can generate states which are impossible to reproduce by
// calling srandom() with any value, since the succeeding terms in the
// state buffer are no longer derived from the LC algorithm applied to
// a fixed seed.

void RandomGenerator::srandomdev()
{
  int fd, done;
  size_t len;

  if (rand_type == TYPE_0)
    len = sizeof state[0];
  else
    len = rand_deg * sizeof state[0];

  done = 0;
  fd = open("/dev/urandom", O_RDONLY);
  if (fd >= 0)
  {
    if (read(fd, state, len) == len) done = 1;
    close(fd);
  }

  if (!done)
  {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec ^ tv.tv_usec);
    return;
  }

  if (rand_type != TYPE_0)
  {
    fptr = &state[rand_sep];
    rptr = &state[0];
  }
  initialized = 1;
}

//
// initstate
//
// Initialize the state information in the given array of n bytes for future
// random number generation.  Based on the number of bytes we are given, and
// the break values for the different R.N.G.'s, we choose the best (largest)
// one we can and set things up for it.  srandom() is then called to
// initialize the state information.
//
// Note that on return from srandom(), we set state[-1] to be the type
// multiplexed with the current value of the rear pointer; this is so
// successive calls to initstate() won't lose this information and will be
// able to restart with setstate().
//
// Note: the first thing we do is save the current state, if any, just like
// setstate() so that it doesn't matter when initstate is called.
//
// Returns a pointer to the old state.
//

char * RandomGenerator::initstate(unsigned long seed, char *arg_state, long n)
{
  char *ostate = (char *) (&state[-1]);
  long *long_arg_state = (long *) arg_state;

  if (rand_type == TYPE_0)
    state[-1] = rand_type;
  else
    state[-1] = MAX_TYPES * (rptr - state) + rand_type;

  if (n < BREAK_0) return NULL;

  if (n < BREAK_1)
  {
    rand_type = TYPE_0;
    rand_deg = DEG_0;
    rand_sep = SEP_0;
  }
  else if (n < BREAK_2)
  {
    rand_type = TYPE_1;
    rand_deg = DEG_1;
    rand_sep = SEP_1;
  }
  else if (n < BREAK_3)
  {
    rand_type = TYPE_2;
    rand_deg = DEG_2;
    rand_sep = SEP_2;
  }
  else if (n < BREAK_4)
  {
    rand_type = TYPE_3;
    rand_deg = DEG_3;
    rand_sep = SEP_3;
  }
  else
  {
    rand_type = TYPE_4;
    rand_deg = DEG_4;
    rand_sep = SEP_4;
  }

  state = (long *) (long_arg_state + 1); // First location
  end_ptr = &state[rand_deg]; // Must set end_ptr before srandom
  srandom(seed);

  if (rand_type == TYPE_0)
    long_arg_state[0] = rand_type;
  else
    long_arg_state[0] = MAX_TYPES * (rptr - state) + rand_type;

  initialized = 1;
  return ostate;
}

//
// setstate
//
// Restore the state from the given state array.
//
// Note: it is important that we also remember the locations of the pointers
// in the current state information, and restore the locations of the pointers
// from the old state information.  This is done by multiplexing the pointer
// location into the zeroeth word of the state information.
//
// Note that due to the order in which things are done, it is OK to call
// setstate() with the same state as the current state.
//
// Returns a pointer to the old state information.
//

char * RandomGenerator::setstate(char *arg_state)
{
  long *new_state = (long *) arg_state;
  long type = new_state[0] % MAX_TYPES;
  long rear = new_state[0] / MAX_TYPES;
  char *ostate = (char *) (&state[-1]);

  if (rand_type == TYPE_0)
    state[-1] = rand_type;
  else
    state[-1] = MAX_TYPES * (rptr - state) + rand_type;

  switch(type)
  {
    case TYPE_0:
    case TYPE_1:
    case TYPE_2:
    case TYPE_3:
    case TYPE_4:
      rand_type = type;
      rand_deg = degrees[type];
      rand_sep = seps[type];
      break;
  }

  state = (long *) (new_state + 1);
  if (rand_type != TYPE_0)
  {
    rptr = &state[rear];
    fptr = &state[(rear + rand_sep) % rand_deg];
  }
  end_ptr = &state[rand_deg];   // Set end_ptr too

  initialized = 1;
  return ostate;
}
#endif //NOT_FOR_SUPERTUX
//
// random:
//
// If we are using the trivial TYPE_0 R.N.G., just do the old linear
// congruential bit.  Otherwise, we do our fancy trinomial stuff, which is
// the same in all the other cases due to all the global variables that have
// been set up.  The basic operation is to add the number at the rear pointer
// into the one at the front pointer.  Then both pointers are advanced to
// the next location cyclically in the table.  The value returned is the sum
// generated, reduced to 31 bits by throwing away the "least random" low bit.
//
// Note: the code takes advantage of the fact that both the front and
// rear pointers can't wrap on the same call by not testing the rear
// pointer if the front one has wrapped.
//
// Returns a 31-bit random number.
//

long RandomGenerator::random()
{
  long i;
  long *f, *r;
  if (!initialized) {
    throw std::runtime_error("uninitialized RandomGenerator object");
  }

  if (rand_type == TYPE_0)
  {
    i = state[0];
    state[0] = i = (good_rand(i)) & 0x7fffffff;
  }
  else
  {
    f = fptr; r = rptr;
    *f += *r;
    i = (*f >> 1) & 0x7fffffff; // Chucking least random bit
    if (++f >= end_ptr)
    {
      f = state;
      ++r;
    }
    else if (++r >= end_ptr)
      r = state;

    fptr = f; rptr = r;
  }

  return i;
}

/* EOF */
