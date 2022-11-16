/* Implement realloc with the help of the malloc and free.

   The CUDA runtime provides malloc and free, but not realloc.  The
   compiler renames the user's malloc & free so we don't directly call
   the CUDA implementations.  We provide wrappers for those functions
   that store a size cookie that we can use here.  Inspection of the
   CUDA malloced area didn't show an obvious cookie we could use.  The
   cookie is of type  size_t, but placed at a long long boundary to
   maintain alignment of the value returned  to the user.

   Copyright (C) 2014-2016 Free Software Foundation, Inc.

   This file is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3, or (at your option) any
   later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdlib.h>

void *
realloc (void *old_ptr, size_t new_size)
{
  void *new_ptr = malloc (new_size);

  if (old_ptr && new_ptr)
    {
      size_t old_size = *(size_t *)((long long *)old_ptr - 1);
      size_t copy_size = old_size > new_size ? new_size : old_size;
      __builtin_memcpy (new_ptr, old_ptr, copy_size);
      free (old_ptr);
    }

  return new_ptr;
}
