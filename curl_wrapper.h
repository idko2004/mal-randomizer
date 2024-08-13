#ifndef CURLWRAPPERH  /* Include guard */
#define CURLWRAPPERH

#include <stdio.h>

typedef struct
{
	void * content;
	int is_text; //1 si es texto, -1 si es un stream
	size_t size;
} CurlResponse;

CurlResponse * new_CurlResponse(int response_is_text); // Initialize a CurlResponse struct, when you finish using it you should free_CurlResponse()

void free_CurlResponse(CurlResponse * curl_response); //Frees the CurlResponse

//Esta función está hecha para ser un punto común entre curlw_get y curlw_get_as_text
int curlw_easy_download(char * url, CurlResponse * curl_response);

CurlResponse * curlw_get_as_text(char * url); //Downloads the specified url from the internet and interprets it as text. Creates a new CurlResponse and returns the contents in it.

CurlResponse * curlw_get(char * url); //Downlaods the specified url from the internet. Creates a new CurlResponse and returns the contents in it.


#endif
