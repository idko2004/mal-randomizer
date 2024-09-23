#ifndef CURLWRAPPERH  /* Include guard */
#define CURLWRAPPERH

#include <stdio.h>
#include <curl/curl.h>

#define CURLW_METHOD_GET 0
#define CURLW_METHOD_POST 1

//Struct en el que se definen los parámetros de la llamada HTTP antes de realizarse.
typedef struct
{
	char * url;
	int method;
	char * body_contents;
	char * cookies;
	char * referer;
	int response_is_text;
} CurlRequest;

//Struct que contiene los datos descargados del sitio web, suele usarse uno para el header y otro para el body.
typedef struct
{
	void * content;
	int is_text; //1 si es texto, -1 si no
	size_t size;
} CurlData;

//Struct que unifica el header y el body, suele ser el struct que terminan retornando los métodos.
typedef struct
{
	CurlData * header;
	CurlData * body;
	int httpCode;
} CurlResponse;

CurlRequest * new_CurlRequest();
void free_CurlRequest(CurlRequest * curl_request);

CurlData * new_CurlData(int response_is_text); // Initialize a CurlResponse struct, this is where the downloaded stuff gets saved, when you finish using it you should free_CurlResponse()
void free_CurlData(CurlData * curl_data); //Frees the CurlResponse

CurlResponse * new_CurlResponse(int body_is_text); //Initialize a CurlResponse struct, in this struct we save both the header and the body of the response as CurlData, this automatically creates two instances of CurlData, when you finish using it you shoud free_CurlResponse.
void free_CurlResponse(CurlResponse * curl_response); //Frees the CurlResponse and the CurlData asociated with it;

//Esta función está hecha para ser un punto común entre curlw_get y curlw_get_as_text
//char * url; 	Aquí va la URL a la que te quieres conectar
//int method; 	El método http que deseas utilizar, usa CURLW_METHOD_* para elegir una.
//char * body_contents; 	El cuerpo de la llamada http, si el método es get se ignora.
//char * cookies;	String que contiene las cookies que enviar al servidor, si es NULL no se envía nada.
//CurlResponse * curl_response; 	 Un puntero de una instancia curl_response que se haya iniciado antes, en este struct se guardarán el encabezado y el cuerpo de la respuesta.
//int * curl_error_code; 	Un puntero a un int, Si ocurre algún error luego puedes revisar el contenido del int para saber cuál fue el error que ocurrió.
int curlw_easy_download(CURLU * url, int method, char * body_contents, char * cookies, char * referer, CurlResponse * curl_response, int * curl_error_code);

CurlResponse * curlw_go(CurlRequest * curl_request, int * error_code_return); //Realiza una petición HTTP con los parámetros específicados en curl_request y retorna curl_response con la información ;obtenida del servidor.

//MEANT FOR DEBUG
CURLU * parse_url(char * url);

//DEPRECATED
/*
CurlResponse * curlw_get_as_text(char * url, int * curl_error_code); //Downloads the specified url from the internet and interprets it as text. Creates a new CurlResponse and returns the contents in it.

CurlResponse * curlw_get(char * url, int * curl_error_code); //Downlaods the specified url from the internet. Creates a new CurlResponse and returns the contents in it.

CurlResponse * curlw_post(char * url, char * body_contents, int * curl_error_code);
*/
char * curlw_get_error_message(int curl_error_code); //Returns a string that describes a curl error code


#endif
