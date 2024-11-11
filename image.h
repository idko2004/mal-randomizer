#ifndef IMAGEDOWNLOADERH  /* Include guard */
#define IMAGEDOWNLOADERH

#include <gtk/gtk.h>

#include "ptrarr.h"

typedef struct
{
	GdkPixbufLoader * pixbuf_loader;
	GdkPixbuf * pixbuf;
} ImageForGtk;

ImageForGtk * new_ImageForGtk();
void free_ImageForGtk(ImageForGtk * image_for_gtk);

ImageForGtk * get_pixbuf_from_url(char * image_url);
Ptrarr * get_multiple_pixbufs_from_urls(Ptrarr * arr_images_urls);

#endif
