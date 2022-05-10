/* File format for coverage information
   Copyright (C) 1996-2018 Free Software Foundation, Inc.
   Contributed by Bob Manson <manson@cygnus.com>.
   Completely remangled by Nathan Sidwell <nathan@codesourcery.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */


/* Coverage information is held in two files.  A notes file, which is
   generated by the compiler, and a data file, which is generated by
   the program under test.  Both files use a similar structure.  We do
   not attempt to make these files backwards compatible with previous
   versions, as you only need coverage information when developing a
   program.  We do hold version information, so that mismatches can be
   detected, and we use a format that allows tools to skip information
   they do not understand or are not interested in.

   Numbers are recorded in the 32 bit unsigned binary form of the
   endianness of the machine generating the file. 64 bit numbers are
   stored as two 32 bit numbers, the low part first.  Strings are
   padded with 1 to 4 NUL bytes, to bring the length up to a multiple
   of 4. The number of 4 bytes is stored, followed by the padded
   string. Zero length and NULL strings are simply stored as a length
   of zero (they have no trailing NUL or padding).

   	int32:  byte3 byte2 byte1 byte0 | byte0 byte1 byte2 byte3
	int64:  int32:low int32:high
	string: int32:0 | int32:length char* char:0 padding
	padding: | char:0 | char:0 char:0 | char:0 char:0 char:0
	item: int32 | int64 | string

   The basic format of the notes file is

	file : int32:magic int32:version int32:stamp int32:support_unexecuted_blocks record*

   The basic format of the data file is

   	file : int32:magic int32:version int32:stamp record*

   The magic ident is different for the notes and the data files.  The
   magic ident is used to determine the endianness of the file, when
   reading.  The version is the same for both files and is derived
   from gcc's version number. The stamp value is used to synchronize
   note and data files and to synchronize merging within a data
   file. It need not be an absolute time stamp, merely a ticker that
   increments fast enough and cycles slow enough to distinguish
   different compile/run/compile cycles.

   Although the ident and version are formally 32 bit numbers, they
   are derived from 4 character ASCII strings.  The version number
   consists of a two character major version number
   (first digit starts from 'A' letter to not to clash with the older
   numbering scheme), the single character minor version number,
   and a single character indicating the status of the release.
   That will be 'e' experimental, 'p' prerelease and 'r' for release.
   Because, by good fortune, these are in alphabetical order, string
   collating can be used to compare version strings.  Be aware that
   the 'e' designation will (naturally) be unstable and might be
   incompatible with itself.  For gcc 17.0 experimental, it would be
   'B70e' (0x42373065).  As we currently do not release more than 5 minor
   releases, the single character should be always fine.  Major number
   is currently changed roughly every year, which gives us space
   for next 250 years (maximum allowed number would be 259.9).

   A record has a tag, length and variable amount of data.

   	record: header data
	header: int32:tag int32:length
	data: item*

   Records are not nested, but there is a record hierarchy.  Tag
   numbers reflect this hierarchy.  Tags are unique across note and
   data files.  Some record types have a varying amount of data.  The
   LENGTH is the number of 4bytes that follow and is usually used to
   determine how much data.  The tag value is split into 4 8-bit
   fields, one for each of four possible levels.  The most significant
   is allocated first.  Unused levels are zero.  Active levels are
   odd-valued, so that the LSB of the level is one.  A sub-level
   incorporates the values of its superlevels.  This formatting allows
   you to determine the tag hierarchy, without understanding the tags
   themselves, and is similar to the standard section numbering used
   in technical documents.  Level values [1..3f] are used for common
   tags, values [41..9f] for the notes file and [a1..ff] for the data
   file.

   The notes file contains the following records
   	note: unit function-graph*
	unit: header int32:checksum string:source
	function-graph: announce_function basic_blocks {arcs | lines}*
	announce_function: header int32:ident
		int32:lineno_checksum int32:cfg_checksum
		string:name string:source int32:start_lineno int32:start_column int32:end_lineno
	basic_block: header int32:flags*
	arcs: header int32:block_no arc*
	arc:  int32:dest_block int32:flags
        lines: header int32:block_no line*
               int32:0 string:NULL
	line:  int32:line_no | int32:0 string:filename

   The BASIC_BLOCK record holds per-bb flags.  The number of blocks
   can be inferred from its data length.  There is one ARCS record per
   basic block.  The number of arcs from a bb is implicit from the
   data length.  It enumerates the destination bb and per-arc flags.
   There is one LINES record per basic block, it enumerates the source
   lines which belong to that basic block.  Source file names are
   introduced by a line number of 0, following lines are from the new
   source file.  The initial source file for the function is NULL, but
   the current source file should be remembered from one LINES record
   to the next.  The end of a block is indicated by an empty filename
   - this does not reset the current source file.  Note there is no
   ordering of the ARCS and LINES records: they may be in any order,
   interleaved in any manner.  The current filename follows the order
   the LINES records are stored in the file, *not* the ordering of the
   blocks they are for.

   The data file contains the following records.
        data: {unit summary:object summary:program* function-data*}*
	unit: header int32:checksum
        function-data:	announce_function present counts
	announce_function: header int32:ident
		int32:lineno_checksum int32:cfg_checksum
	present: header int32:present
	counts: header int64:count*
	summary: int32:checksum {count-summary}GCOV_COUNTERS_SUMMABLE
	count-summary:	int32:num int32:runs int64:sum
			int64:max int64:sum_max histogram
        histogram: {int32:bitvector}8 histogram-buckets*
        histogram-buckets: int32:num int64:min int64:sum

   The ANNOUNCE_FUNCTION record is the same as that in the note file,
   but without the source location.  The COUNTS gives the
   counter values for instrumented features.  The about the whole
   program.  The checksum is used for whole program summaries, and
   disambiguates different programs which include the same
   instrumented object file.  There may be several program summaries,
   each with a unique checksum.  The object summary's checksum is
   zero.  Note that the data file might contain information from
   several runs concatenated, or the data might be merged.

   This file is included by both the compiler, gcov tools and the
   runtime support library libgcov. IN_LIBGCOV and IN_GCOV are used to
   distinguish which case is which.  If IN_LIBGCOV is nonzero,
   libgcov is being built. If IN_GCOV is nonzero, the gcov tools are
   being built. Otherwise the compiler is being built. IN_GCOV may be
   positive or negative. If positive, we are compiling a tool that
   requires additional functions (see the code for knowledge of what
   those functions are).  */

