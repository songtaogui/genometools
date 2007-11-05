#!/bin/sh
#
# Copyright (c) 2007 Gordon Gremme <gremme@zbh.uni-hamburg.de>
# Copyright (c) 2007 Center for Bioinformatics, University of Hamburg
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

localpath=src/libgtmatch

if test "XX${GT_ENV_OPTIONS}" != "XX"
then
  unset GT_ENV_OPTIONS
fi

TMPFILE=`mktemp /tmp/skproto-all.XXXXXX` || exit 1
cat << END_OF_TEXT > ${TMPFILE}
alphabet.c
encodedseq.c
overallseq.c
sfx-suffixer.c
sfx-partssuf.c
enum-patt.c
esa-mmsearch.c
esa-seqread.c
mapspec-gen.c
measure-time.c
sfx-readint.c
test-pairwise.c
verbose.c
END_OF_TEXT

for filename in `ls ${localpath}/*.c | grep -v -f ${TMPFILE}`
do
  prfile="${localpath}/`basename ${filename} .c`.pr"
  if test ! -f ${prfile} ||
     test ! -s ${prfile} ||
     test ${filename} -nt ${prfile}
  then
    echo "create ${prfile}"
    bin/skproto ${filename} > ${prfile}
  fi
done

rm -f ${TMPFILE}
