#include "curl_wrapper.h"

#include <curl/urlapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>

#include "text_parser.h"

#define USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36 GLS/100.10.9939.100"
#define MAIN_PAGE_CERT_FILE "certs/myanimelist-net.pem";
#define CDN_CERT_FILE "certs/cdn-myanimelist-net.pem";

CurlRequest * new_CurlRequest()
{
	CurlRequest * req = malloc(sizeof(CurlRequest));
	if(req == NULL)
	{
		fprintf(stderr, "[ERROR] new_CurlRequest: Failed to allocate for struct.\n");
		return NULL;
	}

	req->url = NULL;
	req->method = CURLW_METHOD_GET;
	req->body_contents = NULL;
	req->cookies = NULL;
	req->referer = NULL;
	req->response_is_text = 1;

	return req;
}

void free_CurlRequest(CurlRequest * curl_request)
{
	free(curl_request);
}

CurlData * new_CurlData(int response_is_text)
{
	CurlData * data = malloc(sizeof(CurlData));
	if(data == NULL)
	{
		fprintf(stderr, "[ERROR] new_CurlData: Failed to allocate for struct.\n");
		return NULL;
	}

	data->content = malloc(1);
	if(data->content == NULL)
	{
		fprintf(stderr, "[ERROR] new_CurlData: Failed to allocate for content.\n");
		return NULL;
	}

	data->size = 0;
	if(response_is_text == 1)
	{
		data->is_text = 1;
	}
	return data;
}

void free_CurlData(CurlData * curl_data)
{
	free(curl_data->content);
	free(curl_data);
}

CurlResponse * new_CurlResponse(int body_is_text)
{
	CurlResponse * curl_response = malloc(sizeof(CurlResponse));
	if(curl_response == NULL)
	{
		fprintf(stderr, "[ERROR] new_CurlResponseCombo: Failed to create struct.\n");
		return NULL;
	}

	CurlData * header = new_CurlData(1);
	CurlData * body = new_CurlData(body_is_text);
	if(header == NULL || body == NULL)
	{
		fprintf(stderr, "[ERROR] new_CurlResponseCombo: Failed to create header and body.\n");
		return NULL;
	}

	curl_response->header = header;
	curl_response->body = body;
	curl_response->httpCode = 0;

	return curl_response;
}

void free_CurlResponse(CurlResponse * curl_response)
{
	free_CurlData(curl_response->header);
	free_CurlData(curl_response->body);
	free(curl_response);
}

char * curlw_get_error_message(int curl_error_code)
{
	switch(curl_error_code)
	{
		case -1: return "Ocurrió un error configurando la petición HTTP.";
		case 0: return "Todo salió bien.";
		case 3: return "La URL no está formateada correctamente.";
		case 6: return "No se pudo resolver el nombre del servidor.";
		case 7: return "No se pudo conectar con el servidor.";
		case 28: return "El servidor tardó demasiado en responder";
		case 58: return "Parece que los certificados no son válidos.";
		case 60: return "El servidor no parece seguro.";
		case 77: return "Parece que los certificados no son válidos o no se puede acceder a ellos.";
		case 78: return "La URL a la que se intentó acceder parece no ser válida.";
		default: return "I don't know ._.";
	}
}

size_t write_curl_data_callback(void * buffer, size_t size, size_t nmemb, void * userp)
{
	size_t realsize = size * nmemb;

	CurlData * data = (CurlData *) userp;

	char * ptr = realloc(data->content, data->size + realsize + 1);
	if(ptr == NULL)
	{
		fprintf(stderr, "Failed to write curl data.\n");
		return 0;
	}

	data->content = ptr;

	memcpy(data->content + data->size, buffer, realsize);

	data->size += realsize;

	if(data->is_text)
	{
		char * content_as_text = data->content;
		content_as_text[data->size] = '\0';
		//printf("buffer = %s\n", content_as_text);
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
int curlw_easy_download(CURLU * url, int method, char * body_contents, char * cookies, char * referer, CurlResponse * curl_response, int * curl_error_code)
{
	char * full_url;
	curl_url_get(url, CURLUPART_URL, &full_url, 0);
	fprintf(stderr, "[INFO] curlw_easy_download: Downloading from \"%s\"\n", full_url);

	CURL * curl;
	CURLcode curl_result;
	*curl_error_code = -1;

	curl = curl_easy_init();
	if(curl == NULL)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to create an easy curl\n");
		return -1;
	}

	//Poner la URL
	if(curl_easy_setopt(curl, CURLOPT_CURLU, url) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_CURLU.\n");
		return -1;
	}

	//Poner User Agent
	if(curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_USERAGENT.\n");
		return -1;
	}

	//Poner cookies
	if(cookies != NULL)
	{
		if(curl_easy_setopt(curl, CURLOPT_COOKIE, cookies) != CURLE_OK)
		{
			fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_COOKIE.\n");
			return -1;
		}
	}

	//Poner el referer, es un enlace que indica que página ha hecho la petición
	if(referer != NULL)
	{
		if(curl_easy_setopt(curl, CURLOPT_REFERER, referer) != CURLE_OK)
		{
			fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_REFERER.\n");
			return -1;
		}
	}

	//Especificar la función que va a usar curl de callback para escribir el contenido de la respuesta
	if(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_data_callback) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_WRITEFUNCTION.\n");
		return -1;
	}

	//Pasarle el pointer del struct que curl va a usar para pasarle datos a la función que especificamos de callback
	if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_response->body) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_WRITEDATA.\n");
		return -1;
	}

	if(curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_curl_data_callback) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_HEADERFUNCTION.\n");
		return -1;
	}

	if(curl_easy_setopt(curl, CURLOPT_HEADERDATA, curl_response->header) != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_HEADERDATA.\n");
		return -1;
	}

	if(method == CURLW_METHOD_POST)
	{
		if(curl_easy_setopt(curl, CURLOPT_POST, 1) != CURLE_OK)
		{
			fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_POST.\n");
			return -1;
		}

		if(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_contents) != CURLE_OK)
		{
			fprintf(stderr, "[ERROR] curlw_easy_download: Failed to set option to curl: CURLOPT_POSTFIELDS.\n");
			return -1;
		}
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
	*curl_error_code = curl_result;
	if(curl_result != CURLE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_easy_download: Failed to easy perform on curl with code %i.\n", curl_result);
		return -1;
	}

	fprintf(stderr, "[INFO] curlw_easy_download: Looks like the download from %s succeeded.\n", full_url);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curl_response->httpCode);

	curl_easy_cleanup(curl);

	return 0;
}

