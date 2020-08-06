//*******************************************************************
//   file: support.cxx
//
// author: ?
//
//   mods:
//
//*******************************************************************

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <string.h>

#include <gtk/gtk.h>

#include "support.h"
#include <stdio.h>
/* This is an internally used function to check if a pixmap file exists. */
#ifndef WIN32
static gchar* check_file_exists        (const gchar     *directory,
                                        const gchar     *filename);
#endif

/* This is an internally used function to create pixmaps. */
static GtkWidget* create_dummy_pixmap  (GtkWidget       *widget);

GtkWidget*
lookup_widget                          (GtkWidget       *widget,
                                        const gchar     *widget_name)
{
  GtkWidget *parent, *found_widget;

  for (;;)
    {
      if (GTK_IS_MENU (widget))
        parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
      else
        parent = widget->parent;
      if (parent == NULL)
        break;
      widget = parent;
    }

  found_widget = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (widget),
                                                   widget_name);
  if (!found_widget)
    g_warning ("Widget not found: %s", widget_name);
  return found_widget;
}

/* This is a dummy pixmap we use when a pixmap can't be found. */
static char *dummy_pixmap_xpm[] = {
/* columns rows colors chars-per-pixel */
"1 1 1 1",
"  c None",
/* pixels */
" "
};

/* This is an internally used function to create pixmaps. */
static GtkWidget*
create_dummy_pixmap                    (GtkWidget       *widget)
{
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d (NULL, colormap, &mask,
                                                     NULL, dummy_pixmap_xpm);
  if (gdkpixmap == NULL)
    g_error ("Couldn't create replacement pixmap.");
  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

static GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
add_pixmap_directory                   (const gchar     *directory)
{
  pixmaps_directories = g_list_prepend (pixmaps_directories,
                                        g_strdup (directory));
}

/* This is an internally used function to create pixmaps. */
GtkWidget*
create_pixmap                          (GtkWidget       *widget,
                                        const gchar     *filename)
{
  gchar *found_filename = NULL;
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;
  GList *elem;

  if (!filename || !filename[0])
      return create_dummy_pixmap (widget);

  /* We first try any pixmaps directories set by the application. */
  elem = pixmaps_directories;
  while (elem)
    {
#ifndef WIN32
      found_filename = check_file_exists ((gchar*)elem->data, filename);
#endif
	  if (found_filename)
        break;
      elem = elem->next;
    }

  /* If we haven't found the pixmap, try the source directory. */
  if (!found_filename) {
#ifndef WIN32
      found_filename = check_file_exists ("../pixmaps", filename);
#endif
  }

  if (!found_filename)
    {
      g_warning (_("Couldn't find pixmap file: %s"), filename);
      return create_dummy_pixmap (widget);
    }

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap, &mask,
                                                   NULL, found_filename);
  if (gdkpixmap == NULL)
    {
      g_warning (_("Error loading pixmap file: %s"), found_filename);
      g_free (found_filename);
      return create_dummy_pixmap (widget);
    }
  g_free (found_filename);
  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

GtkWidget *create_pixmap_from_data(GtkWidget * widget, gchar ** data)
{
    GdkColormap *colormap;
    GdkPixmap *gdkpixmap;
    GdkBitmap *mask;
    GtkWidget *pixmap;

    colormap = gtk_widget_get_colormap(widget);
    gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &mask,
						    NULL, data);
    if (gdkpixmap == NULL) {
	g_warning(_("Error loading pixmap data."));
	return create_dummy_pixmap(widget);
    }
    pixmap = gtk_pixmap_new(gdkpixmap, mask);
    gdk_pixmap_unref(gdkpixmap);
    gdk_bitmap_unref(mask);
    return pixmap;
}

