/*
  Copyright (c) 2007 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
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

#include <stdlib.h>
#include <stdio.h>
#include "core/assert_api.h"
#include "spacedef.h"
#include "intcode-def.h"
#include "core/encodedsequence.h"
#include "sfx-enumcodes.h"
#include "stamp.h"
#include "initbasepower.h"

struct Enumcodeatposition
{
  GtRange previousrange;
  GtSpecialrangeiterator *sri;
  bool moveforward;
  unsigned long totallength;
  bool exhausted;
  const GtEncodedsequence *encseq;
  GtReadmode readmode;
  unsigned int prefixlength;
  GtCodetype **multimappower, *filltable;
};

Enumcodeatposition *newEnumcodeatposition(const GtEncodedsequence *encseq,
                                          GtReadmode readmode,
                                          unsigned int prefixlength,
                                          unsigned int numofchars)
{
  Enumcodeatposition *ecp;

  ALLOCASSIGNSPACE(ecp,NULL,Enumcodeatposition,1);
  ecp->encseq = encseq;
  ecp->readmode = readmode;
  ecp->multimappower = initmultimappower(numofchars,prefixlength);
  ecp->filltable = initfilltable(numofchars,prefixlength);
  ecp->prefixlength = prefixlength;
  ecp->moveforward = GT_ISDIRREVERSE(readmode) ? true : false;
  ecp->totallength = gt_encodedsequence_total_length(encseq);
  if (ecp->moveforward)
  {
    ecp->previousrange.start = ecp->previousrange.end = 0;
  } else
  {
    ecp->previousrange.start = ecp->previousrange.end = ecp->totallength;
  }
  ecp->exhausted = false;
  if (gt_encodedsequence_has_specialranges(encseq))
  {
    ecp->sri = gt_specialrangeiterator_new(encseq,ecp->moveforward);
  } else
  {
    ecp->sri = NULL;
  }
  return ecp;
}

static bool newcodelistelem(Specialcontext *specialcontext,
                            unsigned long smallerval,
                            unsigned long largerval,
                            const Enumcodeatposition *ecp)
{
  if (smallerval < largerval)
  {
    unsigned long distance = largerval - smallerval;

    if (distance > (unsigned long) (ecp->prefixlength-1))
    {
      distance = (unsigned long) (ecp->prefixlength-1);
    }
    specialcontext->maxprefixindex = (unsigned int) distance;
    if (ecp->moveforward)
    {
      specialcontext->position = ecp->totallength - smallerval;
    } else
    {
      specialcontext->position = largerval;
    }
    gt_assert(specialcontext->position >=
              (unsigned long) specialcontext->maxprefixindex);
    return true;
  }
  return false;
}

bool nextEnumcodeatposition(Specialcontext *specialcontext,
                            Enumcodeatposition *ecp)
{
  GtRange currentrange;
  bool done;

  if (ecp->exhausted)
  {
    return false;
  }
  while (ecp->sri != NULL)
  {
    if (!gt_specialrangeiterator_next(ecp->sri,&currentrange))
    {
      gt_specialrangeiterator_delete(ecp->sri);
      ecp->sri = NULL;
      break;
    }
    if (ecp->moveforward)
    {
      if (newcodelistelem(specialcontext,
                          ecp->previousrange.end,
                          currentrange.start,
                          ecp))
      {
        ecp->previousrange = currentrange;
        return true;
      }
    } else
    {
      if (newcodelistelem(specialcontext,
                          currentrange.end,
                          ecp->previousrange.start,
                          ecp))
      {
        ecp->previousrange = currentrange;
        return true;
      }
    }
    ecp->previousrange = currentrange;
  }
  ecp->exhausted = true;
  if (ecp->moveforward)
  {
    done = newcodelistelem(specialcontext,
                           ecp->previousrange.end,
                           ecp->totallength,
                           ecp);
  } else
  {
    done = newcodelistelem(specialcontext,
                           0,
                           ecp->previousrange.start,
                           ecp);
  }
  return done;
}

void freeEnumcodeatposition(Enumcodeatposition **ecp)
{
  FREESPACE((*ecp)->filltable);
  multimappowerfree(&(*ecp)->multimappower);
  FREESPACE(*ecp);
}

GtCodetype computefilledqgramcode(const Enumcodeatposition *ecp,
                                unsigned int prefixindex,
                                unsigned long pos)
{
  GtCodetype code;
  unsigned int idx;
  GtUchar cc;

  gt_assert(prefixindex > 0 && prefixindex < ecp->prefixlength);
  code = ecp->filltable[prefixindex];
  for (idx=0; idx<prefixindex; idx++)
  {
    gt_assert((unsigned long) (pos + idx) < ecp->totallength);
    cc = gt_encodedsequence_getencodedcharnospecial(ecp->encseq,
                                                    pos + idx,
                                                    ecp->readmode);
    gt_assert(ISNOTSPECIAL(cc));
    code += ecp->multimappower[idx][cc];
  }
  return code;
}

bool computefilledqgramcodestopatmax(GtCodetype *code,
                                     const Enumcodeatposition *ecp,
                                     unsigned int prefixindex,
                                     unsigned long pos,
                                     GtCodetype stopcode)
{
  GtCodetype tmpcode;
  unsigned int idx;
  GtUchar cc;

  gt_assert(prefixindex > 0 && prefixindex < ecp->prefixlength);
  tmpcode = ecp->filltable[prefixindex];
  if (tmpcode > stopcode)
  {
    return false;
  }
  for (idx=0; idx<prefixindex; idx++)
  {
    gt_assert((unsigned long) (pos + idx) < ecp->totallength);
    cc = gt_encodedsequence_getencodedcharnospecial(ecp->encseq,
                                                    pos + idx,
                                                    ecp->readmode);
    gt_assert(ISNOTSPECIAL(cc));
    tmpcode += ecp->multimappower[idx][cc];
    if (tmpcode > stopcode)
    {
      return false;
    }
  }
  *code = tmpcode;
  return true;
}
