/*
** autogenerated content - DO NOT EDIT
*/
/*
  Copyright (C) 2007 Thomas Jahns <Thomas.Jahns@gmx.net>

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
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <time.h>
#include <sys/time.h>

#include "core/assert_api.h"
#include "core/bitpackstring.h"
#include "core/error.h"
#include "core/ensure.h"
#include "core/log.h"
#include "core/ma.h"
#include "core/yarandom.h"

enum {
/*   MAX_RND_NUMS = 10, */
  MAX_RND_NUMS_unsigned = 100000,
};

static inline int
icmp_unsigned(unsigned a, unsigned b)
{
  if (a > b)
    return 1;
  else if (a < b)
    return -1;
  else /* if (a == b) */
    return 0;
}

/**
 * \brief bit count reference
 * @param v count the number of bits set in v
 */
static inline int
genBitCount_unsigned(unsigned v)
{
  unsigned c; /* c accumulates the total bits set in v */
  for (c = 0; v; c++)
    v &= v - 1; /* clear the least significant bit set */
  return c;
}

#define freeResourcesAndReturn(retval) \
  do {                                 \
    gt_free(numBitsList);              \
    gt_free(randSrc);                  \
    gt_free(randCmp);                  \
    gt_free(bitStore);                 \
    gt_free(bitStoreCopy);             \
    return retval;                     \
  } while (0)