/* This is an internally used function to check if a pixmap file exists. */
#ifndef WIN32
gchar* check_file_exists(const gchar *directory, const gchar     *filename)
{
  gchar *full_filename;
  struct stat s;
  gint status;

  full_filename = (gchar*) g_malloc (strlen (directory) + 1
                                     + strlen (filename) + 1);
  strcpy (full_filename, directory);
  strcat (full_filename, G_DIR_SEPARATOR_S);
  strcat (full_filename, filename);

  status = stat (full_filename, &s);
  if (status == 0 && S_ISREG (s.st_mode))
    return full_filename;
  g_free (full_filename);
  return NULL;
}
#endif


//This functions creates a new cursor.

//GdkColor white = {0,0xFFFF,0xFFFF,0xFFFF};
//GdkColor black = {0,0x0000,0x0000,0x0000};

GdkCursor *newCursorFromXPM(gchar *xpm[],
        gchar blackLetter,GdkColor *blackColor,
        gchar whiteLetter,GdkColor *whiteColor,
        gint xHot,gint yHot)
{
    gint height;
    gint width;
    gint colors;
    gint pchars;
    gint x;
    gint y;
    GdkBitmap *bitmap;
    GdkBitmap *mask;
    GdkCursor *cursor;

    guchar *bitmapData;
    guchar *maskData;
    gint byteIndex = 0;
    gint bitCount = 0;

    sscanf(xpm[0],"%d %d %d %d",
            &height,&width,&colors,&pchars);
    g_assert(pchars == 1);

    bitmapData = (guchar *)g_malloc((width * height) / 8);
    maskData = (guchar *)g_malloc((width * height) / 8);

    for(y=(colors+4); y < (height+4); y++) {
        for(x=0; x<width; x++) {
            if(xpm[y][x] == whiteLetter) {
                maskData[byteIndex] |= 0x80;
                bitmapData[byteIndex] |= 0x80;
            } else if(xpm[y][x] == blackLetter) {
                maskData[byteIndex] |= 0x80;
            }
            if(++bitCount == 8) {
                byteIndex++;
                bitCount = 0;
            } else {
                maskData[byteIndex] >>= 1;
                bitmapData[byteIndex] >>= 1;
            }
        }
    }
    bitmap = gdk_bitmap_create_from_data(NULL,(const gchar *)bitmapData,
            width,height);
    mask = gdk_bitmap_create_from_data(NULL,(const gchar *)maskData,
            width,height);
    cursor = gdk_cursor_new_from_pixmap(bitmap,mask,
            whiteColor,blackColor,
            xHot,yHot);
    g_free(bitmapData);
    g_free(maskData);
    return(cursor);
}
	
void create_gdk_pixmap(
	GtkWidget *widget,
	GdkPixmap **pixmap,
	GdkBitmap **mask,
	const gchar * filename
)
{
    gchar *found_filename = NULL;
    GdkColormap *colormap;
    GList *elem;

    /* We first try any pixmaps directories set by the application. */
    elem = pixmaps_directories;
    while (elem) {
#ifndef WIN32
		found_filename = check_file_exists((gchar *) elem->data, filename);
#endif
		if (found_filename)
	    break;
	elem = elem->next;
    }

    /* If we haven't found the pixmap, try the source directory. */
    if (!found_filename) {
#ifndef WIN32
		found_filename = check_file_exists("../pixmaps", filename);
#endif
	}

    if (!found_filename) {
	g_warning(_("Couldn't find pixmap file: %s"), filename);
	return;
    }

    colormap = gtk_widget_get_colormap(widget);
    *pixmap = gdk_pixmap_colormap_create_from_xpm(NULL, colormap, mask,
						    NULL, found_filename);
    if (pixmap == NULL) {
	g_warning(_("Error loading pixmap file: %s"), found_filename);
	g_free(found_filename);
	return;
    }
    g_free(found_filename);
}

void create_gdk_pixmap_from_data(
	GtkWidget *widget,
	GdkPixmap **pixmap,
	GdkBitmap **mask,
	gchar **data
)
{
    GdkColormap *colormap;

    colormap = gtk_widget_get_colormap(widget);
    *pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, mask,
						    NULL, data);
    if (pixmap == NULL) {
	g_warning(_("Error loading pixmap."));
	return;
    }
}
