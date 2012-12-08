static unsigned long gt_sain_PLAINSEQ_insertSstarsuffixes(GtSainseq *sainseq,
                                                 unsigned long *suftab)
{
  unsigned long position,
                nextcc = GT_UNIQUEINT(sainseq->totallength),
                countSstartype = 0,
                *fillptr = sainseq->bucketfillptr;
  GtSainbuffer *sainbuffer = gt_sainbuffer_new(suftab,fillptr,
                                               sainseq->numofchars);
  bool nextisStype = true;

  gt_sain_endbuckets(sainseq);
  for (position = sainseq->totallength-1; /* Nothing */; position--)
  {
    unsigned long currentcc = (unsigned long)
sainseq->seq.plainseq[position];
    bool currentisStype = (currentcc < nextcc ||
                           (currentcc == nextcc && nextisStype)) ? true : false;
    if (!currentisStype && nextisStype)
    {
      countSstartype++;
      if (sainseq->sstarfirstcharcount != NULL)
      {
        sainseq->sstarfirstcharcount[nextcc]++;
      }
      if (sainbuffer != NULL)
      {
        gt_sainbuffer_update(sainbuffer,nextcc,position);
      } else
      {
        suftab[--fillptr[nextcc]] = position;
      }
#undef SAINSHOWSTATE
#ifdef SAINSHOWSTATE
      printf("Sstar.suftab[%lu]=%lu\n",fillptr[nextcc],position+1);
#endif
    }
    nextisStype = currentisStype;
    nextcc = currentcc;
    if (position == 0)
    {
      break;
    }
  }
  gt_sainbuffer_flushall(sainbuffer);
  gt_sainbuffer_delete(sainbuffer);
  gt_assert(GT_MULT2(countSstartype) <= sainseq->totallength);
  return countSstartype;
}

static void gt_sain_PLAINSEQ_induceLtypesuffixes1(GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr;
  long *bucketptr = NULL;
  long *suftabptr;

  fillptr = sainseq->bucketfillptr;
  for (suftabptr = suftab, sainseq->currentround = 0;
       suftabptr < suftab + nonspecialentries; suftabptr++)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      unsigned long currentcc;

      if (position >= (long) sainseq->totallength)
      {
        gt_assert(sainseq->roundtable != NULL);
        sainseq->currentround++;
        position -= (long) sainseq->totallength;
      }
      currentcc = (unsigned long)
sainseq->seq.plainseq[(unsigned long) position];
      if (currentcc < sainseq->numofchars)
      {
        if (position > 0)
        {
          unsigned long leftcontextcc;

          gt_assert(position > 0);
          position--;
          leftcontextcc = (unsigned long)
sainseq->seq.plainseq[(unsigned long) position];
          if (sainseq->roundtable != NULL)
          {
            unsigned long t = (currentcc << 1) |
                              (leftcontextcc < currentcc ? 1UL : 0);

            gt_assert(currentcc > 0 &&
                      sainseq->roundtable[t] <= sainseq->currentround);
            if (sainseq->roundtable[t] < sainseq->currentround)
            {
              position += (long) sainseq->totallength;
              sainseq->roundtable[t] = sainseq->currentround;
            }
          }
          GT_SAINUPDATEBUCKETPTR(currentcc);
          /* negative => position does not derive L-suffix
             positive => position may derive L-suffix */
          gt_assert(suftabptr < bucketptr);
          *bucketptr++ = (leftcontextcc < currentcc) ? ~position : position;
          *suftabptr = 0;
#ifdef SAINSHOWSTATE
          gt_assert(bucketptr != NULL);
          printf("L-induce: suftab[%lu]=%ld\n",
                  (unsigned long) (bucketptr-1-suftab),*(bucketptr-1));
#endif
        }
      } else
      {
        *suftabptr = 0;
      }
    } else
    {
      if (position < 0)
      {
        *suftabptr = ~position;
      }
    }
  }
}

