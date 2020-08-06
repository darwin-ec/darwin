//*******************************************************************
//   file: gtk-compat.h
//
// author: JHS (5/10/2005)
//
//   mods:
//
// necessary because GTK_WINDOW_DIALOG is NOT defined in Gtk+ versions
// 2.0.0 and later -- fix modeled after solution found online at
// http://mail.nessus.org/pipermail/nessus-cvs/2004-April/msg00109.html
//
// Broken GTK_WINDOW_DIALOG in versions 2.0 and later, so replace with
// GTK_WINDOW_POPUP if necessary
//
// All occurrences of GTK_WINDOW_DIALOG in DARWIN software are now 
// replaced with WINDOW_DIALOG 
//
//*******************************************************************

#ifndef GTK_COMPAT_H
#define GTK_COMPAT_H


#define GTK_ENABLE_BROKEN 1

#if GTK_CHECK_VERSION(2,0,0)
#define WINDOW_DIALOG GTK_WINDOW_TOPLEVEL
#else
#define WINDOW_DIALOG GTK_WINDOW_DIALOG
#endif


#endif

