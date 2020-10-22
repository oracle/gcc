/*
  jartool.h - generic defines, struct defs etc.
  Copyright (C) 1999, 2005  Bryan Burns
  Copyright (C) 2007  Dalibor Topic

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __FASTJAR_JARTOOL_H__
#define __FASTJAR_JARTOOL_H__

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#else
# ifdef HAVE_STDINT_H
#  include  <stdint.h>
# endif
#endif

#define ACTION_NONE 0
#define ACTION_CREATE 1
#define ACTION_EXTRACT 2
#define ACTION_UPDATE 3
#define ACTION_LIST 4
#define ACTION_INDEX 5

#define TRUE 1
#define FALSE 0

/* Amount of bytes to read at a time.  You can change this to optimize for
   your system */
#define RDSZ 4096

/* Change these to match your system:
   ub1 == unsigned 1 byte word
   ub2 == unsigned 2 byte word
   ub4 == unsigned 4 byte word
*/
typedef uint8_t ub1;
typedef uint16_t ub2;
typedef uint32_t ub4;

struct zipentry {
  ub2 mod_time;
  ub2 mod_date;
  ub4 crc;
  ub4 csize;
  ub4 usize;
  ub4 offset;
  ub1 compressed;
  ub2 flags;
  char *filename;
  
  struct zipentry *next_entry;
};

typedef struct zipentry zipentry;

#endif /* __FASTJAR_JARTOOL_H__ */