static void gt_sain_PLAINSEQ_induceStypesuffixes1(GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  long *suftabptr;

  gt_sain_special_singleSinduction1(sainseq,
                                    suftab,
                                    (long) (sainseq->totallength-1));
  if (sainseq->seqtype == GT_SAIN_ENCSEQ)
  {
    gt_sain_induceStypes1fromspecialranges(sainseq,
                                           sainseq->seq.encseq,
                                           suftab);
  }
  if (nonspecialentries == 0)
  {
    return;
  }
  for (suftabptr = suftab + nonspecialentries - 1; suftabptr >= suftab;
       suftabptr--)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      if (position >= (long) sainseq->totallength)
      {
        gt_assert(sainseq->roundtable != NULL);
        sainseq->currentround++;
        position -= (long) sainseq->totallength;
      }
      if (position > 0)
      {
        unsigned long currentcc
          = (unsigned long)
sainseq->seq.plainseq[(unsigned long) position];

        if (currentcc < sainseq->numofchars)
        {
          unsigned long leftcontextcc;

          position--;
          leftcontextcc
            = (unsigned long)
sainseq->seq.plainseq[(unsigned long) position];
          if (sainseq->roundtable != NULL)
          {
            unsigned long t = (currentcc << 1) |
                              (leftcontextcc > currentcc ? 1UL : 0);

            gt_assert(sainseq->roundtable[t] <= sainseq->currentround);
            if (sainseq->roundtable[t] < sainseq->currentround)
            {
              position += sainseq->totallength;
              sainseq->roundtable[t] = sainseq->currentround;
            }
          }
          GT_SAINUPDATEBUCKETPTR(currentcc);
          gt_assert(bucketptr != NULL && bucketptr - 1 < suftabptr);
          *(--bucketptr) = (leftcontextcc > currentcc)
                            ? ~(position+1) : position;
#ifdef SAINSHOWSTATE
          printf("S-induce: suftab[%lu]=%ld\n",
                  (unsigned long) (bucketptr - suftab),*bucketptr);
#endif
        }
      }
      *suftabptr = 0;
    }
  }
}

static void gt_sain_PLAINSEQ_induceLtypesuffixes2(const GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  long *suftabptr;

  for (suftabptr = suftab; suftabptr < suftab + nonspecialentries; suftabptr++)
  {
    long position = *suftabptr;

    *suftabptr = ~position;
    if (position > 0)
    {
      unsigned long currentcc;

      position--;
      currentcc = (unsigned long)
sainseq->seq.plainseq[(unsigned long) position];
      if (currentcc < sainseq->numofchars)
      {
        gt_assert(currentcc > 0);
        GT_SAINUPDATEBUCKETPTR(currentcc);
        gt_assert(bucketptr != NULL && suftabptr < bucketptr);
        *bucketptr++ = (position > 0 &&
                        ((unsigned long)
sainseq->seq.plainseq[(unsigned long) (position-1)])
                                            < currentcc)
                        ? ~position : position;
#ifdef SAINSHOWSTATE
        gt_assert(bucketptr != NULL);
        printf("L-induce: suftab[%lu]=%ld\n",
               (unsigned long) (bucketptr-1-suftab),*(bucketptr-1));
#endif
      }
    }
  }
}

static void gt_sain_PLAINSEQ_induceStypesuffixes2(const GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  long *suftabptr;

  gt_sain_special_singleSinduction2(sainseq,
                                    suftab,
                                    (long) sainseq->totallength,
                                    nonspecialentries);
  if (sainseq->seqtype == GT_SAIN_ENCSEQ)
  {
    gt_sain_induceStypes2fromspecialranges(sainseq,
                                           sainseq->seq.encseq,
                                           suftab,
                                           nonspecialentries);
  }
  if (nonspecialentries == 0)
  {
    return;
  }
  for (suftabptr = suftab + nonspecialentries - 1; suftabptr >= suftab;
       suftabptr--)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      unsigned long currentcc;

      position--;
      currentcc = (unsigned long)
sainseq->seq.plainseq[(unsigned long) position];
      if (currentcc < sainseq->numofchars)
      {
        GT_SAINUPDATEBUCKETPTR(currentcc);
        gt_assert(bucketptr != NULL && bucketptr - 1 < suftabptr);
        *(--bucketptr) = (position == 0 ||
                          ((unsigned long)
sainseq->seq.plainseq[(unsigned long) (position-1)])
                                             > currentcc)
                         ? ~position : position;
#ifdef SAINSHOWSTATE
        gt_assert(bucketptr != NULL);
        printf("S-induce: suftab[%lu]=%ld\n",
                (unsigned long) (bucketptr-suftab),*bucketptr);
#endif
      }
    } else
    {
      *suftabptr = ~position;
    }
  }
}

