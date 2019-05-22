/* Copyright (C) 2013 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

   This file is part of the GNU OpenMP Library (libgomp).

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

/* This file contains the support of offloading.  */

#include "config.h"
#include "libgomp.h"
#include "oacc-plugin.h"
#include "oacc-int.h"
#include "gomp-constants.h"
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>  /* For PRIu64.  */
#endif
#include <string.h>
#include <assert.h>
#include <errno.h>

attribute_hidden int
gomp_get_num_devices (void)
{
  return 0;
}

/* This function should be called from every offload image while loading.
   It gets the descriptor of the host func and var tables HOST_TABLE, TYPE of
   the target, and TARGET_DATA needed by target plugin.  */

void
GOMP_offload_register_ver (unsigned version, const void *host_table,
			   int target_type, const void *target_data)
{
  (void) version;
  (void) host_table;
  (void) target_type;
  (void) target_data;
}

void
GOMP_offload_register (const void *host_table, int target_type,
		       const void *target_data)
{
  (void) host_table;
  (void) target_type;
  (void) target_data;
}

/* This function should be called from every offload image while unloading.
   It gets the descriptor of the host func and var tables HOST_TABLE, TYPE of
   the target, and TARGET_DATA needed by target plugin.  */

void
GOMP_offload_unregister_ver (unsigned version, const void *host_table,
			     int target_type, const void *target_data)
{
  (void) version;
  (void) host_table;
  (void) target_type;
  (void) target_data;
}

void
GOMP_offload_unregister (const void *host_table, int target_type,
			 const void *target_data)
{
  (void) host_table;
  (void) target_type;
  (void) target_data;
}

/* This function initializes the target device, specified by DEVICEP.  DEVICEP
   must be locked on entry, and remains locked on return.  */

attribute_hidden void
gomp_init_device (struct gomp_device_descr *devicep)
{
  devicep->state = GOMP_DEVICE_INITIALIZED;
}

attribute_hidden void
gomp_unload_device (struct gomp_device_descr *devicep)
{
}

/* Free address mapping tables.  MM must be locked on entry, and remains locked
   on return.  */

attribute_hidden void
gomp_free_memmap (struct splay_tree_s *mem_map)
{
  while (mem_map->root)
    {
      struct target_mem_desc *tgt = mem_map->root->key.tgt;

      splay_tree_remove (mem_map, &mem_map->root->key);
      free (tgt->array);
      free (tgt);
    }
}

/* Host fallback for GOMP_target{,_ext} routines.  */

static void
gomp_target_fallback (void (*fn) (void *), void **hostaddrs)
{
  struct gomp_thread old_thr, *thr = gomp_thread ();
  old_thr = *thr;
  memset (thr, '\0', sizeof (*thr));
  if (gomp_places_list)
    {
      thr->place = old_thr.place;
      thr->ts.place_partition_len = gomp_places_list_len;
    }
  fn (hostaddrs);
  gomp_free_thread (thr);
  *thr = old_thr;
}

/* Calculate alignment and size requirements of a private copy of data shared
   as GOMP_MAP_FIRSTPRIVATE and store them to TGT_ALIGN and TGT_SIZE.  */

static inline void
calculate_firstprivate_requirements (size_t mapnum, size_t *sizes,
				     unsigned short *kinds, size_t *tgt_align,
				     size_t *tgt_size)
{
  size_t i;
  for (i = 0; i < mapnum; i++)
    if ((kinds[i] & 0xff) == GOMP_MAP_FIRSTPRIVATE)
      {
	size_t align = (size_t) 1 << (kinds[i] >> 8);
	if (*tgt_align < align)
	  *tgt_align = align;
	*tgt_size = (*tgt_size + align - 1) & ~(align - 1);
	*tgt_size += sizes[i];
      }
}

/* Copy data shared as GOMP_MAP_FIRSTPRIVATE to DST.  */

static inline void
copy_firstprivate_data (char *tgt, size_t mapnum, void **hostaddrs,
			size_t *sizes, unsigned short *kinds, size_t tgt_align,
			size_t tgt_size)
{
  uintptr_t al = (uintptr_t) tgt & (tgt_align - 1);
  if (al)
    tgt += tgt_align - al;
  tgt_size = 0;
  size_t i;
  for (i = 0; i < mapnum; i++)
    if ((kinds[i] & 0xff) == GOMP_MAP_FIRSTPRIVATE)
      {
	size_t align = (size_t) 1 << (kinds[i] >> 8);
	tgt_size = (tgt_size + align - 1) & ~(align - 1);
	memcpy (tgt + tgt_size, hostaddrs[i], sizes[i]);
	hostaddrs[i] = tgt + tgt_size;
	tgt_size = tgt_size + sizes[i];
      }
}