int
gt_bitPackStringInt_unit_test(GtError *err)
{
  BitString bitStore = NULL;
  BitString bitStoreCopy = NULL;
  unsigned *randSrc = NULL; /*< create random ints here for input as bit
                                *  store */
  unsigned *randCmp = NULL; /*< used for random ints read back */
  unsigned *numBitsList = NULL;
  size_t i, numRnd;
  BitOffset offsetStart, offset;
  int had_err = 0;
  offset = offsetStart = random()%(sizeof (unsigned) * CHAR_BIT);
  numRnd = random() % (MAX_RND_NUMS_unsigned + 1);
  gt_log_log("offset=%lu, numRnd=%lu\n",
          (long unsigned)offsetStart, (long unsigned)numRnd);
  {
    BitOffset numBits = sizeof (unsigned) * CHAR_BIT * numRnd + offsetStart;
    randSrc = gt_malloc(sizeof (unsigned)*numRnd);
    bitStore = gt_malloc(bitElemsAllocSize(numBits) * sizeof (BitElem));
    bitStoreCopy = gt_calloc(bitElemsAllocSize(numBits), sizeof (BitElem));
    randCmp = gt_malloc(sizeof (unsigned)*numRnd);
  }
  /* first test unsigned types */
  gt_log_log("gt_bsStoreUInt/gt_bsGetUInt: ");
  for (i = 0; i < numRnd; ++i)
  {
#if 32 > 32 && LONG_BIT < 32
    unsigned v = randSrc[i] = (unsigned)random() << 32 | random();
#else /* 32 > 32 && LONG_BIT < 32 */
    unsigned v = randSrc[i] = random();
#endif /* 32 > 32 && LONG_BIT < 32 */
    int bits = gt_requiredUIntBits(v);
    gt_bsStoreUInt(bitStore, offset, bits, v);
    offset += bits;
  }
  offset = offsetStart;
  for (i = 0; i < numRnd; ++i)
  {
    unsigned v = randSrc[i];
    int bits = gt_requiredUIntBits(v);
    unsigned r = gt_bsGetUInt(bitStore, offset, bits);
    ensure(had_err, r == v);
    if (had_err)
    {
      gt_log_log("Expected %""u"", got %""u"", i = %lu\n",
              v, r, (unsigned long)i);
      freeResourcesAndReturn(had_err);
    }
    offset += bits;
  }
  gt_log_log("passed\n");
  if (numRnd > 0)
  {
    unsigned v = randSrc[0], r = 0;
    unsigned numBits = gt_requiredUIntBits(v);
    BitOffset i = offsetStart + numBits;
    unsigned mask = ~(unsigned)0;
    if (numBits < 32)
      mask = ~(mask << numBits);
    gt_log_log("bsSetBit, gt_bsClearBit, bsToggleBit, gt_bsGetBit: ");
    while (v)
    {
      int lowBit = v & 1;
      v >>= 1;
      ensure(had_err, lowBit == (r = gt_bsGetBit(bitStore, --i)));
      if (had_err)
      {
        gt_log_log("Expected %d, got %d, i = %llu\n",
                lowBit, (int)r, (unsigned long long)i);
        freeResourcesAndReturn(had_err);
      }
    }
    i = offsetStart + numBits;
    gt_bsClear(bitStoreCopy, offsetStart, numBits, random()&1);
    v = randSrc[0];
    while (i)
    {
      int lowBit = v & 1;
      v >>= 1;
      if (lowBit)
        bsSetBit(bitStoreCopy, --i);
      else
        gt_bsClearBit(bitStoreCopy, --i);
    }
    v = randSrc[0];
    r = gt_bsGetUInt(bitStoreCopy, offsetStart, numBits);
    ensure(had_err, r == v);
    if (had_err)
    {
      gt_log_log("Expected %""u"", got %""u""\n", v, r);
      freeResourcesAndReturn(had_err);
    }
    for (i = 0; i < numBits; ++i)
      bsToggleBit(bitStoreCopy, offsetStart + i);
    r = gt_bsGetUInt(bitStoreCopy, offsetStart, numBits);
    ensure(had_err, r == (v = (~v & mask)));
    if (had_err)
    {
      gt_log_log("Expected %""u"", got %""u""\n", v, r);
      freeResourcesAndReturn(had_err);
    }
    gt_log_log("passed\n");
  }
  if (numRnd > 1)
  {
    gt_log_log("gt_bsCompare: ");
    {
      unsigned v0 = randSrc[0];
      int bits0 = gt_requiredUIntBits(v0);
      unsigned r0;
      offset = offsetStart;
      r0 = gt_bsGetUInt(bitStore, offset, bits0);
      for (i = 1; i < numRnd; ++i)
      {
        unsigned v1 = randSrc[i];
        int bits1 = gt_requiredUIntBits(v1);
        unsigned r1 = gt_bsGetUInt(bitStore, offset + bits0, bits1);
        int result = -2;   /*< -2 is not a return value of gt_bsCompare, thus
                            *   if it is displayed, there was an earlier
                            *   error. */
        ensure(had_err, r0 == v0 && r1 == v1);
        ensure(had_err, icmp_unsigned(v0, v1) ==
               (result = gt_bsCompare(bitStore, offset, bits0,
                                   bitStore, offset + bits0, bits1)));
        if (had_err)
        {
          gt_log_log("Expected v0 %s v1, got v0 %s v1,\n for v0=%"
                  "u"" and v1=%""u"",\n"
                  "i = %lu, bits0=%u, bits1=%u\n",
                  (v0 > v1?">":(v0 < v1?"<":"==")),
                  (result > 0?">":(result < 0?"<":"==")), v0, v1,
                  (unsigned long)i, bits0, bits1);
          freeResourcesAndReturn(had_err);
        }
        offset += bits0;
        bits0 = bits1;
        v0 = v1;
        r0 = r1;
      }
    }
    gt_log_log("passed\n");
  }
  gt_log_log("gt_bsStoreUniformUIntArray/gt_bsGetUInt: ");
  {
    unsigned numBits = random()%32 + 1;
    unsigned mask = ~(unsigned)0;
    if (numBits < 32)
      mask = ~(mask << numBits);
    offset = offsetStart;
    gt_bsStoreUniformUIntArray(bitStore, offset, numBits, numRnd,
                                     randSrc);
    for (i = 0; i < numRnd; ++i)
    {
      unsigned v = randSrc[i] & mask;
      unsigned r = gt_bsGetUInt(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log("Expected %""u"", got %""u"",\n"
                "i = %lu, bits=%u\n", v, r, (unsigned long)i, numBits);
        freeResourcesAndReturn(had_err);
      }
      offset += numBits;
    }
    gt_log_log("passed\n");
    gt_log_log("gt_bsStoreUniformUIntArray/"
               "gt_bsGetUniformUIntArray: ");
    gt_bsGetUniformUIntArray(bitStore, offset = offsetStart,
                               numBits, numRnd, randCmp);
    for (i = 0; i < numRnd; ++i)
    {
      unsigned v = randSrc[i] & mask;
      unsigned r = randCmp[i];
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log( "Expected %""u"", got %""u"",\n"
                " i = %lu, bits=%u\n",
                v, r, (unsigned long)i, numBits);
        freeResourcesAndReturn(had_err);
      }
    }
    if (numRnd > 1)
    {
      unsigned v = randSrc[0] & mask;
      unsigned r;
      gt_bsGetUniformUIntArray(bitStore, offsetStart,
                                 numBits, 1, &r);
      if (r != v)
      {
        gt_log_log("Expected %""u"", got %""u"","
                " one value extraction\n",
                v, r);
        freeResourcesAndReturn(had_err);
      }
    }
    gt_log_log(" passed\n");
  }
  /* int types */
  gt_log_log("gt_bsStoreInt/gt_bsGetInt: ");
  for (i = 0; i < numRnd; ++i)
  {
    int v = (int)randSrc[i];
    unsigned bits = gt_requiredIntBits(v);
    gt_bsStoreInt(bitStore, offset, bits, v);
    offset += bits;
  }
  offset = offsetStart;
  for (i = 0; i < numRnd; ++i)
  {
    int v = randSrc[i];
    unsigned bits = gt_requiredIntBits(v);
    int r = gt_bsGetInt(bitStore, offset, bits);
    ensure(had_err, r == v);
    if (had_err)
    {
      gt_log_log("Expected %""d"", got %""d"",\n"
                  "i = %lu, bits=%u\n",
                  v, r, (unsigned long)i, bits);
      freeResourcesAndReturn(had_err);
    }
    offset += bits;
  }
  gt_log_log("passed\n");
  gt_log_log("gt_bsStoreUniformIntArray/gt_bsGetInt: ");
  {
    unsigned numBits = random()%32 + 1;
    int mask = ~(int)0;
    if (numBits < 32)
      mask = ~(mask << numBits);
    offset = offsetStart;
    gt_bsStoreUniformIntArray(bitStore, offset, numBits, numRnd,
                                (int *)randSrc);
    for (i = 0; i < numRnd; ++i)
    {
      int m = (int)1 << (numBits - 1);
      int v = (int)((randSrc[i] & mask) ^ m) - m;
      int r = gt_bsGetInt(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log("Expected %""d"", got %""d"",\n"
                    "i = %lu, numBits=%u\n",
                    v, r, (unsigned long)i, numBits);
        freeResourcesAndReturn(had_err);
      }
      offset += numBits;
    }
    gt_log_log("passed\n");
    gt_log_log("gt_bsStoreUniformIntArray/"
               "gt_bsGetUniformIntArray: ");
    gt_bsGetUniformIntArray(bitStore, offset = offsetStart,
                              numBits, numRnd, (int *)randCmp);
    for (i = 0; i < numRnd; ++i)
    {
      int m = (int)1 << (numBits - 1);
      int v = (int)((randSrc[i] & mask) ^ m) - m;
      int r = randCmp[i];
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log("Expected %""d"", got %""d"", i = %lu\n",
                v, r, (unsigned long)i);
        freeResourcesAndReturn(had_err);
      }
    }
    if (numRnd > 0)
    {
      int m = (int)1 << (numBits - 1);
      int v = (int)((randSrc[0] & mask) ^ m) - m;
      int r = 0;
      gt_bsGetUniformIntArray(bitStore, offsetStart,
                                numBits, 1, &r);
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log("Expected %""d"", got %""d"
                ", one value extraction\n",
                v, r);
        freeResourcesAndReturn(had_err);
      }
    }
    gt_log_log("passed\n");
  }

  gt_log_log("gt_bsStoreNonUniformUIntArray/gt_bsGetUInt: ");
  {
    BitOffset bitsTotal = 0;
    numBitsList = gt_malloc(sizeof (unsigned) * numRnd);
    for (i = 0; i < numRnd; ++i)
      bitsTotal += (numBitsList[i] = random()%32 + 1);
    offset = offsetStart;
    gt_bsStoreNonUniformUIntArray(bitStore, offset, numRnd, bitsTotal,
                                     numBitsList, randSrc);
    for (i = 0; i < numRnd; ++i)
    {
      unsigned numBits = numBitsList[i];
      unsigned mask = (numBits < 32)?
        ~((~(unsigned)0) << numBits):~(unsigned)0;
      unsigned v = randSrc[i] & mask;
      unsigned r = gt_bsGetUInt(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log("Expected %""u"", got %""u"",\n"
                "i = %lu, bits=%u\n",
                v, r, (unsigned long)i, numBits);
        freeResourcesAndReturn(had_err);
      }
      offset += numBits;
    }
    gt_log_log("passed\n");
    gt_log_log("gt_bsStoreNonUniformUIntArray/"
            "gt_bsGetNonUniformUIntArray: ");
    gt_bsGetNonUniformUIntArray(bitStore, offset = offsetStart,
                                   numRnd, bitsTotal, numBitsList, randCmp);
    for (i = 0; i < numRnd; ++i)
    {
      unsigned numBits = numBitsList[i];
      unsigned mask = (numBits < 32)?
        ~((~(unsigned)0) << numBits):~(unsigned)0;
      unsigned v = randSrc[i] & mask,
        r = randCmp[i];
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log( "Expected %""u"", got %""u"",\n"
                " i = %lu, bits=%u\n",
                v, r, (unsigned long)i, numBits);
        freeResourcesAndReturn(had_err);
      }
    }
    if (numRnd > 1)
    {
      unsigned numBits = numBitsList[0];
      unsigned mask = (numBits < 32)?
        ~((~(unsigned)0) << numBits):~(unsigned)0;
      unsigned v = randSrc[0] & mask;
      unsigned r;
      gt_bsGetNonUniformUIntArray(bitStore, offsetStart, 1, numBits,
                                     numBitsList, &r);
      if (r != v)
      {
        gt_log_log("Expected %""u"", got %""u"", "
                " one value extraction\n",
                v, r);
        freeResourcesAndReturn(had_err);
      }
    }
    gt_log_log(" passed\n");
    gt_free(numBitsList);
    numBitsList = NULL;
  }
  gt_log_log("bsNonStoreUniformIntArray/gt_bsGetInt: ");
  {
    BitOffset bitsTotal = 0;
    numBitsList = gt_malloc(sizeof (unsigned) * numRnd);
    for (i = 0; i < numRnd; ++i)
      bitsTotal += (numBitsList[i] = random()%32 + 1);
    offset = offsetStart;
    gt_bsStoreNonUniformIntArray(bitStore, offset, numRnd, bitsTotal,
                                     numBitsList, (int *)randSrc);
    for (i = 0; i < numRnd; ++i)
    {
      unsigned numBits = numBitsList[i];
      int mask = (numBits < 32)
        ? ~((~(int)0) << numBits) : ~(int)0;
      int m = (int)1 << (numBits - 1);
      int v = (int)((randSrc[i] & mask) ^ m) - m;
      int r = gt_bsGetInt(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log("Expected %""d"", got %""d"",\n"
                    "i = %lu, numBits=%u\n",
                    v, r, (unsigned long)i, numBits);
        freeResourcesAndReturn(had_err);
      }
      offset += numBits;
    }
    gt_log_log("passed\n");
    gt_log_log("gt_bsStoreNonUniformIntArray/"
            "gt_bsGetNonUniformIntArray: ");
    gt_bsGetNonUniformIntArray(bitStore, offset = offsetStart, numRnd,
                                   bitsTotal, numBitsList,
                                   (int *)randCmp);
    for (i = 0; i < numRnd; ++i)
    {
      unsigned numBits = numBitsList[i];
      int mask = (numBits < 32)
        ? ~((~(int)0) << numBits) : ~(int)0;
      int m = (int)1 << (numBits - 1);
      int v = (int)((randSrc[i] & mask) ^ m) - m;
      int r = randCmp[i];
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log("Expected %""d"", got %""d"", i = %lu\n",
                v, r, (unsigned long)i);
        freeResourcesAndReturn(had_err);
      }
    }
    if (numRnd > 0)
    {
      unsigned numBits = numBitsList[0];
      int mask = (numBits < 32)
        ? ~((~(int)0) << numBits) : ~(int)0;
      int m = (int)1 << (numBits - 1);
      int v = (int)((randSrc[0] & mask) ^ m) - m;
      int r = 0;
      gt_bsGetNonUniformIntArray(bitStore, offsetStart,
                                     1, numBits, numBitsList, &r);
      ensure(had_err, r == v);
      if (had_err)
      {
        gt_log_log("Expected %""d"", got %""d"
                ", one value extraction\n",
                v, r);
        freeResourcesAndReturn(had_err);
      }
    }
    gt_log_log("passed\n");
    gt_free(numBitsList);
    numBitsList = NULL;
  }

  if (numRnd > 0)
  {
    gt_log_log("gt_bsCopy: ");
    {
      /* first decide how many of the values to use and at which to start */
      size_t numValueCopies, copyStart;
      BitOffset numCopyBits = 0, destOffset;
      unsigned numBits = random()%32 + 1;
      unsigned mask = ~(unsigned)0;
      if (numBits < 32)
        mask = ~(mask << numBits);
      if (random()&1)
      {
        numValueCopies = random()%(numRnd + 1);
        copyStart = random()%(numRnd - numValueCopies + 1);
      }
      else
      {
        copyStart = random() % numRnd;
        numValueCopies = random()%(numRnd - copyStart) + 1;
      }
      gt_assert(copyStart + numValueCopies <= numRnd);
      offset = offsetStart + (BitOffset)copyStart * numBits;
      gt_bsStoreUniformUIntArray(bitStore, offset, numBits,
                                       numValueCopies, randSrc);
      destOffset = random()%(offsetStart + 32
                             * (BitOffset)(numRnd - numValueCopies) + 1);
      numCopyBits = (BitOffset)numBits * numValueCopies;
      /* the following gt_bsCopy should be equivalent to:
       * gt_bsStoreUniformUIntArray(bitStoreCopy, destOffset,
       *                              numBits, numValueCopies, randSrc); */
      gt_bsCopy(bitStore, offset, bitStoreCopy, destOffset, numCopyBits);
      ensure(had_err,
             gt_bsCompare(bitStore, offset, numCopyBits,
                       bitStoreCopy, destOffset, numCopyBits) == 0);
      if (had_err)
      {
        gt_log_log("Expected equality on bitstrings\n"
                    "offset = %llu, destOffset = %llu,"
                    " numCopyBits=%llu\n",
                    (unsigned long long)offset,
                    (unsigned long long)destOffset,
                    (unsigned long long)numCopyBits);
        /* FIXME: implement bitstring output function */
        freeResourcesAndReturn(had_err);
      }
      gt_log_log("passed\n");
    }
  }
  if (numRnd > 0)
  {
    gt_log_log("gt_bsClear: ");
    {
      /* first decide how many of the values to use and at which to start */
      size_t numResetValues, resetStart;
      BitOffset numResetBits = 0;
      unsigned numBits = random()%32 + 1;
      int bitVal = random()&1;
      int cmpVal = bitVal?-1:0;
      unsigned mask = ~(unsigned)0;
      if (numBits < 32)
        mask = ~(mask << numBits);
      if (random()&1)
      {
        numResetValues = random()%(numRnd + 1);
        resetStart = random()%(numRnd - numResetValues + 1);
      }
      else
      {
        resetStart = random() % numRnd;
        numResetValues = random()%(numRnd - resetStart) + 1;
      }
      gt_assert(resetStart + numResetValues <= numRnd);
      offset = offsetStart;
      gt_bsStoreUniformIntArray(bitStore, offset, numBits, numRnd,
                                    (int *)randSrc);
      numResetBits = (BitOffset)numBits * numResetValues;
      gt_bsClear(bitStore, offset + (BitOffset)resetStart * numBits,
              numResetBits, bitVal);
      {
        int m = (int)1 << (numBits - 1);
        for (i = 0; i < resetStart; ++i)
        {
          int v = (int)((randSrc[i] & mask) ^ m) - m;
          int r = gt_bsGetInt(bitStore, offset, numBits);
          ensure(had_err, r == v);
          if (had_err)
          {
            gt_log_log( "Expected %""d"", got %""d"",\n"
                     "i = %lu, numBits=%u\n",
                     v, r, (unsigned long)i, numBits);
            freeResourcesAndReturn(had_err);
          }
          offset += numBits;
        }
        for (; i < resetStart + numResetValues; ++i)
        {
          int r = gt_bsGetInt(bitStore, offset, numBits);
          ensure(had_err, r == cmpVal);
          if (had_err)
          {
            gt_log_log("Expected %""d"", got %""d"",\n"
                    "i = %lu, numBits=%u\n",
                    cmpVal, r, (unsigned long)i, numBits);
            freeResourcesAndReturn(had_err);
          }
          offset += numBits;
        }
        for (; i < numRnd; ++i)
        {
          int v = (int)((randSrc[i] & mask) ^ m) - m;
          int r = gt_bsGetInt(bitStore, offset, numBits);
          ensure(had_err, r == v);
          if (had_err)
          {
            gt_log_log("Expected %""d"", got %""d"",\n"
                    "i = %lu, numBits=%u\n",
                    v, r, (unsigned long)i, numBits);
            freeResourcesAndReturn(had_err);
          }
          offset += numBits;
        }
      }
    }
    gt_log_log("passed\n");
  }
  if (numRnd > 0)
  {
    gt_log_log("gt_bs1BitsCount: ");
    {
      /* first decide how many of the values to use and at which to start */
      size_t numCountValues, countStart;
      BitOffset numCountBits = 0, bitCountRef = 0, bitCountCmp;
      unsigned numBits = random()%32 + 1;
      unsigned mask = ~(unsigned)0;
      if (numBits < 32)
        mask = ~(mask << numBits);
      if (random()&1)
      {
        numCountValues = random()%(numRnd + 1);
        countStart = random()%(numRnd - numCountValues + 1);
      }
      else
      {
        countStart = random() % numRnd;
        numCountValues = random()%(numRnd - countStart) + 1;
      }
      gt_assert(countStart + numCountValues <= numRnd);
      offset = offsetStart;
      gt_bsStoreUniformUIntArray(bitStore, offset, numBits, numRnd,
                                       randSrc);
      numCountBits = (BitOffset)numBits * numCountValues;
      bitCountCmp = gt_bs1BitsCount(bitStore,
                                 offset + (BitOffset)countStart * numBits,
                                 numCountBits);
      for (i = countStart; i < countStart + numCountValues; ++i)
      {
        unsigned v = (unsigned)randSrc[i] & mask;
        bitCountRef += genBitCount_unsigned(v);
      }
      ensure(had_err, bitCountRef == bitCountCmp);
      if (had_err)
      {
        gt_log_log("Expected %llu, got %llu,\n"
                "numBits=%u\n", (unsigned long long)bitCountRef,
                (unsigned long long)bitCountCmp, numBits);
        freeResourcesAndReturn(had_err);
      }
      offset += numBits;
    }
    gt_log_log("passed\n");
  }
  freeResourcesAndReturn(had_err);
}
