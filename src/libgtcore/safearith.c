/*
  Copyright (c) 2007 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2007 Center for Bioinformatics, University of Hamburg

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <limits.h>
#include "libgtcore/ensure.h"
#include "libgtcore/safearith.h"

int safe_abs(int j)
{
  int rval = j < 0 ? -j : j;
  assert(rval >= 0); /* prevent overflow */
  return rval;
}

long safe_labs(long j)
{
  long rval = j < 0 ? -j : j;
  assert(rval >= 0); /* prevent overflow */
  return rval;
}

long long safe_llabs(long long j)
{
  long long rval = j < 0 ? -j : j;
  assert(rval >= 0); /* prevent overflow */
  return rval;
}

int safearith_example(Env *env)
{
  unsigned long ulong;
  long slong;
  unsigned int a, b, c;
  int dest, src;

  env_error_check(env);

  /* safe assignments */
  slong = 256;
  safe_assign(ulong, slong);
  assert(ulong = 256);

  /* safe additions */
  a = 256;
  b = 1;
  safe_add(c, a, b);
  assert(c == 257);

  /* safe subtractions */
  a = 256;
  b = 1;
  safe_sub(c, a, b);
  assert(c == 255);

  /* safe absolutes */
  src = -256;
  dest = safe_abs(src);
  assert(dest == 256);

  return 0;
}

int safearith_unit_test(Env *env)
{
  int had_err = 0;
  env_error_check(env);

  {
    ensure(had_err, __MIN(char) == -128);
    ensure(had_err, __MAX(char) == 127);
    ensure(had_err, __MIN(unsigned char) == 0);
    ensure(had_err, __MAX(unsigned char) == 255);

    ensure(had_err, __MIN(short) == SHRT_MIN);
    ensure(had_err, __MAX(short) == SHRT_MAX);
    ensure(had_err, __MIN(unsigned short) == 0);
    ensure(had_err, __MAX(unsigned short) == USHRT_MAX);

    ensure(had_err, __MIN(int) == INT_MIN);
    ensure(had_err, __MAX(int) == INT_MAX);
    ensure(had_err, __MIN(unsigned int) == 0);
    ensure(had_err, __MAX(unsigned int) == UINT_MAX);

    ensure(had_err, __MIN(long) == LONG_MIN);
    ensure(had_err, __MAX(long) == LONG_MAX);
    ensure(had_err, __MIN(unsigned long) == 0);
    ensure(had_err, __MAX(unsigned long) == ULONG_MAX);

#ifdef LLONG_MIN
    ensure(had_err, __MIN(long long) == LLONG_MIN);
    ensure(had_err, __MAX(long long) == LLONG_MAX);
    ensure(had_err, __MIN(unsigned long long) == 0);
    ensure(had_err, __MAX(unsigned long long) == ULLONG_MAX);
#endif
  }

  {
    unsigned long ulong;
    long slong;

    slong = -1;
    ensure(had_err, assign(ulong, slong));

    ulong = 0;
    slong = LONG_MAX;
    ensure(had_err, !assign(ulong, slong) && ulong == LONG_MAX);

    ulong = ULONG_MAX;
    ensure(had_err, assign(slong, ulong));

    slong = 0;
    ulong = LONG_MAX;
    ensure(had_err, !assign(slong, ulong) && slong == LONG_MAX);
  }

  {
    int x;
    ensure(had_err, add_of(x, INT_MAX, 1));
    ensure(had_err, add_of(x, INT_MAX, 256));
    ensure(had_err, add_of(x, INT_MAX, INT_MAX));

    x = 0; ensure(had_err, !add_of(x, INT_MAX - 1, 1) && x == INT_MAX);
    x = 0; ensure(had_err, !add_of(x, INT_MAX - 256, 256) && x == INT_MAX);
    x = 0; ensure(had_err, !add_of(x, INT_MAX, 0) && x == INT_MAX);

    ensure(had_err, add_of(x, 0x100000000ll, 0x100000000ll));
    x = INT_MAX;
    ensure(had_err, !add_of(x, 0x100000000ll, -0x100000000ll) && x == 0);

    ensure(had_err, sub_of(x, INT_MIN, 1));
    ensure(had_err, sub_of(x, INT_MIN, 256));
    ensure(had_err, sub_of(x, INT_MIN, INT_MAX));

    x = 0; ensure(had_err, !sub_of(x, INT_MIN + 1, 1) && x == INT_MIN);
    x = 0; ensure(had_err, !sub_of(x, INT_MIN + 256, 256) && x == INT_MIN);
    x = 0; ensure(had_err, !sub_of(x, INT_MIN, 0) && x == INT_MIN);
  }

  {
    unsigned int x;
    ensure(had_err, add_of(x, UINT_MAX, 1));
    ensure(had_err, add_of(x, UINT_MAX, 256));
    ensure(had_err, add_of(x, UINT_MAX, UINT_MAX));

    x = 0; ensure(had_err, !add_of(x, UINT_MAX - 1, 1) && x == UINT_MAX);
    x = 0; ensure(had_err, !add_of(x, UINT_MAX - 256, 256) && x == UINT_MAX);
    x = 0; ensure(had_err, !add_of(x, UINT_MAX, 0) && x == UINT_MAX);

    ensure(had_err, add_of(x, 0x100000000ll, 0x100000000ll));
    x = UINT_MAX;
    ensure(had_err, !add_of(x, 0x100000000ll, -0x100000000ll) && x == 0);

    ensure(had_err, sub_of(x, 0, 1));
    ensure(had_err, sub_of(x, 0, 256));
    ensure(had_err, sub_of(x, 0, UINT_MAX));

    x = 0; ensure(had_err, !sub_of(x, 1, 1) && x == 0);
    x = 0; ensure(had_err, !sub_of(x, 256, 256) && x == 0);
    x = 0; ensure(had_err, !sub_of(x, 0, 0) && x == 0);
  }

  {
    int i;
    long l;
    long long ll;

    i = safe_abs(0);
    ensure(had_err, i == 0);

    i = safe_abs(-1);
    ensure(had_err, i == 1);

    i = safe_abs(INT_MIN + 1);
    ensure(had_err, i == INT_MAX);

    l = safe_labs(0);
    ensure(had_err, l == 0);

    l = safe_labs(-1);
    ensure(had_err, l == 1);

    l = safe_labs(LONG_MIN + 1);
    ensure(had_err, l == LONG_MAX);

    ll = safe_llabs(0);
    ensure(had_err, ll == 0);

    ll = safe_llabs(-1);
    ensure(had_err, ll == 1);

#ifdef LLONG_MIN
    ll = safe_llabs(LLONG_MIN + 1);
    ensure(had_err, ll == LLONG_MAX);
#endif
  }

  return had_err;
}
