/*
  Copyright (c) 2007 David Ellinghaus <d.ellinghaus@ikmb.uni-kiel.de>
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

#include "core/fa.h"
#include "core/str.h"
#include "match/echoseq.h"
#include "core/encodedsequence.h"
#include "core/defined-types.h"
#include "ltrharvest-opt.h"
#include "repeattypes.h"
#include "outputgff3.h"

typedef struct
{
  unsigned long idcounterRepregion,
                idcounterRetrotrans,
                idcounterLTR,
                idcounterTSD,
                idcounterMotif;
} LTRcounter;

static void showboundaries(FILE *fp,
                           const LTRharvestoptions *lo,
                           const LTRboundaries *boundaries,
                           unsigned long offset,
                           LTRcounter *ltrc)
{

  /* repeat-region */
  fprintf(fp, "seq%lu\tLTRharvest\trepeat_region\t%lu"
              "\t%lu\t" ".\t?\t.\tID=RepeatReg%lu\n",
      boundaries->contignumber,
      /* increase boundary position by one for output */
      boundaries->leftLTR_5 -offset + 1
                        - boundaries->lenleftTSD + (unsigned long) lo->offset,
      boundaries->rightLTR_3 -offset + 1
                        + boundaries->lenrightTSD + (unsigned long) lo->offset,
      ltrc->idcounterRepregion++ );

  /* LTR retrotransposon */
  fprintf(fp, "seq%lu\tLTRharvest\tLTR_retrotransposon\t"
              "%lu\t%lu\t"
              ".\t?\t.\tID=LTRret%lu;Parent=RepeatReg%lu\n",
      boundaries->contignumber,
      /* increase boundary position by one for output */
      boundaries->leftLTR_5 -offset + 1 + (unsigned long) lo->offset,
      boundaries->rightLTR_3 -offset  + 1 + (unsigned long) lo->offset,
      ltrc->idcounterRetrotrans++,
      ltrc->idcounterRepregion-1 );

  /* LTRs */
  fprintf(fp, "seq%lu\tLTRharvest\tlong_terminal_repeat\t"
              "%lu\t%lu\t"
              ".\t?\t.\tID=LTR%lu;Parent=LTRret%lu\n",
      boundaries->contignumber,
      /* increase boundary position by one for output */
      boundaries->leftLTR_5 -offset + 1 + (unsigned long) lo->offset,
      boundaries->leftLTR_3 -offset + 1 + (unsigned long) lo->offset,
      ltrc->idcounterLTR++,
      ltrc->idcounterRetrotrans-1 );
  fprintf(fp, "seq%lu\tLTRharvest\tlong_terminal_repeat\t"
              "%lu\t%lu\t"
              ".\t?\t.\tID=LTR%lu;Parent=LTRret%lu\n",
      boundaries->contignumber,
      /* increase boundary position by one for output */
      boundaries->rightLTR_5 -offset + 1 + (unsigned long) lo->offset,
      boundaries->rightLTR_3 -offset + 1 + (unsigned long) lo->offset,
      ltrc->idcounterLTR++,
      ltrc->idcounterRetrotrans-1 );

  if (lo->minlengthTSD > 1U)
  {
    /* TSDs */
    fprintf(fp, "seq%lu\tLTRharvest\ttarget_site_duplication\t"
                "%lu\t%lu\t"
                ".\t?\t.\tID=TSD%lu;Parent=RepeatReg%lu\n",
        boundaries->contignumber,
        /* increase boundary position by one for output */
        boundaries->leftLTR_5 -offset + 1
                          - boundaries->lenleftTSD + (unsigned long) lo->offset,
        boundaries->leftLTR_5 -offset + (unsigned long) lo->offset,
        ltrc->idcounterTSD++,
        ltrc->idcounterRepregion-1 );

    fprintf(fp, "seq%lu\tLTRharvest\ttarget_site_duplication\t"
                "%lu\t%lu\t"
                ".\t?\t.\tID=TSD%lu;Parent=RepeatReg%lu\n",
        boundaries->contignumber,
        /* increase boundary position by one for output */
        boundaries->rightLTR_3 -offset + 2
                          + (unsigned long) lo->offset,
        boundaries->rightLTR_3 -offset + 1
                          + boundaries->lenrightTSD
                          + (unsigned long) lo->offset,
        ltrc->idcounterTSD++,
        ltrc->idcounterRepregion-1 );
  }

  if (lo->motif.allowedmismatches < 4U)
  {
    fprintf(fp, "seq%lu\tLTRharvest\tinverted_repeat\t"
                "%lu\t%lu\t"
                ".\t?\t.\tID=Motif%lu;Parent=RepeatReg%lu\n",
        boundaries->contignumber,
        /* increase boundary position by one for output */
        boundaries->leftLTR_5 -offset + 1 + (unsigned long) lo->offset,
        boundaries->leftLTR_5 -offset + 2 + (unsigned long) lo->offset,
        ltrc->idcounterMotif++,
        ltrc->idcounterRepregion-1 );
    fprintf(fp, "seq%lu\tLTRharvest\tinverted_repeat\t"
                "%lu\t%lu\t"
                ".\t?\t.\tID=Motif%lu;Parent=RepeatReg%lu\n",
        boundaries->contignumber,
        /* increase boundary position by one for output */
        boundaries->leftLTR_3 -offset + (unsigned long) lo->offset,
        boundaries->leftLTR_3 -offset + 1 + (unsigned long) lo->offset,
        ltrc->idcounterMotif++,
        ltrc->idcounterRepregion-1 );

    fprintf(fp, "seq%lu\tLTRharvest\tinverted_repeat\t"
                "%lu\t%lu\t"
                ".\t?\t.\tID=Motif%lu;Parent=RepeatReg%lu\n",
        boundaries->contignumber,
        /* increase boundary position by one for output */
        boundaries->rightLTR_5 -offset + 1 + (unsigned long) lo->offset,
        boundaries->rightLTR_5 -offset + 2 + (unsigned long) lo->offset,
        ltrc->idcounterMotif++,
        ltrc->idcounterRepregion-1 );
    fprintf(fp, "seq%lu\tLTRharvest\tinverted_repeat\t"
                "%lu\t%lu\t"
                ".\t?\t.\tID=Motif%lu;Parent=RepeatReg%lu\n",
        boundaries->contignumber,
        /* increase boundary position by one for output */
        boundaries->rightLTR_3 -offset + (unsigned long) lo->offset,
        boundaries->rightLTR_3 -offset + 1 + (unsigned long) lo->offset,
        ltrc->idcounterMotif++,
        ltrc->idcounterRepregion-1 );
  }
}

