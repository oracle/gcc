## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_INT_FORMATTED_IO_SOURCES = siprintf.c siscanf.c

GENERAL_SOURCES = \
	$(GENERAL_INT_FORMATTED_IO_SOURCES) \
	snprintf.c			\
	sprintf.c			\
	sscanf.c			\
	vsnprintf.c			\
	vsprintf.c			\
	vfprintf.c			\
	vsscanf.c			

LIBADD_OBJS= $(lpfx)svfiprintf.$(oext) $(lpfx)svfprintf.$(oext)

libstdio_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstdio.la
libstdio_la_SOURCES = $(GENERAL_SOURCES)
libstdio_la_LIBADD = $(LIBADD_OBJS)
libstdio_la_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(COMPILE)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared


# Though small footprint nano-formatted-IO implementation is used
# when NEWLIB_NANO_FORMATTED_IO is enabled, we keep all rules for
# the other implementation of formatted IO including all i-family
# functions.  The object files in !NEWLIB_NANO_FORMATTED_IO version
# implementation will be neither compiled nor archived into final
# library, because they are not depended on by final makefile target.

if NEWLIB_NANO_FORMATTED_IO
# Rules compiling small-footprint nano-formatted-io implementation.
$(lpfx)nano-vfprintf.$(oext): nano-vfprintf.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/nano-vfprintf.c -o $@

$(lpfx)nano-vfprintf_i.$(oext): nano-vfprintf_i.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/nano-vfprintf_i.c -o $@

$(lpfx)nano-vfprintf_float.$(oext): nano-vfprintf_float.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/nano-vfprintf_float.c -o $@

$(lpfx)nano-svfprintf.$(oext): nano-vfprintf.c
	$(LIB_COMPILE) -fshort-enums -DSTRING_ONLY -c $(srcdir)/nano-vfprintf.c -o $@
endif

# This rule is needed so that libtool compiles vfiprintf before vfprintf.
# Otherwise libtool moves vfprintf.o and subsequently can't find it.

$(lpfx)vfprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/vfprintf.c -o $@

$(lpfx)vfiprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -fshort-enums -DINTEGER_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -fshort-enums -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfiprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -fshort-enums -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)vfwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)vfiwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -fshort-enums -DINTEGER_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -fshort-enums -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfiwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -fshort-enums -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

if NEWLIB_NANO_FORMATTED_IO
# Rules compiling small-footprint nano-formatted-io implementation.
$(lpfx)nano-vfscanf.$(oext): nano-vfscanf.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf.c -o $@

$(lpfx)nano-vfscanf_i.$(oext): nano-vfscanf_i.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf_i.c -o $@

$(lpfx)nano-vfscanf_float.$(oext): nano-vfscanf_float.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf_float.c -o $@

$(lpfx)nano-svfscanf.$(oext): nano-vfscanf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/nano-vfscanf.c -o $@
endif

$(lpfx)vfscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -c $(srcdir)/vfscanf.c -o $@

$(lpfx)vfiscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfiscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)vfwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)vfiwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfiwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

if NEWLIB_NANO_FORMATTED_IO
CHEWOUT_INT_FORMATTED_IO_FILES =\
	nano-vfprintf.def	\
	nano-vfprintf_i.def	\
	nano-vfprintf_float.def	\
	nano-vfscanf.def	\
	nano-vfscanf_i.def	\
	nano-vfscanf_float.def
else
CHEWOUT_INT_FORMATTED_IO_FILES =\
	diprintf.def		\
	siprintf.def		\
	siscanf.def		\
	vfprintf.def		\
	vfscanf.def		\
	viprintf.def		\
	viscanf.def
endif
CHEWOUT_FILES = \
	$(CHEWOUT_INT_FORMATTED_IO_FILES)	\
	clearerr.def		\
	dprintf.def		\
	fclose.def		\
	fcloseall.def		\
	fdopen.def		\
	feof.def		\
	ferror.def		\
	fflush.def		\
	fgetc.def		\
	fgetpos.def		\
	fgets.def		\
	fgetwc.def		\
	fgetws.def		\
	fileno.def		\
	fmemopen.def		\
	fopen.def		\
	fopencookie.def		\
	fpurge.def		\
	fputc.def		\
	fputs.def		\
	fputwc.def		\
	fputws.def		\
	fread.def		\
	freopen.def		\
	fseek.def		\
	fsetpos.def		\
	ftell.def		\
	funopen.def		\
	fwide.def		\
	fwrite.def		\
	getc.def		\
	getc_u.def		\
	getchar.def		\
	getchar_u.def		\
	getdelim.def		\
	getline.def		\
	gets.def		\
	getw.def		\
	getwc.def		\
	getwchar.def		\
	mktemp.def		\
	open_memstream.def	\
	perror.def		\
	putc.def		\
	putc_u.def		\
	putchar.def		\
	putchar_u.def		\
	puts.def		\
	putw.def		\
	putwc.def		\
	putwchar.def		\
	remove.def		\
	rename.def		\
	rewind.def		\
	setbuf.def		\
	setbuffer.def		\
	setlinebuf.def		\
	setvbuf.def		\
	sprintf.def		\
	sscanf.def		\
	swprintf.def		\
	swscanf.def		\
	tmpfile.def		\
	tmpnam.def		\
	ungetc.def		\
	ungetwc.def		\
	vfwprintf.def		\
	vfwscanf.def

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/stdio.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref

