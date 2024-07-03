#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "cJSON/cJSON.h"

#include "curl_wrapper.h"
#include "text_parser.h"

int main(int argc, char ** argv)
{
	if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
	{
		fprintf(stderr, "[ERROR] main: failed to initialize curl globally.\n");
		return 1;
	}

	CurlResponse * mal_page = curlw_get_as_text("https://myanimelist.net/animelist/idko2004?status=6");

	if(mal_page == NULL)
	{
		fprintf(stderr, "[ERROR] mal_page is NULL, maybe the download failed.\n");
		return 1;
	}

	//printf("%s", mal_page->content);

	long int mal_data_items_start = find_in_text("data-items=\"", mal_page->content, 0);
	long int mal_data_items_end = find_in_text("\" data-broadcasts", mal_page->content, mal_data_items_start);

	char * mal_data_items = slice_text(mal_data_items_start + strlen("data-items=\""), mal_data_items_end, mal_page->content);

	mal_data_items = replace_all("&quot;", "\"", mal_data_items);

	fprintf(stderr, "[INFO] starting to parse json.\n");

	cJSON * mal_json = cJSON_Parse(mal_data_items);

	fprintf(stderr, "[INFO] json parsed.\n");

	char * string = cJSON_Print(mal_json);

	printf("%s", string);

	free(mal_data_items);
	cJSON_Delete(mal_json);
	free_CurlResponse(mal_page);
	curl_global_cleanup();

	return 0;
}
