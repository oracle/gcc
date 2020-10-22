/* dostime.c - convert dos time to/from time_t.

   Copyright (C) 2002 Free Software Foundation
   Copyright (C) 2007 Dalibor Topic

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>

#include <stddef.h>

#include "dostime.h"

/*
 * The specification to which this was written.  From Joe Buck.
 * The DOS format appears to have only 2 second resolution.  It is an
 * unsigned long, and ORs together
 * 
 * (year-1980)<<25
 * month<<21  (month is tm_mon + 1, 1=Jan through 12=Dec)
 * day<<16    (day is tm_mday, 1-31)
 * hour<<11   (hour is tm_hour, 0-23)
 * min<<5	   (min is tm_min, 0-59)
 * sec>>1	   (sec is tm_sec, 0-59, that's right, we throw away the LSB)
 * 
 * DOS uses local time, so the localtime() call is used to turn the time_t
 * into a struct tm.
 */

time_t
dos2unixtime (ub4 dostime)
{
  struct tm ltime;
  time_t now = time (NULL);

  /* Call localtime to initialize timezone in TIME.  */
  ltime = *localtime (&now);

  ltime.tm_year = (int) (dostime >> 25) + 80;
  ltime.tm_mon = (int) ((dostime >> 21) & 0x0f) - 1;
  ltime.tm_mday = (int) (dostime >> 16) & 0x1f;
  ltime.tm_hour = (int) (dostime >> 11) & 0x0f;
  ltime.tm_min = (int) (dostime >> 5) & 0x3f;
  ltime.tm_sec = (int) (dostime & 0x1f) << 1;

  ltime.tm_wday = -1;
  ltime.tm_yday = -1;
  ltime.tm_isdst = -1;

  return mktime (&ltime);
}

ub4
unix2dostime (time_t *unix_time)
{
  struct tm *ltime = localtime (unix_time);
  int year = ltime->tm_year - 80;
  if (year < 0)
    year = 0;

  return ((ub4) year << 25
	  | ((ub4) ltime->tm_mon + 1) << 21
	  | (ub4) ltime->tm_mday << 16
	  | (ub4) ltime->tm_hour << 11
	  | (ub4) ltime->tm_min << 5
	  | (ub4) ltime->tm_sec >> 1);
}
