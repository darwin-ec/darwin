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
        -I$(top_srcdir)/png \
        @GTK_CFLAGS@

bin_PROGRAMS = darwin

darwin_SOURCES = \
        main.cxx \
        CatalogScheme.h \
        CatalogSupport.cxx CatalogSupport.h \
        Chain.cxx Chain.h \
        ConfigFile.cxx ConfigFile.h \
        constants.h \
        Contour.cxx Contour.h \
        Database.cxx Database.h \
        DatabaseFin.h \
        DummyDatabase.h \
        Error.h \
        feature.cxx feature.h \
        FloatContour.cxx FloatContour.h \
        mapContour.cxx mapContour.h \
        IntensityContour.cxx IntensityContour.h \
        IntensityContourCyan.cxx IntensityContourCyan.h \
        OldDatabase.cxx OldDatabase.h \
        Options.h \
        Outline.h Outline.cxx \
        Point.h \
        SQLiteDatabase.cxx SQLiteDatabase.h \
        snake.cxx snake.h \
        sqlite3.c sqlite3.h \
        support.cxx support.h \
        thumbnail.cxx thumbnail.h \
        utility.h \
        waveletUtil.cxx waveletUtil.h

darwin_LDADD = \
        -L./wavelet -lWLC \
        -L./interface -lInterface \
        -L./matching -lMatching \
        -L./image_processing -limage_processing \
        -L./math -lmath \
        -L$(HOME)/gtk/inst/lib/ -ljpeg \
        -L./../png -lPNGsupport \
        -lpng \
        -ldl \
        @GTK_LIBS@ $(INTLLIBS)

darwin_DEPENDENCIES = \
       $(top_srcdir)/src/wavelet/libWLC.a \
       $(top_srcdir)/src/interface/libInterface.a \
       $(top_srcdir)/src/matching/libMatching.a \
       $(top_srcdir)/src/image_processing/libimage_processing.a \
       $(top_srcdir)/src/math/libmath.a \
       $(top_srcdir)/png/libPNGsupport.a

darwin_CFLAGS = -Wno-narrowing make

darwin_CXXFLAGS = -pthread