/* Called when encountering a target directive.  If DEVICE
   is GOMP_DEVICE_ICV, it means use device-var ICV.  If it is
   GOMP_DEVICE_HOST_FALLBACK (or any value
   larger than last available hw device), use host fallback.
   FN is address of host code, UNUSED is part of the current ABI, but
   we're not actually using it.  HOSTADDRS, SIZES and KINDS are arrays
   with MAPNUM entries, with addresses of the host objects,
   sizes of the host objects (resp. for pointer kind pointer bias
   and assumed sizeof (void *) size) and kinds.  */

void
GOMP_target (int device, void (*fn) (void *), const void *unused,
	     size_t mapnum, void **hostaddrs, size_t *sizes,
	     unsigned char *kinds)
{
  return gomp_target_fallback (fn, hostaddrs);
}

/* Like GOMP_target, but KINDS is 16-bit, UNUSED is no longer present,
   and several arguments have been added:
   FLAGS is a bitmask, see GOMP_TARGET_FLAG_* in gomp-constants.h.
   DEPEND is array of dependencies, see GOMP_task for details.

   ARGS is a pointer to an array consisting of a variable number of both
   device-independent and device-specific arguments, which can take one two
   elements where the first specifies for which device it is intended, the type
   and optionally also the value.  If the value is not present in the first
   one, the whole second element the actual value.  The last element of the
   array is a single NULL.  Among the device independent can be for example
   NUM_TEAMS and THREAD_LIMIT.

   NUM_TEAMS is positive if GOMP_teams will be called in the body with
   that value, or 1 if teams construct is not present, or 0, if
   teams construct does not have num_teams clause and so the choice is
   implementation defined, and -1 if it can't be determined on the host
   what value will GOMP_teams have on the device.
   THREAD_LIMIT similarly is positive if GOMP_teams will be called in the
   body with that value, or 0, if teams construct does not have thread_limit
   clause or the teams construct is not present, or -1 if it can't be
   determined on the host what value will GOMP_teams have on the device.  */

void
GOMP_target_ext (int device, void (*fn) (void *), size_t mapnum,
		 void **hostaddrs, size_t *sizes, unsigned short *kinds,
		 unsigned int flags, void **depend, void **args)
{
  size_t tgt_align = 0, tgt_size = 0;
  bool fpc_done = false;

  if (flags & GOMP_TARGET_FLAG_NOWAIT)
    {
      struct gomp_thread *thr = gomp_thread ();
      if (thr->ts.team
	  && !thr->task->final_task)
	{
	  gomp_create_target_task (NULL, fn, mapnum, hostaddrs,
				   sizes, kinds, flags, depend, args,
				   GOMP_TARGET_TASK_BEFORE_MAP);
	  return;
	}
    }

  /* If there are depend clauses, but nowait is not present
     (or we are in a final task), block the parent task until the
     dependencies are resolved and then just continue with the rest
     of the function as if it is a merged task.  */
  if (depend != NULL)
    {
      struct gomp_thread *thr = gomp_thread ();
      if (thr->task && thr->task->depend_hash)
	{
	  /* If we might need to wait, copy firstprivate now.  */
	  calculate_firstprivate_requirements (mapnum, sizes, kinds,
					       &tgt_align, &tgt_size);
	  if (tgt_align)
	    {
	      char *tgt = gomp_alloca (tgt_size + tgt_align - 1);
	      copy_firstprivate_data (tgt, mapnum, hostaddrs, sizes, kinds,
				      tgt_align, tgt_size);
	    }
	  fpc_done = true;
	  gomp_task_maybe_wait_for_dependencies (depend);
	}
    }

  if (!fpc_done)
    {
      calculate_firstprivate_requirements (mapnum, sizes, kinds,
					   &tgt_align, &tgt_size);
      if (tgt_align)
	{
	  char *tgt = gomp_alloca (tgt_size + tgt_align - 1);
	  copy_firstprivate_data (tgt, mapnum, hostaddrs, sizes, kinds,
				  tgt_align, tgt_size);
	}
    }
  gomp_target_fallback (fn, hostaddrs);
}

/* Host fallback for GOMP_target_data{,_ext} routines.  */

static void
gomp_target_data_fallback (void)
{
}

