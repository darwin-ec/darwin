/*
 * jconfig.h
 *
 * This file is NOT part of the original jpeg library source.  It is a 
 * redirection to allow use of same source to compile Win32 and Linux
 * versions of the DARWIN code using the libraries libjepeg.lib, jpeg.lib
 * and libjpeg.a
 *
 * JHS - 1/30/2006
 */

#ifdef WIN32
#include "jconfig.vc"    /* Visual Studio version of jconfig.h */
#else
#include "jconfig.unix"  /* UNIX/Linux version of jconfig.h from jconfig.doc */
#endif
