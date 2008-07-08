## Process this file with automake to produce Makefile.in

## Current version 8/10/2005

SUBDIRS = intl m4 po src
EXTRA_DIST = config.rpath BUGS TODO

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi
	if test -d msvc; then \
	  mkdir $(distdir)/msvc; \
	  for m in msvc/*; do \
	    if test -f $$m; then \
	      cp -p $$m $(distdir)/msvc; \
	    fi \
	  done \
	fi
	if test -d filters; then \
	  mkdir $(distdir)/filters; \
	  for f in filters/*; do \
	    if test -f $$f; then \
	      cp -p $$f $(distdir)/filters; \
	    fi \
	  done \
	fi

ACLOCAL_AMFLAGS = -I m4