void
GOMP_target_data (int device, const void *unused, size_t mapnum,
		  void **hostaddrs, size_t *sizes, unsigned char *kinds)
{
  return gomp_target_data_fallback ();
}

void
GOMP_target_data_ext (int device, size_t mapnum, void **hostaddrs,
		      size_t *sizes, unsigned short *kinds)
{
  return gomp_target_data_fallback ();
}

void
GOMP_target_end_data (void)
{
}

void
GOMP_target_update (int device, const void *unused, size_t mapnum,
		    void **hostaddrs, size_t *sizes, unsigned char *kinds)
{
}

void
GOMP_target_update_ext (int device, size_t mapnum, void **hostaddrs,
			size_t *sizes, unsigned short *kinds,
			unsigned int flags, void **depend)
{
  /* If there are depend clauses, but nowait is not present,
     block the parent task until the dependencies are resolved
     and then just continue with the rest of the function as if it
     is a merged task.  Until we are able to schedule task during
     variable mapping or unmapping, ignore nowait if depend clauses
     are not present.  */
  if (depend != NULL)
    {
      struct gomp_thread *thr = gomp_thread ();
      if (thr->task && thr->task->depend_hash)
	{
	  if ((flags & GOMP_TARGET_FLAG_NOWAIT)
	      && thr->ts.team
	      && !thr->task->final_task)
	    {
	      if (gomp_create_target_task (NULL, (void (*) (void *)) NULL,
					   mapnum, hostaddrs, sizes, kinds,
					   flags | GOMP_TARGET_FLAG_UPDATE,
					   depend, NULL, GOMP_TARGET_TASK_DATA))
		return;
	    }
	  else
	    {
	      struct gomp_team *team = thr->ts.team;
	      /* If parallel or taskgroup has been cancelled, don't start new
		 tasks.  */
	      if (team
		  && (gomp_team_barrier_cancelled (&team->barrier)
		      || (thr->task->taskgroup
			  && thr->task->taskgroup->cancelled)))
		return;

	      gomp_task_maybe_wait_for_dependencies (depend);
	    }
	}
    }
}

void
GOMP_target_enter_exit_data (int device, size_t mapnum, void **hostaddrs,
			     size_t *sizes, unsigned short *kinds,
			     unsigned int flags, void **depend)
{
  /* If there are depend clauses, but nowait is not present,
     block the parent task until the dependencies are resolved
     and then just continue with the rest of the function as if it
     is a merged task.  Until we are able to schedule task during
     variable mapping or unmapping, ignore nowait if depend clauses
     are not present.  */
  if (depend != NULL)
    {
      struct gomp_thread *thr = gomp_thread ();
      if (thr->task && thr->task->depend_hash)
	{
	  if ((flags & GOMP_TARGET_FLAG_NOWAIT)
	      && thr->ts.team
	      && !thr->task->final_task)
	    {
	      if (gomp_create_target_task (NULL, (void (*) (void *)) NULL,
					   mapnum, hostaddrs, sizes, kinds,
					   flags, depend, NULL,
					   GOMP_TARGET_TASK_DATA))
		return;
	    }
	  else
	    {
	      struct gomp_team *team = thr->ts.team;
	      /* If parallel or taskgroup has been cancelled, don't start new
		 tasks.  */
	      if (team
		  && (gomp_team_barrier_cancelled (&team->barrier)
		      || (thr->task->taskgroup
			  && thr->task->taskgroup->cancelled)))
		return;

	      gomp_task_maybe_wait_for_dependencies (depend);
	    }
	}
    }
}

bool
gomp_target_task_fn (void *data)
{
  struct gomp_target_task *ttask = (struct gomp_target_task *) data;

  if (ttask->fn != NULL)
    {
      ttask->state = GOMP_TARGET_TASK_FALLBACK;
      gomp_target_fallback (ttask->fn, ttask->hostaddrs);
      return false;
    }
  return false;
}

void
GOMP_teams (unsigned int num_teams, unsigned int thread_limit)
{
  if (thread_limit)
    {
      struct gomp_task_icv *icv = gomp_icv (true);
      icv->thread_limit_var
	= thread_limit > INT_MAX ? UINT_MAX : thread_limit;
    }
  (void) num_teams;
}

void *
omp_target_alloc (size_t size, int device_num)
{
  if (device_num == GOMP_DEVICE_HOST_FALLBACK)
    return malloc (size);

  return NULL;
}

