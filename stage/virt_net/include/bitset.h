/*
 * Copyright (c) 2016-2023 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SYS__BITSET_H_
#define _SYS__BITSET_H_

#include <stddef.h>

#if 0
/*
 * Find First Set bit
 */
static inline int
ffsl(long mask) {
  int bit;

  if (mask == 0)
    return (0);
  for (bit = 1; !(mask & 1); bit++)
    mask = (unsigned long)mask >> 1;
  return (bit);
}

/*
 * Find Last Set bit
 */
static inline int
flsl(long mask) {
  int bit;

  if (mask == 0)
    return (0);
  for (bit = 1; mask != 1; bit++)
    mask = (unsigned long)mask >> 1;
  return (bit);
}
#endif

#define __size_t size_t

/*
 * Macros addressing word and bit within it, tuned to make compiler
 * optimize cases when SETSIZE fits into single machine word.
 */
#define _BITSET_BITS (sizeof(long) * 8)

#define __howmany(x, y) (((x) + ((y)-1)) / (y))

#define __bitset_words(_s) (__howmany(_s, _BITSET_BITS))

#define BITSET_DEFINE(t, _s)           \
  struct t {                           \
    long __bits[__bitset_words((_s))]; \
  }

#define __bitset_mask(_s, n) \
  (1L << ((__bitset_words((_s)) == 1) ? (__size_t)(n) : ((n) % _BITSET_BITS)))

#define __bitset_word(_s, n) \
  ((__bitset_words((_s)) == 1) ? 0 : ((n) / _BITSET_BITS))

#define BIT_CLR(_s, n, p) \
  ((p)->__bits[__bitset_word(_s, n)] &= ~__bitset_mask((_s), (n)))

#define BIT_ISSET(_s, n, p) \
  ((((p)->__bits[__bitset_word(_s, n)] & __bitset_mask((_s), (n))) != 0))

#define BIT_SET(_s, n, p) \
  ((p)->__bits[__bitset_word(_s, n)] |= __bitset_mask((_s), (n)))

#define BIT_ZERO(_s, p)                              \
  do {                                               \
    __size_t __i;                                    \
    for (__i = 0; __i < __bitset_words((_s)); __i++) \
      (p)->__bits[__i] = 0L;                         \
  } while (0)

#define BIT_FILL(_s, p)                              \
  do {                                               \
    __size_t __i;                                    \
    for (__i = 0; __i < __bitset_words((_s)); __i++) \
      (p)->__bits[__i] = -1L;                        \
  } while (0)

#define BIT_SETOF(_s, n, p)                                       \
  do {                                                            \
    BIT_ZERO(_s, p);                                              \
    (p)->__bits[__bitset_word(_s, n)] = __bitset_mask((_s), (n)); \
  } while (0)

/* Is p empty. */
#define BIT_EMPTY(_s, p) __extension__({           \
  __size_t __i;                                    \
  for (__i = 0; __i < __bitset_words((_s)); __i++) \
    if ((p)->__bits[__i])                          \
      break;                                       \
  __i == __bitset_words((_s));                     \
})

/* Is p full set. */
#define BIT_ISFULLSET(_s, p) __extension__({       \
  __size_t __i;                                    \
  for (__i = 0; __i < __bitset_words((_s)); __i++) \
    if ((p)->__bits[__i] != (long)-1)              \
      break;                                       \
  __i == __bitset_words((_s));                     \
})

/* Is c a subset of p. */
#define BIT_SUBSET(_s, p, c) __extension__({       \
  __size_t __i;                                    \
  for (__i = 0; __i < __bitset_words((_s)); __i++) \
    if (((c)->__bits[__i] &                        \
	 (p)->__bits[__i]) !=                      \
	(c)->__bits[__i])                          \
      break;                                       \
  __i == __bitset_words((_s));                     \
})

/* Are there any common bits between b & c? */
#define BIT_OVERLAP(_s, p, c) __extension__({      \
  __size_t __i;                                    \
  for (__i = 0; __i < __bitset_words((_s)); __i++) \
    if (((c)->__bits[__i] &                        \
	 (p)->__bits[__i]) != 0)                   \
      break;                                       \
  __i != __bitset_words((_s));                     \
})

/* Compare two sets, returns 0 if equal 1 otherwise. */
#define BIT_CMP(_s, p, c) __extension__({          \
  __size_t __i;                                    \
  for (__i = 0; __i < __bitset_words((_s)); __i++) \
    if (((c)->__bits[__i] !=                       \
	 (p)->__bits[__i]))                        \
      break;                                       \
  __i != __bitset_words((_s));                     \
})

