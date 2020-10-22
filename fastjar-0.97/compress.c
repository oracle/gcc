/*
  compress.c - code for handling deflation
  Copyright (C) 1999  Bryan Burns
  Copyright (C) 2004  Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <zlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef STDC_HEADERS
#include <stdlib.h>
#endif

#include <sys/types.h>

#include "jartool.h"
#include "pushback.h"
#include "compress.h"
#include "shift.h"

static ssize_t write_data (int, void *, size_t, struct zipentry *);

static z_stream zs;

void init_compression(void){

  memset(&zs, 0, sizeof(z_stream));

  zs.zalloc = NULL;
  zs.zfree = NULL;
  zs.opaque = Z_NULL;

  /* Why -MAX_WBITS?  zlib has an undocumented feature, where if the windowbits
     parameter is negative, it omits the zlib header, which seems to kill
     any other zip/unzip program.  This caused me SO much pain.. */
  if(deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 
                  9, Z_DEFAULT_STRATEGY) != Z_OK){
    
    fprintf(stderr, "Error initializing deflation!\n");
    exit(EXIT_FAILURE);
  }
}

static ssize_t
write_data (int fd, void *buf, size_t len,
	    struct zipentry *ze)
{
  struct zipentry *next = NULL;
  off_t here = lseek (fd, 0, SEEK_CUR);
  /*
   * If we are updating and there is not enough space before the next
   * entry, expand the file.
   */
  if (ze)
    {
      next = ze->next_entry;
      if (next && here + len >= next->offset)
	{
	  if (shift_down (fd, next->offset, (here + len) - next->offset, next))
	    {
	      perror ("can't expand file");
	      exit(EXIT_FAILURE);
	    }
	}
    }

  return write (fd, buf, len);
}

int compress_file(int in_fd, int out_fd, struct zipentry *ze,
		  struct zipentry *existing)
{
  Bytef in_buff[RDSZ];
  Bytef out_buff[RDSZ];
  size_t rdamt;
  size_t wramt;
  unsigned long tr = 0;
  ssize_t rtval;
  ssize_t num_written;

  rdamt = 0;

  zs.avail_in = 0;
  zs.next_in = in_buff;
  
  zs.next_out = out_buff;
  zs.avail_out = (uInt)RDSZ;
  
  ze->crc = crc32(0L, Z_NULL, 0); 
  
