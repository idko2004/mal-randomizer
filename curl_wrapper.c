#include "curl_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

CurlResponse * new_CurlResponse()
{
	CurlResponse * response = malloc(sizeof(CurlResponse));
	response->content = malloc(1);
	response->size = 0;
	return response;
}

void free_CurlResponse(CurlResponse * curl_response)
{
	free(curl_response->content);
	free(curl_response);
}

size_t write_curl_response_callback(void * buffer, size_t size, size_t nmemb, void * userp)
{
	size_t realsize = size * nmemb;

	CurlResponse * response = (CurlResponse *) userp;

	char * ptr = realloc(response->content, response->size + realsize + 1);
	if(ptr == NULL)
	{
		fprintf(stderr, "Failed to write curl response.\n");
		return 0;
	}

	response->content = ptr;
	memcpy(&(response->content[response->size]), buffer, realsize);

	response->size += realsize;
	response->content[response->size] = '\0';

	return realsize;
}

CurlResponse * curlw_get_as_text(char * url)
{
	fprintf(stderr, "[INFO] curlw_get_as_text: Downloading from %s\n", url);
	CURL * curl;
	CURLcode curl_result;
	
	CurlResponse * curl_response = new_CurlResponse();

	/*if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to initialize curl globally.\n");
		return NULL;
	}*/

	curl = curl_easy_init();
	if(curl == NULL)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to create an easy curl\n");
		return NULL;
	}

	if(curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to set option to curl: CURLOPT_URL.\n");
		return NULL;
	}

	//Especificar la función que va a usar curl de callback para escribir el contenido de la respuesta
	if(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_response_callback) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to set option to curl: CURLOPT_WRITEFUNCTION.\n");
		return NULL;
	}

	//Pasarle el pointer del struct que curl va a usar para pasarle datos a la función que especificamos de callback
	if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_response) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to set option to curl: CURLOPT_WRITEDATA.\n");
		return NULL;
	}

	curl_result = curl_easy_perform(curl);
	if(curl_result != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to easy perform on curl.\n");
		return NULL;
	}

	fprintf(stderr, "[INFO] curl_get_as_text: Looks like the download from %s succeeded.\n", url);

	return curl_response;
}
