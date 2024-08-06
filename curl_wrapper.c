#include "curl_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

CurlResponse * new_CurlResponse(int response_is_text)
{
	CurlResponse * response = malloc(sizeof(CurlResponse));
	if(response == NULL)
	{
		fprintf(stderr, "[ERROR] new_CurlResponse: Failed to allocate for struct.\n");
		return NULL;
	}

	response->content = malloc(1);
	if(response->content == NULL)
	{
		fprintf(stderr, "[ERROR] new_CurlResponse: Failed to allocate for content.\n");
		return NULL;
	}

	response->size = 0;
	if(response_is_text == 1)
	{
		response->is_text = 1;
		response->content_as_text = (char *) response->content;
	}
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

	memcpy(response->content + response->size, buffer, realsize);

	response->size += realsize;

	if(response->is_text)
	{
		response->content_as_text = response->content; //Asegurarse de que los dos punteros siempre sean iguales, ya que content puede moverse con realloc
		response->content_as_text[response->size] = '\0';
	}

	return realsize;
}

//Esta función está hecha para ser un punto común entre curlw_get y curlw_get_as_text
int curlw_easy_download(char * url, CurlResponse * curl_response)
{
	fprintf(stderr, "[INFO] curlw_easy_download: Downloading from %s\n", url);
	CURL * curl;
	CURLcode curl_result;

	curl = curl_easy_init();
	if(curl == NULL)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to create an easy curl\n");
		return -1;
	}

	if(curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to set option to curl: CURLOPT_URL.\n");
		return -1;
	}

	//Especificar la función que va a usar curl de callback para escribir el contenido de la respuesta
	if(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_response_callback) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to set option to curl: CURLOPT_WRITEFUNCTION.\n");
		return -1;
	}

	//Pasarle el pointer del struct que curl va a usar para pasarle datos a la función que especificamos de callback
	if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_response) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to set option to curl: CURLOPT_WRITEDATA.\n");
		return -1;
	}
	
	#ifdef _WIN32
		//Abrir certificado CA de myanimelist.net sino no se puede establecer una conexión HTTPS en windows por algún motivo
		fprintf(stderr, "[INFO] curl_get_as_text: Setting CURLOPT_CAINFO.\n");

		if(curl_easy_setopt(curl, CURLOPT_CAINFO, "myanimelist-net.pem") != CURLE_OK)
		{
			fprintf(stderr, "[ERROR] curl_get_as_text: Failed to set option to curl: CURLOPT_CAINFO.\n");
			return -1;
		}
	#endif

	curl_result = curl_easy_perform(curl);
	if(curl_result != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curl_get_as_text: Failed to easy perform on curl with code %i.\n", curl_result);
		return -1;
	}

	fprintf(stderr, "[INFO] curl_get_as_text: Looks like the download from %s succeeded.\n", url);

	return 0;
}

CurlResponse * curlw_get_as_text(char * url)
{
	CurlResponse * curl_response = new_CurlResponse(1);
	if(curl_response == NULL)
	{
		fprintf(stderr, "[ERROR] curlw_get_as_text: Failed to create new CurlResponse.\n");
		return NULL;
	}

	if(curlw_easy_download(url, curl_response) != 0)
	{
		fprintf(stderr, "[ERROR] curlw_get_as_text: curlw_easy_download failed.\n");
		return NULL;
	}

	return curl_response;
}

CurlResponse * curlw_get(char * url)
{
	CurlResponse * curl_response = new_CurlResponse(0);
	if(curl_response == NULL)
	{
		fprintf(stderr, "[ERROR] curlw_get: Failed to create new CurlResponse.\n");
		return NULL;
	}

	if(curlw_easy_download(url, curl_response) != 0)
	{
		fprintf(stderr, "[ERROR] curlw_get: curlw_easy_download failed.\n");
		return NULL;
	}

	return curl_response;
}