int gt_printgff3format(const LTRharvestoptions *lo,
                    const LTRboundaries **bdptrtab,
                    unsigned long numofboundaries,
                    const GtEncodedsequence *encseq,
                    GtError *err)
{
  bool haserr = false;
  FILE *fp;

  fp = gt_fa_fopen(gt_str_get(lo->str_gff3filename), "w", err);
  if (fp == NULL)
  {
    haserr = true;
  } else
  {
    LTRcounter ltrc;
    GtSeqinfo seqinfo;
    const char *desptr = NULL;
    Definedunsignedlong previouscontignum = {false, 0};
    unsigned long seqnum, i, desclen;

    ltrc.idcounterRepregion = ltrc.idcounterRetrotrans
                            = ltrc.idcounterLTR
                            = ltrc.idcounterTSD
                            = ltrc.idcounterMotif = 0;
    fprintf(fp, "##gff-version 3\n");
    for (i = 0; i<numofboundaries; i++)
    {
      seqnum = bdptrtab[i]->contignumber;
      if (!previouscontignum.defined ||
          previouscontignum.valueunsignedlong != seqnum)
      {
        previouscontignum.defined = true;
        previouscontignum.valueunsignedlong = seqnum;
        gt_encodedsequence_seqinfo(encseq,&seqinfo,seqnum);
        fprintf(fp, "##sequence-region seq%lu %lu %lu\n",
                    seqnum,
                    1 + (unsigned long) lo->offset,
                    seqinfo.seqlength + (unsigned long) lo->offset);
        desptr = gt_encodedsequence_description(encseq, &desclen, seqnum);
        fprintf(fp,"# %*.*s\n",(int) desclen,(int) desclen,desptr);
      }
      showboundaries(fp,
                     lo,
                     bdptrtab[i],
                     seqinfo.seqstartpos,
                     &ltrc);
    }
  }
  gt_fa_xfclose(fp);
  return haserr ? -1 : 0;
}
