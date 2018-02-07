/* shift.c -- utilities to move regions of data in a file.
   Copyright (C) 2004, 2005  Free Software Foundation, Inc.
   Copyright (C) 2007 Dalibor Topic

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.
  
This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.
  
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.  */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include "jartool.h"
#include "shift.h"

#define BUFFER_SIZE 1024

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* The offset of the end of the last zip entry. */
ub4 end_of_entries;

/*
 * Shift the contents of a file up by `amount' bytes, starting at `begin'.
 * The file is not truncated, data from `amount' to `begin - amount' is
 * overwritten. The current file pointer of `fd' is preserved. Note that
 * this might be past the new "end" of the file.
 *
 * If this function is passed a `struct zipentry', then all `offset'
 * fields from that entry down the list that are greater than or equal
 * to `begin' will be decreased by `amount'.
 * 
 * fd     - The file descriptor.
 * begin  - The offset of the first byte that should be shifted.
 * amount - The number of bytes to shift by.
 * ze     - A pointer into a list of zip entries that should be updated
 *          to reflect the modified offset.
 */
int
shift_up (int fd, off_t begin, off_t amount, struct zipentry *ze)
{
  int  moved = 0;
  ub1 buffer[BUFFER_SIZE];
  off_t where, end, save;
  size_t len;

  if (amount <= 0)
    return 0;

  if ((save = lseek (fd, 0, SEEK_CUR)) == -1)
    return 1;
  if ((end = lseek (fd, 0, SEEK_END)) == -1)
    return 1;
  if (end < begin)
    return 0;

  where = begin;

  do
    {
      ssize_t num_read;

      if (lseek (fd, where, SEEK_SET) < 0)
	return 1;
      if ((num_read = read (fd, buffer, BUFFER_SIZE)) < 0)
	return 1;
      if (num_read == 0)
	break;
      if (lseek (fd, where - amount, SEEK_SET) < 0)
	return 1;

      len = (size_t) num_read;

      if (write (fd, buffer, len) < 0)
	return 1;
      where += len;
    }
  while (where < end);

  for (; ze; ze = ze->next_entry)
    {
      if ((off_t) ze->offset >= begin)
	{
	  ze->offset -= amount;
	  moved = 1;
	}
    }
  if (moved)
    end_of_entries -= amount;

  if (lseek (fd, save, SEEK_SET) == -1)
    return 1;
  return 0;
}

/*
 * Shift the contents of this file down by `amount' bytes, extending the
 * end of file, starting at `begin'. This function will preserve the
 * current file pointer of `fd'. Naturally, this function will fail if
 * `fd' is not seekable.
 *
 * If this function is passed a `struct zipentry', then all `offset'
 * fields from that entry down the list that are greater than or equal
 * to `begin' will be increased by `amount'.
 *
 * fd     - The file descriptor.
 * begin  - The offset of the first byte that should be shifted.
 * amount - The number of bytes to shift by.
 * ze     - A pointer into a list of zip entries that should be updated
 *          to reflect the modified offset.
 */
int
shift_down (int fd, off_t begin, off_t amount, struct zipentry *ze)
{
  int moved = 0;
  ub1 buffer[BUFFER_SIZE];
  off_t where, save, off;
  size_t len;

  if (amount <= 0)
    return 0;

  if ((save = lseek (fd, 0, SEEK_CUR)) == -1)
    return 1;
  if ((where = lseek (fd, 0, SEEK_END)) == -1)
    return 1;
  if (where < begin)
    return 0;
  off = (where - begin) % BUFFER_SIZE;
  if (off == 0)
    where -= BUFFER_SIZE;
  else
    where -= off;

  do
    {
      ssize_t num_read;

      if (lseek (fd, where, SEEK_SET) < 0)
	return 1;
      if ((num_read = read (fd, buffer, BUFFER_SIZE)) < 0)
	return 1;
      if (lseek (fd, where + amount, SEEK_SET) < 0)
	return 1;

      len = (size_t) num_read;

      if (write (fd, buffer, len) < 0)
	return 1;
      where -= BUFFER_SIZE;
    }
  while (where >= begin);

  for (; ze; ze = ze->next_entry)
    {
      if ((off_t) ze->offset >= begin)
	{
	  ze->offset += amount;
	  moved = 1;
	}
    }
  if (moved)
    end_of_entries += amount;

  if (lseek (fd, save, SEEK_SET) == -1)
    return 1;

  return 0;
}