#define BIT_OR(_s, d, s)                             \
  do {                                               \
    __size_t __i;                                    \
    for (__i = 0; __i < __bitset_words((_s)); __i++) \
      (d)->__bits[__i] |= (s)->__bits[__i];          \
  } while (0)

#define BIT_OR2(_s, d, s1, s2)                                  \
  do {                                                          \
    __size_t __i;                                               \
    for (__i = 0; __i < __bitset_words((_s)); __i++)            \
      (d)->__bits[__i] = (s1)->__bits[__i] | (s2)->__bits[__i]; \
  } while (0)

#define BIT_AND(_s, d, s)                            \
  do {                                               \
    __size_t __i;                                    \
    for (__i = 0; __i < __bitset_words((_s)); __i++) \
      (d)->__bits[__i] &= (s)->__bits[__i];          \
  } while (0)

#define BIT_AND2(_s, d, s1, s2)                                 \
  do {                                                          \
    __size_t __i;                                               \
    for (__i = 0; __i < __bitset_words((_s)); __i++)            \
      (d)->__bits[__i] = (s1)->__bits[__i] & (s2)->__bits[__i]; \
  } while (0)

#define BIT_NAND(_s, d, s)                           \
  do {                                               \
    __size_t __i;                                    \
    for (__i = 0; __i < __bitset_words((_s)); __i++) \
      (d)->__bits[__i] &= ~(s)->__bits[__i];         \
  } while (0)

#define BIT_NAND2(_s, d, s1, s2)                                 \
  do {                                                           \
    __size_t __i;                                                \
    for (__i = 0; __i < __bitset_words((_s)); __i++)             \
      (d)->__bits[__i] = (s1)->__bits[__i] & ~(s2)->__bits[__i]; \
  } while (0)

#define BIT_XOR(_s, d, s)                            \
  do {                                               \
    __size_t __i;                                    \
    for (__i = 0; __i < __bitset_words((_s)); __i++) \
      (d)->__bits[__i] ^= (s)->__bits[__i];          \
  } while (0)

#define BIT_XOR2(_s, d, s1, s2)                                 \
  do {                                                          \
    __size_t __i;                                               \
    for (__i = 0; __i < __bitset_words((_s)); __i++)            \
      (d)->__bits[__i] = (s1)->__bits[__i] ^ (s2)->__bits[__i]; \
  } while (0)

#define BIT_FFS(_s, p) __extension__({               \
  __size_t __i;                                      \
  int __bit;                                         \
                                                     \
  __bit = 0;                                         \
  for (__i = 0; __i < __bitset_words((_s)); __i++) { \
    if ((p)->__bits[__i] != 0) {                     \
      __bit = ffsl((p)->__bits[__i]);                \
      __bit += __i * _BITSET_BITS;                   \
      break;                                         \
    }                                                \
  }                                                  \
  __bit;                                             \
})

#define BIT_FLS(_s, p) __extension__({               \
  __size_t __i;                                      \
  int __bit;                                         \
                                                     \
  __bit = 0;                                         \
  for (__i = __bitset_words((_s)); __i > 0; __i--) { \
    if ((p)->__bits[__i - 1] != 0) {                 \
      __bit = flsl((p)->__bits[__i - 1]);            \
      __bit += (__i - 1) * _BITSET_BITS;             \
      break;                                         \
    }                                                \
  }                                                  \
  __bit;                                             \
})

#define BIT_COUNT(_s, p) __extension__({           \
  __size_t __i;                                    \
  int __count;                                     \
                                                   \
  __count = 0;                                     \
  for (__i = 0; __i < __bitset_words((_s)); __i++) \
    __count += __bitcountl((p)->__bits[__i]);      \
  __count;                                         \
})

#define BITSET_T_INITIALIZER(x) \
  {                             \
    .__bits = { x }             \
  }

/*
 * Helper to declare a bitset without it's size being a constant.
 *
 * Sadly we cannot declare a bitset struct with '__bits[]', because it's
 * the only member of the struct and the compiler complains.
 */
#define BITSET_DEFINE_VAR(t) BITSET_DEFINE(t, 1)

/*
 * Define a default type that can be used while manually specifying size
 * to every call.
 */
BITSET_DEFINE(bitset, 1);

#endif /* !_SYS__BITSET_H_ */