static void gt_sain_PLAINSEQ_expandorder2original(GtSainseq *sainseq,
                                         unsigned long numberofsuffixes,
                                         unsigned long *suftab)
{
  unsigned long *suftabptr, position,
                writeidx = numberofsuffixes - 1,
                nextcc = GT_UNIQUEINT(sainseq->totallength),
                *sstarsuffixes = suftab + numberofsuffixes;
  unsigned long *sstarfirstcharcount = NULL, *bucketsize = NULL;
  bool nextisStype = true;

  if (sainseq->seqtype == GT_SAIN_INTSEQ)
  {
    unsigned long charidx;

    gt_assert(sainseq->sstarfirstcharcount == NULL);
    sstarfirstcharcount = sainseq->sstarfirstcharcount
                        = sainseq->bucketfillptr;
    bucketsize = sainseq->bucketsize;
    for (charidx = 0; charidx < sainseq->numofchars; charidx++)
    {
      sstarfirstcharcount[charidx] = 0;
      bucketsize[charidx] = 0;
    }
  }
  for (position = sainseq->totallength-1; /* Nothing */; position--)
  {
    unsigned long currentcc = (unsigned long)
sainseq->seq.plainseq[position];
    bool currentisStype = (currentcc < nextcc ||
                           (currentcc == nextcc && nextisStype)) ? true : false;

    if (!currentisStype && nextisStype)
    {
      if (sstarfirstcharcount != NULL)
      {
        sstarfirstcharcount[nextcc]++;
      }
      sstarsuffixes[writeidx--] = position+1;
    }
    if (bucketsize != NULL)
    {
      bucketsize[currentcc]++;
    }
    nextisStype = currentisStype;
    nextcc = currentcc;
    if (position == 0)
    {
      break;
    }
  }
  for (suftabptr = suftab; suftabptr < suftab + numberofsuffixes; suftabptr++)
  {
    *suftabptr = sstarsuffixes[*suftabptr];
  }
}
static unsigned long gt_sain_ENCSEQ_insertSstarsuffixes(GtSainseq *sainseq,
                                                 unsigned long *suftab)
{
  unsigned long position,
                nextcc = GT_UNIQUEINT(sainseq->totallength),
                countSstartype = 0,
                *fillptr = sainseq->bucketfillptr;
  GtSainbuffer *sainbuffer = gt_sainbuffer_new(suftab,fillptr,
                                               sainseq->numofchars);
  GtUchar tmpcc;
bool nextisStype = true;

  gt_sain_endbuckets(sainseq);
  for (position = sainseq->totallength-1; /* Nothing */; position--)
  {
    unsigned long currentcc = ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
position,
sainseq->readmode))
  ? GT_UNIQUEINT(position) : (unsigned long) tmpcc;
    bool currentisStype = (currentcc < nextcc ||
                           (currentcc == nextcc && nextisStype)) ? true : false;
    if (!currentisStype && nextisStype)
    {
      countSstartype++;
      if (sainseq->sstarfirstcharcount != NULL)
      {
        sainseq->sstarfirstcharcount[nextcc]++;
      }
      if (sainbuffer != NULL)
      {
        gt_sainbuffer_update(sainbuffer,nextcc,position);
      } else
      {
        suftab[--fillptr[nextcc]] = position;
      }
#undef SAINSHOWSTATE
#ifdef SAINSHOWSTATE
      printf("Sstar.suftab[%lu]=%lu\n",fillptr[nextcc],position+1);
#endif
    }
    nextisStype = currentisStype;
    nextcc = currentcc;
    if (position == 0)
    {
      break;
    }
  }
  gt_sainbuffer_flushall(sainbuffer);
  gt_sainbuffer_delete(sainbuffer);
  gt_assert(GT_MULT2(countSstartype) <= sainseq->totallength);
  return countSstartype;
}

