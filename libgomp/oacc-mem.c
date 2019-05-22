/* OpenACC Runtime initialization routines

   Copyright (C) 2013-2016 Free Software Foundation, Inc.

   Contributed by Mentor Embedded.

   This file is part of the GNU Offloading and Multi Processing Library
   (libgomp).

   Libgomp is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "openacc.h"
#include "config.h"
#include "libgomp.h"
#include "gomp-constants.h"
#include "oacc-int.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>

/* OpenACC is silent on how memory exhaustion is indicated.  We return
   NULL.  */

void *
acc_malloc (size_t s)
{
  if (!s)
    return NULL;

  goacc_lazy_initialize ();
  return malloc (s);
}

/* OpenACC 2.0a (3.2.16) doesn't specify what to do in the event
   the device address is mapped. We choose to check if it mapped,
   and if it is, to unmap it. */
void
acc_free (void *d)
{
  return free (d);
}

void
acc_memcpy_to_device (void *d, void *h, size_t s)
{
  memmove (d, h, s);
}

void
acc_memcpy_from_device (void *h, void *d, size_t s)
{
  memmove (h, d, s);
}

/* Return the device pointer that corresponds to host data H.  Or NULL
   if no mapping.  */

void *
acc_deviceptr (void *h)
{
  goacc_lazy_initialize ();
  return h;
}

/* Return the host pointer that corresponds to device data D.  Or NULL
   if no mapping.  */

void *
acc_hostptr (void *d)
{
  goacc_lazy_initialize ();
  return d;
}

/* Return 1 if host data [H,+S] is present on the device.  */

int
acc_is_present (void *h, size_t s)
{
  if (!s || !h)
    return 0;

  goacc_lazy_initialize ();
  return h != NULL;
}

/* Create a mapping for host [H,+S] -> device [D,+S] */

void
acc_map_data (void *h, void *d, size_t s)
{
  goacc_lazy_initialize ();

  if (d != h)
    gomp_fatal ("cannot map data on shared-memory system");
}

void
acc_unmap_data (void *h)
{
}

#define FLAG_PRESENT (1 << 0)
#define FLAG_CREATE (1 << 1)
#define FLAG_COPY (1 << 2)

static void *
present_create_copy (unsigned f, void *h, size_t s)
{
  if (!h || !s)
    gomp_fatal ("[%p,+%d] is a bad range", (void *)h, (int)s);

  goacc_lazy_initialize ();
  return h;
}

void *
acc_create (void *h, size_t s)
{
  return present_create_copy (FLAG_CREATE, h, s);
}

void *
acc_copyin (void *h, size_t s)
{
  return present_create_copy (FLAG_CREATE | FLAG_COPY, h, s);
}

void *
acc_present_or_create (void *h, size_t s)
{
  return present_create_copy (FLAG_PRESENT | FLAG_CREATE, h, s);
}

void *
acc_present_or_copyin (void *h, size_t s)
{
  return present_create_copy (FLAG_PRESENT | FLAG_CREATE | FLAG_COPY, h, s);
}

#define FLAG_COPYOUT (1 << 0)

static void
delete_copyout (unsigned f, void *h, size_t s, const char *libfnname)
{
}

void
acc_delete (void *h , size_t s)
{
  delete_copyout (0, h, s, __FUNCTION__);
}

void
acc_copyout (void *h, size_t s)
{
  delete_copyout (FLAG_COPYOUT, h, s, __FUNCTION__);
}

static void
update_dev_host (int is_dev, void *h, size_t s)
{
  goacc_lazy_initialize ();
}

void
acc_update_device (void *h, size_t s)
{
  update_dev_host (1, h, s);
}

void
acc_update_self (void *h, size_t s)
{
  update_dev_host (0, h, s);
}

void
gomp_acc_insert_pointer (size_t mapnum, void **hostaddrs, size_t *sizes,
			 void *kinds)
{
}

void
gomp_acc_remove_pointer (void *h, bool force_copyfrom, int async, int mapnum)
{
}
