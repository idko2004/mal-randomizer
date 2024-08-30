#ifndef PROCESSANIMEH  /* Include guard */
#define PROCESSANIMEH

#include "cJSON/cJSON.h"
#include "ptrarr.h"

typedef struct
{
	Ptrarr * arr_anime_names;
	Ptrarr * arr_anime_names_eng;
	Ptrarr * arr_anime_urls;
	Ptrarr * arr_anime_images_paths;
} AnimeArrays;

AnimeArrays * process_anime(cJSON * mal_data_items_json, int * error);

#endif
