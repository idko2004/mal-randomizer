#ifndef PROCESSANIMEH  /* Include guard */
#define PROCESSANIMEH

#include "cJSON/cJSON.h"
#include "strarr.h"

typedef struct
{
	Strarr * arr_anime_names;
	Strarr * arr_anime_names_eng;
	Strarr * arr_anime_urls;
	Strarr * arr_anime_images_paths;
} AnimeArrays;

AnimeArrays * process_anime(cJSON * mal_data_items_json);

#endif
