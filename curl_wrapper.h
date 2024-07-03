#ifndef CURLWRAPPERH  /* Include guard */
#define CURLWRAPPERH

#include <stdio.h>

typedef struct
{
	char * content;
	size_t size;
} CurlResponse;

CurlResponse * new_CurlResponse(); // Initialize a CurlResponse struct, when you finish using it you should free_CurlResponse()

void free_CurlResponse(CurlResponse * curl_response); //Frees the CurlResponse

CurlResponse * curlw_get_as_text(char * url); //Downloads the specified url from the internet and interprets it as text. Creates a new CurlResponse and returns the contents in it.


#endif
