/* Copyright (C) 2013-2016 Free Software Foundation, Inc.

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

/* This file handles OpenACC constructs.  */

#include "openacc.h"
#include "libgomp.h"
#include "libgomp_g.h"
#include "gomp-constants.h"
#include "oacc-int.h"
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>  /* For PRIu64.  */
#endif
#include <string.h>
#include <stdarg.h>
#include <assert.h>

static void goacc_wait (int async, int num_waits, va_list *ap);


/* Launch a possibly offloaded function on DEVICE.  FN is the host fn
   address.  MAPNUM, HOSTADDRS, SIZES & KINDS  describe the memory
   blocks to be copied to/from the device.  Varadic arguments are
   keyed optional parameters terminated with a zero.  */

void
GOACC_parallel_keyed (int device, void (*fn) (void *),
		      size_t mapnum, void **hostaddrs, size_t *sizes,
		      unsigned short *kinds, ...)
{
  bool host_fallback = device == GOMP_DEVICE_HOST_FALLBACK;
  struct goacc_thread *thr;
  struct gomp_device_descr *acc_dev;

#ifdef HAVE_INTTYPES_H
  gomp_debug (0, "%s: mapnum=%"PRIu64", hostaddrs=%p, size=%p, kinds=%p\n",
	      __FUNCTION__, (uint64_t) mapnum, hostaddrs, sizes, kinds);
#else
  gomp_debug (0, "%s: mapnum=%lu, hostaddrs=%p, sizes=%p, kinds=%p\n",
	      __FUNCTION__, (unsigned long) mapnum, hostaddrs, sizes, kinds);
#endif
  goacc_lazy_initialize ();

  thr = goacc_thread ();
  acc_dev = thr->dev;

  /* Host fallback if "if" clause is false or if the current device is set to
     the host.  */
  if (host_fallback)
    {
      goacc_save_and_set_bind (acc_device_host);
      fn (hostaddrs);
      goacc_restore_bind ();
      return;
    }
  else if (acc_device_type (acc_dev->type) == acc_device_host)
    {
      fn (hostaddrs);
      return;
    }

  /* acc_device_host is the only supported device type.  */
}

/* Legacy entry point, only provide host execution.  */

void
GOACC_parallel (int device, void (*fn) (void *),
		size_t mapnum, void **hostaddrs, size_t *sizes,
		unsigned short *kinds,
		int num_gangs, int num_workers, int vector_length,
		int async, int num_waits, ...)
{
  goacc_save_and_set_bind (acc_device_host);
  fn (hostaddrs);
  goacc_restore_bind ();
}

void
GOACC_data_start (int device, size_t mapnum,
		  void **hostaddrs, size_t *sizes, unsigned short *kinds)
{
  goacc_lazy_initialize ();
}

void
GOACC_data_end (void)
{
  gomp_debug (0, "  %s: restore mappings\n", __FUNCTION__);
  gomp_debug (0, "  %s: mappings restored\n", __FUNCTION__);
}

void
GOACC_enter_exit_data (int device, size_t mapnum,
		       void **hostaddrs, size_t *sizes, unsigned short *kinds,
		       int async, int num_waits, ...)
{
  goacc_lazy_initialize ();
}

static void
goacc_wait (int async, int num_waits, va_list *ap)
{
  struct goacc_thread *thr = goacc_thread ();
  struct gomp_device_descr *acc_dev = thr->dev;

  while (num_waits--)
    {
      int qid = va_arg (*ap, int);
      
      if (acc_async_test (qid))
	continue;

      if (async == acc_async_sync)
	acc_wait (qid);
      else if (qid == async)
	;/* If we're waiting on the same asynchronous queue as we're
	    launching on, the queue itself will order work as
	    required, so there's no need to wait explicitly.  */
      else
	acc_dev->openacc.async_wait_async_func (qid, async);
    }
}

void
GOACC_update (int device, size_t mapnum,
	      void **hostaddrs, size_t *sizes, unsigned short *kinds,
	      int async, int num_waits, ...)
{
  goacc_lazy_initialize ();
}

void
GOACC_wait (int async, int num_waits, ...)
{
  if (num_waits)
    {
      va_list ap;

      va_start (ap, num_waits);
      goacc_wait (async, num_waits, &ap);
      va_end (ap);
    }
  else if (async == acc_async_sync)
    acc_wait_all ();
  else if (async == acc_async_noval)
    goacc_thread ()->dev->openacc.async_wait_all_async_func (acc_async_noval);
}

int
GOACC_get_num_threads (void)
{
  return 1;
}

int
GOACC_get_thread_num (void)
{
  return 0;
}

void
GOACC_declare (int device, size_t mapnum,
	       void **hostaddrs, size_t *sizes, unsigned short *kinds)
{
  int i;

  for (i = 0; i < mapnum; i++)
    {
      unsigned char kind = kinds[i] & 0xff;

      if (kind == GOMP_MAP_POINTER || kind == GOMP_MAP_TO_PSET)
	continue;

      switch (kind)
	{
	  case GOMP_MAP_FORCE_ALLOC:
	  case GOMP_MAP_FORCE_FROM:
	  case GOMP_MAP_FORCE_TO:
	  case GOMP_MAP_POINTER:
	  case GOMP_MAP_DELETE:
	    GOACC_enter_exit_data (device, 1, &hostaddrs[i], &sizes[i],
				   &kinds[i], 0, 0);
	    break;

	  case GOMP_MAP_FORCE_DEVICEPTR:
	    break;

	  case GOMP_MAP_ALLOC:
	    if (!acc_is_present (hostaddrs[i], sizes[i]))
	      GOACC_enter_exit_data (device, 1, &hostaddrs[i], &sizes[i],
				     &kinds[i], 0, 0);
	    break;

	  case GOMP_MAP_TO:
	    GOACC_enter_exit_data (device, 1, &hostaddrs[i], &sizes[i],
				   &kinds[i], 0, 0);

	    break;

	  case GOMP_MAP_FROM:
	    kinds[i] = GOMP_MAP_FORCE_FROM;
	    GOACC_enter_exit_data (device, 1, &hostaddrs[i], &sizes[i],
				   &kinds[i], 0, 0);
	    break;

	  case GOMP_MAP_FORCE_PRESENT:
	    if (!acc_is_present (hostaddrs[i], sizes[i]))
	      gomp_fatal ("[%p,%ld] is not mapped", hostaddrs[i],
			  (unsigned long) sizes[i]);
	    break;

	  default:
	    assert (0);
	    break;
	}
    }
}