void
omp_target_free (void *device_ptr, int device_num)
{
  if (device_ptr == NULL)
    return;

  if (device_num == GOMP_DEVICE_HOST_FALLBACK)
    {
      free (device_ptr);
      return;
    }
}

int
omp_target_is_present (void *ptr, int device_num)
{
  if (ptr == NULL)
    return 1;

  if (device_num == GOMP_DEVICE_HOST_FALLBACK)
    return 1;

  return 0;
}

int
omp_target_memcpy (void *dst, void *src, size_t length, size_t dst_offset,
		   size_t src_offset, int dst_device_num, int src_device_num)
{
  if (dst_device_num != GOMP_DEVICE_HOST_FALLBACK)
    return EINVAL;
  if (src_device_num != GOMP_DEVICE_HOST_FALLBACK)
    return EINVAL;
  memcpy ((char *) dst + dst_offset, (char *) src + src_offset, length);
  return 0;
}

#define HALF_SIZE_T (((size_t) 1) << (8 * sizeof (size_t) / 2))

#define __builtin_mul_overflow(x, y, z) \
  ({ bool retval = false;					\
     size_t xval = (x);						\
     size_t yval = (y);						\
     size_t zval = xval * yval;					\
     if (__builtin_expect ((xval | yval) >= HALF_SIZE_T, 0))	\
       {							\
         if (xval && zval / xval != yval)			\
	   retval = true;					\
       }							\
     *(z) = zval;						\
     retval; })

static int
omp_target_memcpy_rect_worker (void *dst, void *src, size_t element_size,
			       int num_dims, const size_t *volume,
			       const size_t *dst_offsets,
			       const size_t *src_offsets,
			       const size_t *dst_dimensions,
			       const size_t *src_dimensions)
{
  size_t dst_slice = element_size;
  size_t src_slice = element_size;
  size_t j, dst_off, src_off, length;
  int i, ret;


  if (num_dims == 1)
    {
      if (__builtin_mul_overflow (element_size, volume[0], &length)
	  || __builtin_mul_overflow (element_size, dst_offsets[0], &dst_off)
	  || __builtin_mul_overflow (element_size, src_offsets[0], &src_off))
	return EINVAL;
      memcpy ((char *) dst + dst_off, (char *) src + src_off, length);
      ret = 1;
      return ret ? 0 : EINVAL;
    }

  /* FIXME: it would be nice to have some plugin function to handle
     num_dims == 2 and num_dims == 3 more efficiently.  Larger ones can
     be handled in the generic recursion below, and for host-host it
     should be used even for any num_dims >= 2.  */

  for (i = 1; i < num_dims; i++)
    if (__builtin_mul_overflow (dst_slice, dst_dimensions[i], &dst_slice)
	|| __builtin_mul_overflow (src_slice, src_dimensions[i], &src_slice))
      return EINVAL;
  if (__builtin_mul_overflow (dst_slice, dst_offsets[0], &dst_off)
      || __builtin_mul_overflow (src_slice, src_offsets[0], &src_off))
    return EINVAL;
  for (j = 0; j < volume[0]; j++)
    {
      ret = omp_target_memcpy_rect_worker ((char *) dst + dst_off,
					   (char *) src + src_off,
					   element_size, num_dims - 1,
					   volume + 1, dst_offsets + 1,
					   src_offsets + 1, dst_dimensions + 1,
					   src_dimensions + 1);
      if (ret)
	return ret;
      dst_off += dst_slice;
      src_off += src_slice;
    }
  return 0;
}

int
omp_target_memcpy_rect (void *dst, void *src, size_t element_size,
			int num_dims, const size_t *volume,
			const size_t *dst_offsets,
			const size_t *src_offsets,
			const size_t *dst_dimensions,
			const size_t *src_dimensions,
			int dst_device_num, int src_device_num)
{
  if (!dst && !src)
    return INT_MAX;

  if (dst_device_num != GOMP_DEVICE_HOST_FALLBACK)
    return EINVAL;
  if (src_device_num != GOMP_DEVICE_HOST_FALLBACK)
    return EINVAL;

  int ret = omp_target_memcpy_rect_worker (dst, src, element_size, num_dims,
					   volume, dst_offsets, src_offsets,
					   dst_dimensions, src_dimensions);
  return ret;
}

int
omp_target_associate_ptr (void *host_ptr, void *device_ptr, size_t size,
			  size_t device_offset, int device_num)
{
  return EINVAL;
}

int
omp_target_disassociate_ptr (void *ptr, int device_num)
{
  return EINVAL;
}
