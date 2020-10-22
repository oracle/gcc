# Process this with automake to create Makefile.in

SUBDIRS = lib
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I. -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_builddir)/lib
AM_LDFLAGS = -no-undefined
LDADD = lib/libgnu.la libfastjar_convenience.la $(ZLIBS)

bin_PROGRAMS = fastjar grepjar

noinst_LTLIBRARIES = libfastjar_convenience.la

libfastjar_convenience_la_SOURCES = \
	argv.c \
	argv.h \
	compress.c \
	compress.h \
	dostime.c \
	dostime.h \
	jartool.h \
	pushback.c \
	pushback.h \
	shift.c \
	shift.h \
	zipfile.h

nodist_libfastjar_convenience_la_SOURCES = $(nodist_libfastjar_la_SOURCES)

fastjar_SOURCES = jartool.c
grepjar_SOURCES = jargrep.c jargrep.h

info_TEXINFOS = fastjar.texi
man_MANS = fastjar.1 grepjar.1

EXTRA_DIST = \
	CHANGES \
	$(man_MANS) \
	texi2pod.pl

TEXI2POD = perl $(srcdir)/texi2pod.pl
POD2MAN = pod2man --center="GNU" --release=@VERSION@

.pod.1:
	-($(POD2MAN) --section=1 $< > $(@).T$$$$ && \
		mv -f $(@).T$$$$ $@) || \
		(rm -f $(@).T$$$$ && exit 1)

.INTERMEDIATE: fastjar.pod grepjar.pod

fastjar.pod: $(srcdir)/fastjar.texi
	-$(TEXI2POD) -D fastjar $< > $@

grepjar.pod: $(srcdir)/fastjar.texi
	-$(TEXI2POD) -D grepjar $< > $@

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# Support for static checkers
pscan:
	for i in $(libfastjar_convenience_la_SOURCES) \
		$(fastjar_SOURCES) $(grepjar_SOURCES); do \
		$(PSCAN) $(top_srcdir)/$$i ; \
	done

flawfinder:
	$(FLAWFINDER) $(top_srcdir)

rats:
	$(RATS) $(top_srcdir)

SPLINT_FLAGS=-I . -I $(srcdir)/lib -I $(srcdir) -DHAVE_CONFIG_H -DPRIx32= -warnposix +weak
splint:
	for i in $(libfastjar_convenience_la_SOURCES) \
		$(fastjar_SOURCES) $(grepjar_SOURCES); do \
		$(SPLINT) $(SPLINT_FLAGS) $(top_srcdir)/$$i ; \
	done