static void gt_sain_ENCSEQ_induceLtypesuffixes1(GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr;
  long *bucketptr = NULL;
  GtUchar tmpcc;
long *suftabptr;

  fillptr = sainseq->bucketfillptr;
  for (suftabptr = suftab, sainseq->currentround = 0;
       suftabptr < suftab + nonspecialentries; suftabptr++)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      unsigned long currentcc;

      if (position >= (long) sainseq->totallength)
      {
        gt_assert(sainseq->roundtable != NULL);
        sainseq->currentround++;
        position -= (long) sainseq->totallength;
      }
      currentcc = ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
(unsigned long) position,
sainseq->readmode))
  ? GT_UNIQUEINT((unsigned long) position) : (unsigned long) tmpcc;
      if (currentcc < sainseq->numofchars)
      {
        if (position > 0)
        {
          unsigned long leftcontextcc;

          gt_assert(position > 0);
          position--;
          leftcontextcc = ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
(unsigned long) position,
sainseq->readmode))
  ? GT_UNIQUEINT((unsigned long) position) : (unsigned long) tmpcc;
          if (sainseq->roundtable != NULL)
          {
            unsigned long t = (currentcc << 1) |
                              (leftcontextcc < currentcc ? 1UL : 0);

            gt_assert(currentcc > 0 &&
                      sainseq->roundtable[t] <= sainseq->currentround);
            if (sainseq->roundtable[t] < sainseq->currentround)
            {
              position += (long) sainseq->totallength;
              sainseq->roundtable[t] = sainseq->currentround;
            }
          }
          GT_SAINUPDATEBUCKETPTR(currentcc);
          /* negative => position does not derive L-suffix
             positive => position may derive L-suffix */
          gt_assert(suftabptr < bucketptr);
          *bucketptr++ = (leftcontextcc < currentcc) ? ~position : position;
          *suftabptr = 0;
#ifdef SAINSHOWSTATE
          gt_assert(bucketptr != NULL);
          printf("L-induce: suftab[%lu]=%ld\n",
                  (unsigned long) (bucketptr-1-suftab),*(bucketptr-1));
#endif
        }
      } else
      {
        *suftabptr = 0;
      }
    } else
    {
      if (position < 0)
      {
        *suftabptr = ~position;
      }
    }
  }
}

static void gt_sain_ENCSEQ_induceStypesuffixes1(GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  GtUchar tmpcc;
long *suftabptr;

  gt_sain_special_singleSinduction1(sainseq,
                                    suftab,
                                    (long) (sainseq->totallength-1));
  if (sainseq->seqtype == GT_SAIN_ENCSEQ)
  {
    gt_sain_induceStypes1fromspecialranges(sainseq,
                                           sainseq->seq.encseq,
                                           suftab);
  }
  if (nonspecialentries == 0)
  {
    return;
  }
  for (suftabptr = suftab + nonspecialentries - 1; suftabptr >= suftab;
       suftabptr--)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      if (position >= (long) sainseq->totallength)
      {
        gt_assert(sainseq->roundtable != NULL);
        sainseq->currentround++;
        position -= (long) sainseq->totallength;
      }
      if (position > 0)
      {
        unsigned long currentcc
          = ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
(unsigned long) position,
sainseq->readmode))
  ? GT_UNIQUEINT((unsigned long) position) : (unsigned long) tmpcc;

        if (currentcc < sainseq->numofchars)
        {
          unsigned long leftcontextcc;

          position--;
          leftcontextcc
            = ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
(unsigned long) position,
sainseq->readmode))
  ? GT_UNIQUEINT((unsigned long) position) : (unsigned long) tmpcc;
          if (sainseq->roundtable != NULL)
          {
            unsigned long t = (currentcc << 1) |
                              (leftcontextcc > currentcc ? 1UL : 0);

            gt_assert(sainseq->roundtable[t] <= sainseq->currentround);
            if (sainseq->roundtable[t] < sainseq->currentround)
            {
              position += sainseq->totallength;
              sainseq->roundtable[t] = sainseq->currentround;
            }
          }
          GT_SAINUPDATEBUCKETPTR(currentcc);
          gt_assert(bucketptr != NULL && bucketptr - 1 < suftabptr);
          *(--bucketptr) = (leftcontextcc > currentcc)
                            ? ~(position+1) : position;