#ifndef GCC_GCOV_IO_H
#define GCC_GCOV_IO_H

#ifndef IN_LIBGCOV
/* About the host */

typedef unsigned gcov_unsigned_t;
typedef unsigned gcov_position_t;
/* gcov_type is typedef'd elsewhere for the compiler */
#if IN_GCOV
#define GCOV_LINKAGE static
typedef int64_t gcov_type;
typedef uint64_t gcov_type_unsigned;
#if IN_GCOV > 0
#include <sys/types.h>
#endif
#endif

#if defined (HOST_HAS_F_SETLKW)
#define GCOV_LOCKED 1
#else
#define GCOV_LOCKED 0
#endif

#define ATTRIBUTE_HIDDEN

#endif /* !IN_LIBGOCV */

#ifndef GCOV_LINKAGE
#define GCOV_LINKAGE extern
#endif

#if IN_LIBGCOV
#define gcov_nonruntime_assert(EXPR) ((void)(0 && (EXPR)))
#else
#define gcov_nonruntime_assert(EXPR) gcc_assert (EXPR)
#define gcov_error(...) fatal_error (input_location, __VA_ARGS__)
#endif

/* File suffixes.  */
#define GCOV_DATA_SUFFIX ".gcda"
#define GCOV_NOTE_SUFFIX ".gcno"

/* File magic. Must not be palindromes.  */
#define GCOV_DATA_MAGIC ((gcov_unsigned_t)0x67636461) /* "gcda" */
#define GCOV_NOTE_MAGIC ((gcov_unsigned_t)0x67636e6f) /* "gcno" */

/* gcov-iov.h is automatically generated by the makefile from
   version.c, it looks like
   	#define GCOV_VERSION ((gcov_unsigned_t)0x89abcdef)
*/
#include "gcov-iov.h"

/* Convert a magic or version number to a 4 character string.  */
#define GCOV_UNSIGNED2STRING(ARRAY,VALUE)	\
  ((ARRAY)[0] = (char)((VALUE) >> 24),		\
   (ARRAY)[1] = (char)((VALUE) >> 16),		\
   (ARRAY)[2] = (char)((VALUE) >> 8),		\
   (ARRAY)[3] = (char)((VALUE) >> 0))

/* The record tags.  Values [1..3f] are for tags which may be in either
   file.  Values [41..9f] for those in the note file and [a1..ff] for
   the data file.  The tag value zero is used as an explicit end of
   file marker -- it is not required to be present.  */