CurlResponse * curlw_go(CurlRequest * curl_request, int * error_code_return)
{
	if(curl_request == NULL)
	{
		fprintf(stderr, "[ERROR] curlw_go: curl_request is NULL.\n");
		return NULL;
	}

	if(curl_request->response_is_text != 0 && curl_request->response_is_text != 1)
	{
		fprintf(stderr, "[ERROR] curlw_go: response_is_text has an invalid value, only 0 and 1 are accepted, current value = %i\n", curl_request->response_is_text);
		return NULL;
	}

	if(curl_request->method != CURLW_METHOD_GET && curl_request->method != CURLW_METHOD_POST)
	{
		fprintf(stderr, "[ERROR] curlw_go: method has an invalid value, only CURLW_METHOD_GET and CURLW_METHOD_POST are accepted.\n");
		return NULL;
	}

	CurlResponse * curl_response = new_CurlResponse(curl_request->response_is_text);
	if(curl_response == NULL)
	{
		fprintf(stderr, "[ERROR] curlw_go: Failed to create curl_response\n");
		return NULL;
	}

	//CURLU * url_handler = parse_url(curl_request->url);
	CURLU * url_handler = curl_url();
	if(curl_url_set(url_handler, CURLUPART_URL, curl_request->url, 0) != CURLUE_OK)
	{
		fprintf(stderr, "[ERROR] curlw_go: Seems like the url failed to parse.\n");
		return NULL;
	}

	*error_code_return = 0;
	int result = curlw_easy_download
	(
		url_handler,
		curl_request->method,
		curl_request->body_contents,
		curl_request->cookies,
		curl_request->referer,
		curl_response,
		error_code_return
	);

	if(result != 0)
	{
		fprintf(stderr, "[ERROR] curlw_go: download failed with code %i (%s)\n", *error_code_return, curlw_get_error_message(*error_code_return));
		return NULL;
	}

	return curl_response;
}
/*
CurlResponse * curlw_get_as_text(char * url, int * curl_error_code)
{
	fprintf(stderr, "[WARN] Using deprecated function: curlw_get_as_text.\n");

	CurlResponse * curl_response = new_CurlResponse(1);
	if(curl_response == NULL)
	{
		fprintf(stderr, "[ERROR] curlw_get_as_text: Failed to create CurlResponse.\n");
		return NULL;
	}

	if(curlw_easy_download(url, CURLW_METHOD_GET, NULL, NULL, NULL, curl_response, curl_error_code) != 0)
	{
		fprintf(stderr, "[ERROR] curlw_get_as_text: curlw_easy_download failed.\n");
		return NULL;
	}

	return curl_response;
}

CurlResponse * curlw_get(char * url, int * curl_error_code)
{
	fprintf(stderr, "[WARN] Using deprecated function: curlw_get.\n");

	CurlResponse * curl_response = new_CurlResponse(0);
	if(curl_response == NULL)
	{
		fprintf(stderr, "[ERROR] curlw_get: Failed to create new CurlResponse.\n");
		return NULL;
	}

	if(curlw_easy_download(url, CURLW_METHOD_GET, NULL, NULL, NULL, curl_response, curl_error_code) != 0)
	{
		fprintf(stderr, "[ERROR] curlw_get: curlw_easy_download failed.\n");
		return NULL;
	}

	return curl_response;
}

CurlResponse * curlw_post(char * url, char * body_contents, int * curl_error_code)
{
	fprintf(stderr, "[WARN] Using deprecated function: curlw_post.\n");

	CurlResponse * curl_response = new_CurlResponse(1);
	if(curl_response == NULL)
	{
		fprintf(stderr, "[ERROR] curlw_post: Failed to create new CurlResponse.\n");
		return NULL;
	}

	if(curlw_easy_download(url, CURLW_METHOD_POST, body_contents, NULL, NULL, curl_response, curl_error_code) != 0)
	{
		fprintf(stderr, "[ERROR] curlw_post: curlw_easy_download failed.\n");
		return NULL;
	}

	return curl_response;
}
*/