$(lpfx)fclose.$(oext): local.h
$(lpfx)fdopen.$(oext): local.h
$(lpfx)fflush.$(oext): local.h
$(lpfx)fgetwc.$(oext): local.h
$(lpfx)fgetws.$(oext): local.h
$(lpfx)findfp.$(oext): local.h
$(lpfx)fmemopen.$(oext): local.h
$(lpfx)fopen.$(oext): local.h
$(lpfx)fopencookie.$(oext): local.h
$(lpfx)fpurge.$(oext): local.h
$(lpfx)fputs.$(oext): fvwrite.h
$(lpfx)fputwc.$(oext): local.h
$(lpfx)fputws.$(oext): local.h fvwrite.h
$(lpfx)fread.$(oext): local.h
$(lpfx)freopen.$(oext): local.h
$(lpfx)fseek.$(oext): local.h
$(lpfx)ftell.$(oext): local.h
$(lpfx)funopen.$(oext): local.h
$(lpfx)fvwrite.$(oext): local.h fvwrite.h
$(lpfx)fwalk.$(oext): local.h
$(lpfx)fwide.$(oext): local.h
$(lpfx)fwprintf.$(oext): local.h
$(lpfx)fwrite.$(oext): local.h fvwrite.h
$(lpfx)fwscanf.$(oext): local.h
$(lpfx)getwc.$(oext): local.h
$(lpfx)getwchar.$(oext): local.h
$(lpfx)iscanf.$(oext): local.h
$(lpfx)makebuf.$(oext): local.h
$(lpfx)open_memstream.$(oext): local.h
$(lpfx)puts.$(oext): fvwrite.h
$(lpfx)putwc.$(oext): local.h
$(lpfx)putwchar.$(oext): local.h
$(lpfx)refill.$(oext): local.h
$(lpfx)scanf.$(oext): local.h
$(lpfx)setbuf.$(oext): local.h
$(lpfx)setvbuf.$(oext): local.h
$(lpfx)siprintf.$(oext): local.h
$(lpfx)siscanf.$(oext): local.h
$(lpfx)sniprintf.$(oext): local.h
$(lpfx)sprintf.$(oext): local.h
$(lpfx)sscanf.$(oext): local.h
$(lpfx)stdio.$(oext): local.h
if NEWLIB_NANO_FORMATTED_IO
$(lpfx)nano-svfprintf.$(oext): local.h nano-vfprintf_local.h
$(lpfx)nano-svfscanf.$(oext): local.h nano-vfscanf_local.h
endif
$(lpfx)svfiprintf.$(oext): local.h
$(lpfx)svfiscanf.$(oext): local.h floatio.h
$(lpfx)svfprintf.$(oext): local.h
$(lpfx)svfscanf.$(oext): local.h floatio.h
$(lpfx)swprintf.$(oext): local.h
$(lpfx)swscanf.$(oext): local.h
$(lpfx)ungetc.$(oext): local.h
$(lpfx)ungetwc.$(oext): local.h
if NEWLIB_NANO_FORMATTED_IO
$(lpfx)nano-vfprintf.$(oext): local.h nano-vfprintf_local.h
$(lpfx)nano-vfprintf_i.$(oext): local.h nano-vfprintf_local.h
$(lpfx)nano-vfprintf_float.$(oext): local.h floatio.h nano-vfprintf_local.h
$(lpfx)nano-vfscanf.$(oext): local.h nano-vfscanf_local.h
$(lpfx)nano-vfscanf_i.$(oext): local.h nano-vfscanf_local.h
$(lpfx)nano-vfscanf_float.$(oext): local.h floatio.h nano-vfscanf_local.h
endif
$(lpfx)vfiprintf.$(oext): local.h
$(lpfx)vfiscanf.$(oext): local.h floatio.h
$(lpfx)vfprintf.$(oext): local.h
$(lpfx)vfscanf.$(oext): local.h floatio.h
$(lpfx)vfwprintf.$(oext): local.h
$(lpfx)vfwscanf.$(oext): local.h
$(lpfx)viscanf.$(oext): local.h
$(lpfx)vscanf.$(oext): local.h
$(lpfx)vsiscanf.$(oext): local.h
$(lpfx)vsniprintf.$(oext): local.h
$(lpfx)vsscanf.$(oext): local.h
$(lpfx)vswprintf.$(oext): local.h
$(lpfx)vswscanf.$(oext): local.h
$(lpfx)vwprintf.$(oext): local.h
$(lpfx)vwscanf.$(oext): local.h
$(lpfx)wbuf.$(oext): local.h fvwrite.h
$(lpfx)wprintf.$(oext): local.h
$(lpfx)wscanf.$(oext): local.h
$(lpfx)wsetup.$(oext): local.h
