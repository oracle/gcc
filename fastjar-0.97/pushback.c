/*
  pushback.c - code for a pushback buffer to handle file I/O
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

#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "jartool.h"
#include "pushback.h"

void pb_init(pb_file *pbf, int fd){
  pbf->fd = fd;
  pbf->next = pbf->pb_buff;
  pbf->buff_amt = 0;
}

size_t pb_push(pb_file *pbf, void *buff, size_t amt){
  size_t in_amt;
  size_t wrap = 0;

#ifdef DEBUG
  printf("%d bytes being pushed back to the buffer\n", amt);
#endif

  /* determine how much we can take */
  if((RDSZ - pbf->buff_amt) < amt)
    in_amt = RDSZ - pbf->buff_amt;
  else
    in_amt = amt;

  if(in_amt == 0)
    return 0;

  /* figure out if we need to wrap around, and if so, by how much */
  if((size_t) ((pbf->pb_buff + RDSZ) - pbf->next) < in_amt)
    wrap = in_amt - ((pbf->pb_buff + RDSZ) - pbf->next);

  /* write everything up til the end of the buffer */
  memcpy(pbf->next, buff, (in_amt - wrap));

  /* finish writing what's wrapped around */
  memcpy(pbf->pb_buff, ((char *)buff + (in_amt - wrap)), wrap);
         
  /* update the buff_amt field */
  pbf->buff_amt += in_amt;

#ifdef DEBUG
  printf("%d bytes we can't accept\n", (amt - in_amt));
#endif

  return in_amt;
}

size_t pb_read(pb_file *pbf, void *buff, size_t amt){
  size_t out_amt = 0;
  size_t wrap = 0;
  void *bp = buff;
  size_t tmp;

#ifdef DEBUG
  printf("%d bytes requested from us\n", amt);
#endif
  while(out_amt < amt){
    /* if our push-back buffer contains some data */
    if(pbf->buff_amt > 0){
      
#ifdef DEBUG
      printf("giving data from buffer\n");
#endif
      
      /* calculate how much we can actually give the caller */
      if( (amt - out_amt) < pbf->buff_amt )
        tmp = (amt - out_amt);
      else
        tmp = pbf->buff_amt;
      
      /* Determine if we're going to need to wrap around the buffer */
      if(tmp > ((size_t) ((pbf->pb_buff + RDSZ) - pbf->next)))
        wrap = tmp - ((pbf->pb_buff + RDSZ) - pbf->next);
      
      memcpy(bp, pbf->next, (tmp - wrap));
      bp = &(((char *)bp)[tmp - wrap]);
      
      /* If we need to wrap, read from the start of the buffer */
      if(wrap > 0){
        memcpy(bp, pbf->pb_buff, wrap);
        bp = &(((char *)bp)[wrap]);
      }
      
      /* update the buff_amt field */
      pbf->buff_amt -= tmp;
      pbf->next += tmp;
      
#ifdef DEBUG
      printf("%d bytes remaining in buffer\n", pbf->buff_amt);
#endif
      
      /* if the buffer is empty, reset the next header to the front of the
         buffer so subsequent pushbacks/reads won't have to wrap */
      if(pbf->buff_amt == 0)
        pbf->next = pbf->pb_buff;
      
      out_amt += tmp;

    } else {
      ssize_t num_read;
#ifdef DEBUG
      printf("Reading from file..\n");
#endif
      
      /* The pushback buffer was empty, so we just need to read from the file */
      num_read = read(pbf->fd, bp, (amt - out_amt));
      if(-1 == num_read) {
        perror("read");
        exit(EXIT_FAILURE);
      }
      if(num_read == 0)
        break;
      else {
        tmp = (size_t) num_read;
        out_amt += tmp;
      }
      
      bp = &(((char *)bp)[tmp]);
    }
  }

#ifdef DEBUG
  printf("managed to read %d bytes\n", out_amt);
#endif
  return out_amt;
}
