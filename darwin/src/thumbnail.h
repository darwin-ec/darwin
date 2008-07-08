//*******************************************************************
//   file: thumbnail.h
//
// author: J H Stewman 5/11/2007
//
//   mods:
// 
//*******************************************************************

#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <gdk/gdk.h>

void free_thumbnail(gpointer data);
char** copy_thumbnail(char ** data);

#endif // THUMNAIL_H