#ifdef SAINSHOWSTATE
          printf("S-induce: suftab[%lu]=%ld\n",
                  (unsigned long) (bucketptr - suftab),*bucketptr);
#endif
        }
      }
      *suftabptr = 0;
    }
  }
}

static void gt_sain_ENCSEQ_induceLtypesuffixes2(const GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  GtUchar tmpcc;
long *suftabptr;

  for (suftabptr = suftab; suftabptr < suftab + nonspecialentries; suftabptr++)
  {
    long position = *suftabptr;

    *suftabptr = ~position;
    if (position > 0)
    {
      unsigned long currentcc;

      position--;
      currentcc = ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
(unsigned long) position,
sainseq->readmode))
  ? GT_UNIQUEINT((unsigned long) position) : (unsigned long) tmpcc;
      if (currentcc < sainseq->numofchars)
      {
        gt_assert(currentcc > 0);
        GT_SAINUPDATEBUCKETPTR(currentcc);
        gt_assert(bucketptr != NULL && suftabptr < bucketptr);
        *bucketptr++ = (position > 0 &&
                        (ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
(unsigned long) (position-1),
sainseq->readmode))
  ? GT_UNIQUEINT((unsigned long) (position-1)) : (unsigned long) tmpcc)
                                            < currentcc)
                        ? ~position : position;
#ifdef SAINSHOWSTATE
        gt_assert(bucketptr != NULL);
        printf("L-induce: suftab[%lu]=%ld\n",
               (unsigned long) (bucketptr-1-suftab),*(bucketptr-1));
#endif
      }
    }
  }
}

static void gt_sain_ENCSEQ_induceStypesuffixes2(const GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  GtUchar tmpcc;
long *suftabptr;

  gt_sain_special_singleSinduction2(sainseq,
                                    suftab,
                                    (long) sainseq->totallength,
                                    nonspecialentries);
  if (sainseq->seqtype == GT_SAIN_ENCSEQ)
  {
    gt_sain_induceStypes2fromspecialranges(sainseq,
                                           sainseq->seq.encseq,
                                           suftab,
                                           nonspecialentries);
  }
  if (nonspecialentries == 0)
  {
    return;
  }
  for (suftabptr = suftab + nonspecialentries - 1; suftabptr >= suftab;
       suftabptr--)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      unsigned long currentcc;

      position--;
      currentcc = ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
(unsigned long) position,
sainseq->readmode))
  ? GT_UNIQUEINT((unsigned long) position) : (unsigned long) tmpcc;
      if (currentcc < sainseq->numofchars)
      {
        GT_SAINUPDATEBUCKETPTR(currentcc);
        gt_assert(bucketptr != NULL && bucketptr - 1 < suftabptr);
        *(--bucketptr) = (position == 0 ||
                          (ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
(unsigned long) (position-1),
sainseq->readmode))
  ? GT_UNIQUEINT((unsigned long) (position-1)) : (unsigned long) tmpcc)
                                             > currentcc)
                         ? ~position : position;
#ifdef SAINSHOWSTATE
        gt_assert(bucketptr != NULL);
        printf("S-induce: suftab[%lu]=%ld\n",
                (unsigned long) (bucketptr-suftab),*bucketptr);
#endif
      }
    } else
    {
      *suftabptr = ~position;
    }
  }
}

