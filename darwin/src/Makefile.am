SUBDIRS = \
        wavelet \
        interface \
        matching \
        image_processing \
	math

INCLUDES = \
	-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src/interface \
	-I$(top_srcdir)/src/matching \
        -I$(top_srcdir)/src/image_processing \
	-I$(top_srcdir)/src/math \
	@GTK_CFLAGS@

bin_PROGRAMS = darwin

darwin_SOURCES = \
	main.cxx \
        Chain.cxx Chain.h \
        ConfigFile.cxx ConfigFile.h \
        constants.h \
        Contour.cxx Contour.h \
        Database.h \
        DatabaseFin.h \
	Error.h \
        feature.cxx feature.h \
	FloatContour.cxx FloatContour.h \
	mapContour.cxx mapContour.h \
	IntensityContour.cxx IntensityContour.h \
        Options.h \
	Outline.h Outline.cxx \
	Point.h \
        snake.cxx snake.h \
	support.cxx support.h \
        testReg.cxx testReg.h \
        utility.h \
	waveletUtil.cxx waveletUtil.h

darwin_LDADD = \
        -L./wavelet -lWLC \
        -L./interface -lInterface \
        -L./matching -lMatching \
        -L./image_processing -limage_processing \
	-L./math -lmath \
        -L./../jpeg-6b/lib/libjpeg.a -ljpeg\
        @GTK_LIBS@ $(INTLLIBS)

darwin_DEPENDENCIES = \
       $(top_srcdir)/src/wavelet/libWLC.a \
       $(top_srcdir)/src/interface/libInterface.a \
       $(top_srcdir)/src/matching/libMatching.a \
       $(top_srcdir)/src/image_processing/libimage_processing.a \
       $(top_srcdir)/src/math/libmath.a 

