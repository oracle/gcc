/*
  compress.h - header for compression
  Copyright (C) 1999  Bryan Burns
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

#ifndef __FASTJAR_COMPRESS_H__
#define __FASTJAR_COMPRESS_H__

#include <zlib.h>

#include "jartool.h"
#include "pushback.h"

/* Initializes the compression data structure(s) */
void init_compression(void);

/* Compresses the file specified by in_fd and appends it to out_fd */
int compress_file(int, int, struct zipentry *, struct zipentry *);

/* Frees memory used by compression function */
void end_compression(void);

void init_inflation(void);
int inflate_file(pb_file *, int, struct zipentry *);
void end_inflation(void);
Bytef *inflate_string(pb_file *, ub4 *, ub4 *);

#endif /* __FASTJAR_COMPRESS_H__ */