static void gt_sain_ENCSEQ_expandorder2original(GtSainseq *sainseq,
                                         unsigned long numberofsuffixes,
                                         unsigned long *suftab)
{
  unsigned long *suftabptr, position,
                writeidx = numberofsuffixes - 1,
                nextcc = GT_UNIQUEINT(sainseq->totallength),
                *sstarsuffixes = suftab + numberofsuffixes;
  unsigned long *sstarfirstcharcount = NULL, *bucketsize = NULL;
  GtUchar tmpcc;
bool nextisStype = true;

  if (sainseq->seqtype == GT_SAIN_INTSEQ)
  {
    unsigned long charidx;

    gt_assert(sainseq->sstarfirstcharcount == NULL);
    sstarfirstcharcount = sainseq->sstarfirstcharcount
                        = sainseq->bucketfillptr;
    bucketsize = sainseq->bucketsize;
    for (charidx = 0; charidx < sainseq->numofchars; charidx++)
    {
      sstarfirstcharcount[charidx] = 0;
      bucketsize[charidx] = 0;
    }
  }
  for (position = sainseq->totallength-1; /* Nothing */; position--)
  {
    unsigned long currentcc = ISSPECIAL(tmpcc = gt_encseq_get_encoded_char(
sainseq->seq.encseq,
position,
sainseq->readmode))
  ? GT_UNIQUEINT(position) : (unsigned long) tmpcc;
    bool currentisStype = (currentcc < nextcc ||
                           (currentcc == nextcc && nextisStype)) ? true : false;

    if (!currentisStype && nextisStype)
    {
      if (sstarfirstcharcount != NULL)
      {
        sstarfirstcharcount[nextcc]++;
      }
      sstarsuffixes[writeidx--] = position+1;
    }
    if (bucketsize != NULL)
    {
      bucketsize[currentcc]++;
    }
    nextisStype = currentisStype;
    nextcc = currentcc;
    if (position == 0)
    {
      break;
    }
  }
  for (suftabptr = suftab; suftabptr < suftab + numberofsuffixes; suftabptr++)
  {
    *suftabptr = sstarsuffixes[*suftabptr];
  }
}
static unsigned long gt_sain_INTSEQ_insertSstarsuffixes(GtSainseq *sainseq,
                                                 unsigned long *suftab)
{
  unsigned long position,
                nextcc = GT_UNIQUEINT(sainseq->totallength),
                countSstartype = 0,
                *fillptr = sainseq->bucketfillptr;
  GtSainbuffer *sainbuffer = gt_sainbuffer_new(suftab,fillptr,
                                               sainseq->numofchars);
  bool nextisStype = true;

  gt_sain_endbuckets(sainseq);
  for (position = sainseq->totallength-1; /* Nothing */; position--)
  {
    unsigned long currentcc = sainseq->seq.array[position];
    bool currentisStype = (currentcc < nextcc ||
                           (currentcc == nextcc && nextisStype)) ? true : false;
    if (!currentisStype && nextisStype)
    {
      countSstartype++;
      if (sainseq->sstarfirstcharcount != NULL)
      {
        sainseq->sstarfirstcharcount[nextcc]++;
      }
      if (sainbuffer != NULL)
      {
        gt_sainbuffer_update(sainbuffer,nextcc,position);
      } else
      {
        suftab[--fillptr[nextcc]] = position;
      }
#undef SAINSHOWSTATE
#ifdef SAINSHOWSTATE
      printf("Sstar.suftab[%lu]=%lu\n",fillptr[nextcc],position+1);
#endif
    }
    nextisStype = currentisStype;
    nextcc = currentcc;
    if (position == 0)
    {
      break;
    }
  }
  gt_sainbuffer_flushall(sainbuffer);
  gt_sainbuffer_delete(sainbuffer);
  gt_assert(GT_MULT2(countSstartype) <= sainseq->totallength);
  return countSstartype;
}