#define GCOV_TAG_FUNCTION	 ((gcov_unsigned_t)0x01000000)
#define GCOV_TAG_FUNCTION_LENGTH (3)
#define GCOV_TAG_BLOCKS		 ((gcov_unsigned_t)0x01410000)
#define GCOV_TAG_BLOCKS_LENGTH(NUM) (NUM)
#define GCOV_TAG_ARCS		 ((gcov_unsigned_t)0x01430000)
#define GCOV_TAG_ARCS_LENGTH(NUM)  (1 + (NUM) * 2)
#define GCOV_TAG_ARCS_NUM(LENGTH)  (((LENGTH) - 1) / 2)
#define GCOV_TAG_LINES		 ((gcov_unsigned_t)0x01450000)
#define GCOV_TAG_COUNTER_BASE 	 ((gcov_unsigned_t)0x01a10000)
#define GCOV_TAG_COUNTER_LENGTH(NUM) ((NUM) * 2)
#define GCOV_TAG_COUNTER_NUM(LENGTH) ((LENGTH) / 2)
#define GCOV_TAG_OBJECT_SUMMARY  ((gcov_unsigned_t)0xa1000000) /* Obsolete */
#define GCOV_TAG_PROGRAM_SUMMARY ((gcov_unsigned_t)0xa3000000)
#define GCOV_TAG_SUMMARY_LENGTH(NUM)  \
        (1 + GCOV_COUNTERS_SUMMABLE * (10 + 3 * 2) + (NUM) * 5)
#define GCOV_TAG_AFDO_FILE_NAMES ((gcov_unsigned_t)0xaa000000)
#define GCOV_TAG_AFDO_FUNCTION ((gcov_unsigned_t)0xac000000)
#define GCOV_TAG_AFDO_WORKING_SET ((gcov_unsigned_t)0xaf000000)


/* Counters that are collected.  */

#define DEF_GCOV_COUNTER(COUNTER, NAME, MERGE_FN) COUNTER,
enum {
#include "gcov-counter.def"
GCOV_COUNTERS
};
#undef DEF_GCOV_COUNTER

/* Counters which can be summaried.  */
#define GCOV_COUNTERS_SUMMABLE	(GCOV_COUNTER_ARCS + 1)

/* The first of counters used for value profiling.  They must form a
   consecutive interval and their order must match the order of
   HIST_TYPEs in value-prof.h.  */
#define GCOV_FIRST_VALUE_COUNTER GCOV_COUNTERS_SUMMABLE

/* The last of counters used for value profiling.  */
#define GCOV_LAST_VALUE_COUNTER (GCOV_COUNTERS - 1)

/* Number of counters used for value profiling.  */
#define GCOV_N_VALUE_COUNTERS \
  (GCOV_LAST_VALUE_COUNTER - GCOV_FIRST_VALUE_COUNTER + 1)

/* The number of hottest callees to be tracked.  */
#define GCOV_ICALL_TOPN_VAL  2

/* The number of counter entries per icall callsite.  */
#define GCOV_ICALL_TOPN_NCOUNTS (1 + GCOV_ICALL_TOPN_VAL * 4)

/* Convert a counter index to a tag.  */
#define GCOV_TAG_FOR_COUNTER(COUNT)				\
	(GCOV_TAG_COUNTER_BASE + ((gcov_unsigned_t)(COUNT) << 17))
/* Convert a tag to a counter.  */
#define GCOV_COUNTER_FOR_TAG(TAG)					\
	((unsigned)(((TAG) - GCOV_TAG_COUNTER_BASE) >> 17))
/* Check whether a tag is a counter tag.  */
#define GCOV_TAG_IS_COUNTER(TAG)				\
	(!((TAG) & 0xFFFF) && GCOV_COUNTER_FOR_TAG (TAG) < GCOV_COUNTERS)

/* The tag level mask has 1's in the position of the inner levels, &
   the lsb of the current level, and zero on the current and outer
   levels.  */
#define GCOV_TAG_MASK(TAG) (((TAG) - 1) ^ (TAG))

/* Return nonzero if SUB is an immediate subtag of TAG.  */
#define GCOV_TAG_IS_SUBTAG(TAG,SUB)				\
	(GCOV_TAG_MASK (TAG) >> 8 == GCOV_TAG_MASK (SUB) 	\
	 && !(((SUB) ^ (TAG)) & ~GCOV_TAG_MASK (TAG)))

/* Return nonzero if SUB is at a sublevel to TAG.  */
#define GCOV_TAG_IS_SUBLEVEL(TAG,SUB)				\
     	(GCOV_TAG_MASK (TAG) > GCOV_TAG_MASK (SUB))

/* Basic block flags.  */
#define GCOV_BLOCK_UNEXPECTED	(1 << 1)

/* Arc flags.  */
#define GCOV_ARC_ON_TREE 	(1 << 0)
#define GCOV_ARC_FAKE		(1 << 1)
#define GCOV_ARC_FALLTHROUGH	(1 << 2)

/* Structured records.  */

/* Structure used for each bucket of the log2 histogram of counter values.  */
typedef struct
{
  /* Number of counters whose profile count falls within the bucket.  */
  gcov_unsigned_t num_counters;
  /* Smallest profile count included in this bucket.  */
  gcov_type min_value;
  /* Cumulative value of the profile counts in this bucket.  */
  gcov_type cum_value;
} gcov_bucket_type;

