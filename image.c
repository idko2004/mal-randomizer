#include "image.h"

#include <gtk/gtk.h>

#include "curl_wrapper.h"
#include "ptrarr.h"
#include "text_parser.h"

#define BASE64_START_URL_STRING "data:image/svg+xml;base64,"

ImageForGtk * new_ImageForGtk()
{
	ImageForGtk * img = malloc(sizeof(ImageForGtk));
	if(img == NULL)
	{
		fprintf(stderr, "[ERROR] new_ImageForGtk: Failed to malloc.\n");
		return NULL;
	}

	img->pixbuf = NULL;
	img->pixbuf_loader = NULL;

	return img;
}

void free_ImageForGtk(ImageForGtk * image_for_gtk)
{
	if(image_for_gtk->pixbuf_loader != NULL)
	{
		g_object_unref(image_for_gtk->pixbuf_loader);
	}

	if(image_for_gtk->pixbuf != NULL)
	{
		g_object_unref(image_for_gtk->pixbuf);
	}

	free(image_for_gtk);
}

CurlResponse * download_image(char * image_url)
{
	CurlRequest * request = new_CurlRequest();
	request->url = image_url;
	request->method = CURLW_METHOD_GET;
	request->response_is_text = 0;

	int curl_error_code = 0;
	CurlResponse * image_response = curlw_go(request, &curl_error_code);
	if(curl_error_code != 0)
	{
		fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: Failed to download image: curl code = %i: %s\n", curl_error_code, curlw_get_error_message(curl_error_code));
		return NULL;
	}
	if(image_response == NULL)
	{
		fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: Failed to download image.\n");
		return NULL;
	}

	free_CurlRequest(request);

	return image_response;
}

//Retorna 0 si es base64
int is_base64(char * url)
{
	if(find_in_text(BASE64_START_URL_STRING, url, 0) == -1)
	{
		//No es base64
		return 1;
	}
	else return 0;
}

//Public function to download or process an image from a url
ImageForGtk * get_pixbuf_from_url(char * image_url)
{
	void * image_contents = NULL;
	
	#ifdef _WIN32
		long long unsigned int image_size = 0; //Por algún motivo windows necesita que sea más largo.
	#else
		long unsigned int image_size = 0;
	#endif

	fprintf(stderr, "[INFO] download_image_and_get_pixbuf is here!\n");

	if(image_url == NULL)
	{
		fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: image url is NULL.\n");
		return NULL;
	}

	//fprintf(stderr, "[INFO] download_image_and_get_pixbuf: Printing url:\n%s\n", image_url);

	if(is_base64(image_url) == 0)
	{
		fprintf(stderr, "[INFO] download_image_and_get_pixbuff: Image is base64.\n");

		//Quitar la primera parte de la url para poder pasárselo al decodificador de base64
		char * encoded_image = replace_all(BASE64_START_URL_STRING, "", image_url);
		if(encoded_image == NULL)
		{
			fprintf(stderr, "[ERROR] Couldn't find base64 in url even though is_base64 said that indeed, it was base64.\n");
			return NULL;
		}

		fprintf(stderr, "[INFO] download_image_and_get_pixbuf: Decoding base64 image.\n");

		unsigned char * decoded_image = g_base64_decode(encoded_image, &image_size);
		
		image_contents = (void *) decoded_image;
		free(encoded_image);
	}
	else
	{
		//No es base64
		fprintf(stderr, "[INFO] download_image_and_get_pixbuf: Image has to be downloaded.\n");

		CurlResponse * image_response = download_image(image_url);
		if(image_response == NULL)
		{
			fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: Failed to download image.\n");
			return NULL;
		}

		if(image_response->body->content == NULL)
		{
			fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: Somehow the contents of the image are not present.\n");
			return NULL;
		}

		void * content = malloc(image_response->body->size);
		if(content == NULL)
		{
			fprintf(stderr, "[ERROR] Failed to allocate for content.\n");
			return NULL;
		}

		memcpy(content, image_response->body->content, image_response->body->size);

		image_contents = content;
		image_size = image_response->body->size;

		free_CurlResponse(image_response);
	}

	fprintf(stderr, "[INFO] download_image_and_get_pixbuf: Trying to put the image in a pixbuf.\n");

	//Load the image to a pixbuf
	GdkPixbufLoader * pixbuf_loader = gdk_pixbuf_loader_new();
	GError * g_error_write = NULL, * g_error_close = NULL;

	if(pixbuf_loader == NULL)
	{
		fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: Failed to create a new pixbuf loader.\n");
		return NULL;
	}
	fprintf(stderr, "[INFO] download_image_and_get_pixbuf: writing to pixbuf loader.\n");
	if(gdk_pixbuf_loader_write(pixbuf_loader, image_contents, image_size, &g_error_write) == FALSE)
	{
		fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: Failed to write to pixbuf loader. Code: %i, message: %s\n", g_error_write->code, g_error_write->message);
		return NULL;
	}
	fprintf(stderr, "[INFO] download_image_and_get_pixbuf: closing pixbuf loader.\n");
	if(gdk_pixbuf_loader_close(pixbuf_loader, &g_error_close) == FALSE)
	{
		fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: Failed to close pixbuf loader. Code: %i, message: %s\n", g_error_close->code, g_error_close->message);
		return NULL;
	}

	free(image_contents);

	fprintf(stderr, "[INFO] download_image_and_get_pixbuf: getting pixbuf.\n");
	GdkPixbuf * pixbuf = gdk_pixbuf_loader_get_pixbuf(pixbuf_loader);
	if(pixbuf == NULL)
	{
		fprintf(stderr, "[ERROR] download_image_and_get_pixbuf: Resultant pixbuf is NULL :c.\n");
		return NULL;
	}

	//Create struct to return
	ImageForGtk * result = new_ImageForGtk();
	if(result == NULL)
	{
		fprintf(stderr, "[ERROR] Failed to create struct for result.\n");
		return NULL;
	}
	result->pixbuf_loader = pixbuf_loader;
	result->pixbuf = pixbuf;

	//g_object_ref(pixbuf); //Para poder separar al pixbuf_loader del pixbuf y que no se extrañen mucho.
	//g_object_unref(pixbuf_loader);
	
	return result;
}

//Public function to download or process multiple images from a url
Ptrarr * get_multiple_pixbufs_from_urls(Ptrarr * arr_images_urls)
{
	Ptrarr * arr_pixbuf = ptrarr_new(arr_images_urls->length);
	if(arr_pixbuf == NULL)
	{
		fprintf(stderr, "[ERROR] download_multiple_images_and_get_pixbufs: Failed to create ptrarr.\n");
		return NULL;
	}

	for(int i = 0; i < arr_images_urls->length; i++)
	{
		ImageForGtk * image_for_gtk = get_pixbuf_from_url((char *) ptrarr_get(arr_images_urls, i));
		ptrarr_push(arr_pixbuf, image_for_gtk);
	}

	return arr_pixbuf;
}

/*


*/