static void gt_sain_INTSEQ_induceLtypesuffixes1(GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr;
  long *bucketptr = NULL;
  long *suftabptr;

  fillptr = sainseq->bucketfillptr;
  for (suftabptr = suftab, sainseq->currentround = 0;
       suftabptr < suftab + nonspecialentries; suftabptr++)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      unsigned long currentcc;

      if (position >= (long) sainseq->totallength)
      {
        gt_assert(sainseq->roundtable != NULL);
        sainseq->currentround++;
        position -= (long) sainseq->totallength;
      }
      currentcc = sainseq->seq.array[(unsigned long) position];
      if (currentcc < sainseq->numofchars)
      {
        if (position > 0)
        {
          unsigned long leftcontextcc;

          gt_assert(position > 0);
          position--;
          leftcontextcc = sainseq->seq.array[(unsigned long) position];
          if (sainseq->roundtable != NULL)
          {
            unsigned long t = (currentcc << 1) |
                              (leftcontextcc < currentcc ? 1UL : 0);

            gt_assert(currentcc > 0 &&
                      sainseq->roundtable[t] <= sainseq->currentround);
            if (sainseq->roundtable[t] < sainseq->currentround)
            {
              position += (long) sainseq->totallength;
              sainseq->roundtable[t] = sainseq->currentround;
            }
          }
          GT_SAINUPDATEBUCKETPTR(currentcc);
          /* negative => position does not derive L-suffix
             positive => position may derive L-suffix */
          gt_assert(suftabptr < bucketptr);
          *bucketptr++ = (leftcontextcc < currentcc) ? ~position : position;
          *suftabptr = 0;
#ifdef SAINSHOWSTATE
          gt_assert(bucketptr != NULL);
          printf("L-induce: suftab[%lu]=%ld\n",
                  (unsigned long) (bucketptr-1-suftab),*(bucketptr-1));
#endif
        }
      } else
      {
        *suftabptr = 0;
      }
    } else
    {
      if (position < 0)
      {
        *suftabptr = ~position;
      }
    }
  }
}

static void gt_sain_INTSEQ_induceStypesuffixes1(GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  long *suftabptr;

  gt_sain_special_singleSinduction1(sainseq,
                                    suftab,
                                    (long) (sainseq->totallength-1));
  if (sainseq->seqtype == GT_SAIN_ENCSEQ)
  {
    gt_sain_induceStypes1fromspecialranges(sainseq,
                                           sainseq->seq.encseq,
                                           suftab);
  }
  if (nonspecialentries == 0)
  {
    return;
  }
  for (suftabptr = suftab + nonspecialentries - 1; suftabptr >= suftab;
       suftabptr--)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      if (position >= (long) sainseq->totallength)
      {
        gt_assert(sainseq->roundtable != NULL);
        sainseq->currentround++;
        position -= (long) sainseq->totallength;
      }
      if (position > 0)
      {
        unsigned long currentcc
          = sainseq->seq.array[(unsigned long) position];

        if (currentcc < sainseq->numofchars)
        {
          unsigned long leftcontextcc;

          position--;
          leftcontextcc
            = sainseq->seq.array[(unsigned long) position];
          if (sainseq->roundtable != NULL)
          {
            unsigned long t = (currentcc << 1) |
                              (leftcontextcc > currentcc ? 1UL : 0);

            gt_assert(sainseq->roundtable[t] <= sainseq->currentround);
            if (sainseq->roundtable[t] < sainseq->currentround)
            {
              position += sainseq->totallength;
              sainseq->roundtable[t] = sainseq->currentround;
            }
          }
          GT_SAINUPDATEBUCKETPTR(currentcc);
          gt_assert(bucketptr != NULL && bucketptr - 1 < suftabptr);
          *(--bucketptr) = (leftcontextcc > currentcc)
                            ? ~(position+1) : position;
#ifdef SAINSHOWSTATE
          printf("S-induce: suftab[%lu]=%ld\n",
                  (unsigned long) (bucketptr - suftab),*bucketptr);
#endif
        }
      }
      *suftabptr = 0;
    }
  }
}

static void gt_sain_INTSEQ_induceLtypesuffixes2(const GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  long *suftabptr;

  for (suftabptr = suftab; suftabptr < suftab + nonspecialentries; suftabptr++)
  {
    long position = *suftabptr;

    *suftabptr = ~position;
    if (position > 0)
    {
      unsigned long currentcc;

      position--;
      currentcc = sainseq->seq.array[(unsigned long) position];
      if (currentcc < sainseq->numofchars)
      {
        gt_assert(currentcc > 0);
        GT_SAINUPDATEBUCKETPTR(currentcc);
        gt_assert(bucketptr != NULL && suftabptr < bucketptr);
        *bucketptr++ = (position > 0 &&
                        (sainseq->seq.array[(unsigned long) (position-1)])
                                            < currentcc)
                        ? ~position : position;
#ifdef SAINSHOWSTATE
        gt_assert(bucketptr != NULL);
        printf("L-induce: suftab[%lu]=%ld\n",
               (unsigned long) (bucketptr-1-suftab),*(bucketptr-1));
#endif
      }
    }
  }
}