/* For a log2 scale histogram with each range split into 4
   linear sub-ranges, there will be at most 64 (max gcov_type bit size) - 1 log2
   ranges since the lowest 2 log2 values share the lowest 4 linear
   sub-range (values 0 - 3).  This is 252 total entries (63*4).  */

#define GCOV_HISTOGRAM_SIZE 252

/* How many unsigned ints are required to hold a bit vector of non-zero
   histogram entries when the histogram is written to the gcov file.
   This is essentially a ceiling divide by 32 bits.  */
#define GCOV_HISTOGRAM_BITVECTOR_SIZE (GCOV_HISTOGRAM_SIZE + 31) / 32

/* Cumulative counter data.  */
struct gcov_ctr_summary
{
  gcov_unsigned_t num;		/* number of counters.  */
  gcov_unsigned_t runs;		/* number of program runs */
  gcov_type sum_all;		/* sum of all counters accumulated.  */
  gcov_type run_max;		/* maximum value on a single run.  */
  gcov_type sum_max;    	/* sum of individual run max values.  */
  gcov_bucket_type histogram[GCOV_HISTOGRAM_SIZE]; /* histogram of
                                                      counter values.  */
};

/* Object & program summary record.  */
struct gcov_summary
{
  gcov_unsigned_t checksum;	/* checksum of program */
  struct gcov_ctr_summary ctrs[GCOV_COUNTERS_SUMMABLE];
};

#if !defined(inhibit_libc)

/* Functions for reading and writing gcov files. In libgcov you can
   open the file for reading then writing. Elsewhere you can open the
   file either for reading or for writing. When reading a file you may
   use the gcov_read_* functions, gcov_sync, gcov_position, &
   gcov_error. When writing a file you may use the gcov_write
   functions, gcov_seek & gcov_error. When a file is to be rewritten
   you use the functions for reading, then gcov_rewrite then the
   functions for writing.  Your file may become corrupted if you break
   these invariants.  */

#if !IN_LIBGCOV
GCOV_LINKAGE int gcov_open (const char */*name*/, int /*direction*/);
#endif

#if !IN_LIBGCOV || defined (IN_GCOV_TOOL)
GCOV_LINKAGE int gcov_magic (gcov_unsigned_t, gcov_unsigned_t);
#endif

/* Available everywhere.  */
GCOV_LINKAGE int gcov_close (void) ATTRIBUTE_HIDDEN;
GCOV_LINKAGE gcov_unsigned_t gcov_read_unsigned (void) ATTRIBUTE_HIDDEN;
GCOV_LINKAGE gcov_type gcov_read_counter (void) ATTRIBUTE_HIDDEN;
GCOV_LINKAGE void gcov_read_summary (struct gcov_summary *) ATTRIBUTE_HIDDEN;
GCOV_LINKAGE const char *gcov_read_string (void);
GCOV_LINKAGE void gcov_sync (gcov_position_t /*base*/,
			     gcov_unsigned_t /*length */);
char *mangle_path (char const *base);

#if !IN_GCOV
/* Available outside gcov */
GCOV_LINKAGE void gcov_write_unsigned (gcov_unsigned_t) ATTRIBUTE_HIDDEN;
#endif

#if !IN_GCOV && !IN_LIBGCOV
/* Available only in compiler */
GCOV_LINKAGE unsigned gcov_histo_index (gcov_type value);
GCOV_LINKAGE void gcov_write_string (const char *);
GCOV_LINKAGE void gcov_write_filename (const char *);
GCOV_LINKAGE gcov_position_t gcov_write_tag (gcov_unsigned_t);
GCOV_LINKAGE void gcov_write_length (gcov_position_t /*position*/);
#endif

#if IN_GCOV <= 0 && !IN_LIBGCOV
/* Available in gcov-dump and the compiler.  */

/* Number of data points in the working set summary array. Using 128
   provides information for at least every 1% increment of the total
   profile size. The last entry is hardwired to 99.9% of the total.  */
#define NUM_GCOV_WORKING_SETS 128

/* Working set size statistics for a given percentage of the entire
   profile (sum_all from the counter summary).  */
typedef struct gcov_working_set_info
{
  /* Number of hot counters included in this working set.  */
  unsigned num_counters;
  /* Smallest counter included in this working set.  */
  gcov_type min_counter;
} gcov_working_set_t;

GCOV_LINKAGE void compute_working_sets (const struct gcov_ctr_summary *summary,
                                        gcov_working_set_t *gcov_working_sets);
#endif

#if IN_GCOV > 0
/* Available in gcov */
GCOV_LINKAGE time_t gcov_time (void);
#endif

#endif /* !inhibit_libc  */

#endif /* GCC_GCOV_IO_H */
