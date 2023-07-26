/*
 * Support file for nvptx in newlib.
 * Copyright (c) 2014 Mentor Graphics.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <stdlib.h>
#include <string.h>

void *_realloc_os_r (struct _reent *r, void *p, size_t n, size_t oldn)
{
  void *newp = p;
  if (oldn < n)
    {
      newp = malloc (n);
      if (p != 0)
	memcpy (newp, p, oldn);
    }
  return newp;
}