static void gt_sain_INTSEQ_induceStypesuffixes2(const GtSainseq *sainseq,
                                         long *suftab,
                                         unsigned long nonspecialentries)
{
  unsigned long lastupdatecc = 0, *fillptr = sainseq->bucketfillptr;
  long *bucketptr = NULL;
  long *suftabptr;

  gt_sain_special_singleSinduction2(sainseq,
                                    suftab,
                                    (long) sainseq->totallength,
                                    nonspecialentries);
  if (sainseq->seqtype == GT_SAIN_ENCSEQ)
  {
    gt_sain_induceStypes2fromspecialranges(sainseq,
                                           sainseq->seq.encseq,
                                           suftab,
                                           nonspecialentries);
  }
  if (nonspecialentries == 0)
  {
    return;
  }
  for (suftabptr = suftab + nonspecialentries - 1; suftabptr >= suftab;
       suftabptr--)
  {
    long position = *suftabptr;

    if (position > 0)
    {
      unsigned long currentcc;

      position--;
      currentcc = sainseq->seq.array[(unsigned long) position];
      if (currentcc < sainseq->numofchars)
      {
        GT_SAINUPDATEBUCKETPTR(currentcc);
        gt_assert(bucketptr != NULL && bucketptr - 1 < suftabptr);
        *(--bucketptr) = (position == 0 ||
                          (sainseq->seq.array[(unsigned long) (position-1)])
                                             > currentcc)
                         ? ~position : position;
#ifdef SAINSHOWSTATE
        gt_assert(bucketptr != NULL);
        printf("S-induce: suftab[%lu]=%ld\n",
                (unsigned long) (bucketptr-suftab),*bucketptr);
#endif
      }
    } else
    {
      *suftabptr = ~position;
    }
  }
}

static void gt_sain_INTSEQ_expandorder2original(GtSainseq *sainseq,
                                         unsigned long numberofsuffixes,
                                         unsigned long *suftab)
{
  unsigned long *suftabptr, position,
                writeidx = numberofsuffixes - 1,
                nextcc = GT_UNIQUEINT(sainseq->totallength),
                *sstarsuffixes = suftab + numberofsuffixes;
  unsigned long *sstarfirstcharcount = NULL, *bucketsize = NULL;
  bool nextisStype = true;

  if (sainseq->seqtype == GT_SAIN_INTSEQ)
  {
    unsigned long charidx;

    gt_assert(sainseq->sstarfirstcharcount == NULL);
    sstarfirstcharcount = sainseq->sstarfirstcharcount
                        = sainseq->bucketfillptr;
    bucketsize = sainseq->bucketsize;
    for (charidx = 0; charidx < sainseq->numofchars; charidx++)
    {
      sstarfirstcharcount[charidx] = 0;
      bucketsize[charidx] = 0;
    }
  }
  for (position = sainseq->totallength-1; /* Nothing */; position--)
  {
    unsigned long currentcc = sainseq->seq.array[position];
    bool currentisStype = (currentcc < nextcc ||
                           (currentcc == nextcc && nextisStype)) ? true : false;

    if (!currentisStype && nextisStype)
    {
      if (sstarfirstcharcount != NULL)
      {
        sstarfirstcharcount[nextcc]++;
      }
      sstarsuffixes[writeidx--] = position+1;
    }
    if (bucketsize != NULL)
    {
      bucketsize[currentcc]++;
    }
    nextisStype = currentisStype;
    nextcc = currentcc;
    if (position == 0)
    {
      break;
    }
  }
  for (suftabptr = suftab; suftabptr < suftab + numberofsuffixes; suftabptr++)
  {
    *suftabptr = sstarsuffixes[*suftabptr];
  }
}