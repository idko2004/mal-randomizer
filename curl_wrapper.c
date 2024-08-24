#include "curl_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "text_parser.h"

#define MAIN_PAGE_CERT_FILE "certs/myanimelist-net.pem";
#define CDN_CERT_FILE "certs/cdn-myanimelist-net.pem";

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
		char * content_as_text = response->content;
		content_as_text[response->size] = '\0';
	}

	return realsize;
}

char * get_cert_name(char * url)
{
	if(find_in_text("cdn.myanimelist.net", url, 0) != -1)
	{
		fprintf(stderr, "[INFO] using cert file: cdn-myanimelist-net.pem\n");
		return CDN_CERT_FILE;
	}
	else
	{
		fprintf(stderr, "[INFO] using cert file: myanimelist-net.pem\n");
		return MAIN_PAGE_CERT_FILE;
	}
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
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to create an easy curl\n");
		return -1;
	}

	if(curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_URL.\n");
		return -1;
	}

	//Especificar la función que va a usar curl de callback para escribir el contenido de la respuesta
	if(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_response_callback) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_WRITEFUNCTION.\n");
		return -1;
	}

	//Pasarle el pointer del struct que curl va a usar para pasarle datos a la función que especificamos de callback
	if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_response) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_WRITEDATA.\n");
		return -1;
	}
	
	#ifdef _WIN32
		//Abrir certificado CA de myanimelist.net sino no se puede establecer una conexión HTTPS en windows por algún motivo
		fprintf(stderr, "[INFO] curlw_easy_download: Setting CURLOPT_CAINFO.\n");

		if(curl_easy_setopt(curl, CURLOPT_CAINFO, get_cert_name(url)) != CURLE_OK)
		{
			fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_CAINFO.\n");
			return -1;
		}
	#endif

	curl_result = curl_easy_perform(curl);
	if(curl_result != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to easy perform on curl with code %i.\n", curl_result);
		return -1;
	}

	fprintf(stderr, "[INFO] curlw_easy_download: Looks like the download from %s succeeded.\n", url);

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
