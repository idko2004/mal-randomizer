#include "image.h"

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "curl_wrapper.h"

int download_and_show_image(char * image_url, GtkBuilder * builder)
{
	CurlResponse * image_response = curlw_get(image_url);
	if(image_response == NULL)
	{
		fprintf(stderr, "[ERROR] download_and_show_image: Failed to download image.\n");
		return -1;
	}

	if(image_response->content == NULL)
	{
		fprintf(stderr, "[ERROR] download_and_show_image: Somehow the contents of the image are not present.\n");
		return -1;
	}

	//Load the image to a pixbuf
	fprintf(stderr, "[INFO] download_and_show_image: creating pixbuf.\n");

	GdkPixbufLoader * pixbuf_loader = gdk_pixbuf_loader_new();
	gdk_pixbuf_loader_write(pixbuf_loader, image_response->content, image_response->size, NULL);
	gdk_pixbuf_loader_close(pixbuf_loader, NULL);

	GdkPixbuf * pixbuf = gdk_pixbuf_loader_get_pixbuf(pixbuf_loader);


	//Set the image
	fprintf(stderr, "[INFO] download_and_show_image: setting image.\n");

	GObject * gtk_image = gtk_builder_get_object(builder, "animeImage");
	gtk_image_set_from_pixbuf(GTK_IMAGE(gtk_image), pixbuf);

	return 0;
}