  for(; ;){
    
    /* If deflate is out of input, fill the input buffer for it */
    if(zs.avail_in == 0 && zs.avail_out > 0){
      if((rtval = read(in_fd, in_buff, RDSZ)) == 0)
        break;

      if(rtval == -1){
        perror("read");
        exit(EXIT_FAILURE);
      }

      rdamt = (size_t) rtval;

      /* compute the CRC while we're at it */
      ze->crc = crc32(ze->crc, in_buff, rdamt); 

      /* update the total amount read sofar */
      tr += rdamt;

      zs.next_in = in_buff;
      zs.avail_in = (uInt) rdamt;
    }
    
    /* deflate the data */
    if(deflate(&zs, 0) != Z_OK){
      fprintf(stderr, "Error deflating! %s:%d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
    }
    
    /* If the output buffer is full, dump it to disk */
    if(zs.avail_out == 0){

      if (write_data (out_fd, out_buff, RDSZ, existing) != RDSZ)
	{
	  perror("write");
	  exit(EXIT_FAILURE);
	}

      /* clear the output buffer */
      zs.next_out = out_buff;
      zs.avail_out = (uInt)RDSZ;
    }

  }
  
  /* If we have any data waiting in the buffer after we're done with the file
     we can flush it */
  if(zs.avail_out < RDSZ){
    wramt = (size_t) RDSZ - zs.avail_out;
    num_written = write_data (out_fd, out_buff, wramt, existing);

    if (num_written == -1 || num_written != (ssize_t)wramt)
      {
	perror("write");
	exit(EXIT_FAILURE);
      }
    /* clear the output buffer */
    zs.next_out = out_buff;
    zs.avail_out = (uInt)RDSZ;
  }
  

  /* finish deflation.  This purges zlib's internal data buffers */
  while(deflate(&zs, Z_FINISH) == Z_OK){
    wramt = (size_t) RDSZ - zs.avail_out;
    num_written = write_data (out_fd, out_buff, wramt, existing);

    if (num_written == -1 || num_written != (ssize_t)wramt)
      {
	perror("write");
	exit(EXIT_FAILURE);
      }

    zs.next_out = out_buff;
    zs.avail_out = (uInt)RDSZ;
  }

  /* If there's any data left in the buffer, write it out */
  if(zs.avail_out != RDSZ){
    wramt = (size_t) RDSZ - zs.avail_out;
    num_written = write_data (out_fd, out_buff, wramt, existing);

    if (num_written == -1 || num_written != (ssize_t)wramt)
      {
	perror("write");
	exit(EXIT_FAILURE);
      }
  }

  /* update fastjar's entry information */
  ze->usize = (ub4)zs.total_in;
  ze->csize = (ub4)zs.total_out;

  /* Reset the deflation for the next time around */
  if(deflateReset(&zs) != Z_OK){
    fprintf(stderr, "Error resetting deflation\n");
    exit(EXIT_FAILURE);
  }
  
  return 0;
}

void end_compression(void){
  int rtval;

  /* Oddly enough, zlib always returns Z_DATA_ERROR if you specify no
     zlib header.  Go fig. */
  if((rtval = deflateEnd(&zs)) != Z_OK && rtval != Z_DATA_ERROR){
    fprintf(stderr, "Error calling deflateEnd\n");
    fprintf(stderr, "error: (%d) %s\n", rtval, zs.msg);
    exit(EXIT_FAILURE);
  }
}


void init_inflation(void){

  memset(&zs, 0, sizeof(z_stream));
    
  zs.zalloc = NULL;
  zs.zfree = NULL;
  zs.opaque = Z_NULL;
  
  if(inflateInit2(&zs, -15) != Z_OK){
    fprintf(stderr, "Error initializing deflation!\n");
    exit(EXIT_FAILURE);
  }

}

int inflate_file(pb_file *pbf, int out_fd, struct zipentry *ze){
  Bytef in_buff[RDSZ];
  Bytef out_buff[RDSZ];
  size_t rdamt;
  size_t num_pushed;
  int rtval;
  ub4 crc = 0;

  zs.avail_in = 0;

  crc = crc32(crc, NULL, 0); /* initialize crc */

  /* loop until we've consumed all the compressed data */
  for(;;){
    
    if(zs.avail_in == 0){
      if((rdamt = pb_read(pbf, in_buff, RDSZ)) == 0)
        break;

#ifdef DEBUG
      printf("%d bytes read\n", rdamt);
#endif

      zs.next_in = in_buff;
      zs.avail_in = (uInt) rdamt;
    }

    zs.next_out = out_buff;
    zs.avail_out = RDSZ;
    
    if((rtval = inflate(&zs, 0)) != Z_OK){
      if(rtval == Z_STREAM_END){
#ifdef DEBUG
        printf("end of stream\n");
#endif
        if(zs.avail_out != RDSZ){
          crc = crc32(crc, out_buff, (RDSZ - zs.avail_out));

          if(out_fd >= 0) {
            const size_t num_to_write = (size_t) RDSZ - zs.avail_out;
            const ssize_t num_written = write(out_fd, out_buff, num_to_write);
            if(num_written == -1 || num_written != (ssize_t) num_to_write){
              perror("write");
              exit(EXIT_FAILURE);
            }
          }
        }
        
        break;
      } else {
        fprintf(stderr, "Error inflating file! (%d)\n", rtval);
        exit(EXIT_FAILURE);
      }
    } else {
      if(zs.avail_out != RDSZ){
        crc = crc32(crc, out_buff, (RDSZ - zs.avail_out));

        if(out_fd >= 0) {
          const size_t num_to_write = (size_t) RDSZ - zs.avail_out;
          const ssize_t num_written = write(out_fd, out_buff, num_to_write);
          if(num_written == -1 || num_written != (ssize_t) num_to_write){
            perror("write");
            exit(EXIT_FAILURE);
          }
        }
        zs.next_out = out_buff;
        zs.avail_out = RDSZ;
      }
    }
  }
#ifdef DEBUG
  printf("done inflating\n");
#endif

#ifdef DEBUG
  printf("%d bytes left over\n", zs.avail_in);
#endif

#ifdef DEBUG    
  printf("CRC is %x\n", crc);
#endif

  ze->crc = crc;
  
  num_pushed = pb_push(pbf, zs.next_in, zs.avail_in);
  if (num_pushed != (size_t) zs.avail_in) {
    fprintf(stderr, "Pushback failure.");
    exit(EXIT_FAILURE);
  }

  ze->usize = zs.total_out;

  inflateReset(&zs);
  return 0;
}

/*
Function name: report_str_error
args:	val	Error code returned from zlib.
purpose: Put out an error message corresponding to error code returned from zlib.
Be suitably cryptic seeing I don't really know exactly what these errors mean.
*/

static void report_str_error(int val) {
	switch(val) {
	case Z_STREAM_END:
		break;
	case Z_NEED_DICT:
		fprintf(stderr, "Need a dictionary?\n");
		exit(EXIT_FAILURE);
	case Z_DATA_ERROR:
		fprintf(stderr, "Z_DATA_ERROR\n");
		exit(EXIT_FAILURE);
	case Z_STREAM_ERROR:
		fprintf(stderr, "Z_STREAM_ERROR\n");
		exit(EXIT_FAILURE);
	case Z_MEM_ERROR:
		fprintf(stderr, "Z_MEM_ERROR\n");
		exit(EXIT_FAILURE);
	case Z_BUF_ERROR:
		fprintf(stderr, "Z_BUF_ERROR\n");
		exit(EXIT_FAILURE);
	case Z_OK:
		break;
	default:
		fprintf(stderr, "Unknown behavior from inflate\n");
		exit(EXIT_FAILURE);
	}
}

/*
Function name: ez_inflate_str
args:	pbf		Pointer to pushback handle for file.
		csize	Compressed size of embedded file.
		usize	Uncompressed size of embedded file.
purpose: Read in and decompress the contents of an embedded file and store it in a
byte array.
returns: Byte array of uncompressed embedded file.
*/

static Bytef *ez_inflate_str(pb_file *pbf, ub4 csize, ub4 usize) {
	Bytef *out_buff;
	Bytef *in_buff;
	size_t rdamt;

	if((zs.next_in = in_buff = (Bytef *) malloc(csize))) {
		if((zs.next_out = out_buff = (Bytef *) malloc(usize + 1))) { 
			if((rdamt = pb_read(pbf, zs.next_in, csize)) == csize) {
				zs.avail_in = (uInt) csize;
				zs.avail_out = (uInt) usize;
				report_str_error(inflate(&zs, 0));
				free(in_buff);
				inflateReset(&zs);
				out_buff[usize] = (Bytef) '\0';
			}
			else {
				fprintf(stderr, "Read failed on input file.\n");
				fprintf(stderr, "Tried to read %lu but read %lu instead.\n", (unsigned long) csize, (unsigned long) rdamt);
				free(in_buff);
				free(out_buff);
				exit(EXIT_FAILURE);
			}
		}
		else {
			fprintf(stderr, "Malloc of out_buff failed.\n");
			fprintf(stderr, "Error: %s\n", strerror(errno));
			free(in_buff);
			exit(EXIT_FAILURE);
		}
	}
	else {
		fprintf(stderr, "Malloc of in_buff failed.\n");
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return out_buff;
}

/*
Function name: hrd_inflate_str
args:	pbf		Pointer to pushback handle for file.
		csize	Pointer to compressed size of embedded file.
		usize	Pointer to uncompressed size of embedded file.
purpose: Read and decompress an embedded file into a string.  Set csize and usize
accordingly.  This function does the reading for us in the case there is not size
information in the header for the embedded file.
returns: Byte array of the contents of the embedded file.
*/

static Bytef *hrd_inflate_str(pb_file *pbf, ub4 *csize, ub4 *usize) {
	Bytef *out_buff;
	Bytef *tmp;
	Bytef in_buff[RDSZ];
	size_t rdamt;
	size_t i;
	int zret;
        size_t num_pushed;

	i = 1; 
	out_buff = NULL;
	zret = Z_OK;
	while(zret != Z_STREAM_END && (rdamt = pb_read(pbf, in_buff, RDSZ)))
	{
		zs.avail_in = (uInt) rdamt;
		zs.avail_out = 0;
		zs.next_in = in_buff;
		do {
			if((tmp = (Bytef *) realloc(out_buff, (RDSZ * i) + 1))) {
				out_buff = tmp;
				zs.next_out = &(out_buff[(RDSZ * (i - 1)) - zs.avail_out]);
				zs.avail_out += RDSZ;
				i++;
			}
			else {
				fprintf(stderr, "Realloc of out_buff failed.\n");
				fprintf(stderr, "Error: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
		} while((zret = inflate(&zs, 0)) == Z_OK);
		report_str_error(zret);
	}
	num_pushed = pb_push(pbf, zs.next_in, zs.avail_in);
        if ((uInt) num_pushed != zs.avail_in) {
		fprintf(stderr, "Pushback failed.\n");
		exit(EXIT_FAILURE);
        }

	out_buff[(RDSZ * (i - 1)) - zs.avail_out] = (Bytef) '\0';
	*usize = zs.total_out;
	*csize = zs.total_in;

	inflateReset(&zs);

	return out_buff;
}

/*
Function name: inflate_string
args:	pbf		Pointer to pushback handle for file.
		csize	Pointer to compressed size of embedded file.  May be 0 if not set.
		usize	Pointer to uncompressed size of embedded file. May be 0 if not set.
purpose: Decide the easiest (in computer terms) methos of decompressing this embedded
file to a string.
returns: Pointer to a string containing the decompressed contents of the embedded file.
If csize and usize are not set set them to correct numbers.
*/

Bytef *inflate_string(pb_file *pbf, ub4 *csize, ub4 *usize) {
Bytef *ret_buf;

	if(*csize && *usize) ret_buf = ez_inflate_str(pbf, *csize, *usize);
	else ret_buf = hrd_inflate_str(pbf, csize, usize);

	return ret_buf;
}
