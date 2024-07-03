#include "process_anime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON/cJSON.h"

#include "strarr.h"

AnimeArrays * process_anime(cJSON * mal_data_items_json) //Guarda los animes del json en varias arrays
{
	Strarr * arr_anime_names = strarr_new(10);
	Strarr * arr_anime_names_eng = strarr_new(10);
	Strarr * arr_anime_urls = strarr_new(10);
	Strarr * arr_anime_images_paths = strarr_new(10);
	if(arr_anime_names == NULL || arr_anime_urls == NULL || arr_anime_images_paths == NULL)
	{
		fprintf(stderr, "[INFO] process_anime: Failed to create Strarr.\n");
		return NULL;
	}

	cJSON * child = mal_data_items_json->child;
	cJSON * anime_title;
	cJSON * anime_title_eng;
	cJSON * anime_url;
	cJSON * anime_image_path;
	int push_result = 0;

	while(1)
	{
		anime_title = cJSON_GetObjectItemCaseSensitive(child, "anime_title");
		anime_title_eng = cJSON_GetObjectItemCaseSensitive(child, "anime_title_eng");
		anime_url = cJSON_GetObjectItemCaseSensitive(child, "anime_url");
		anime_image_path = cJSON_GetObjectItemCaseSensitive(child, "anime_image_path");
		if(anime_title == NULL || anime_title_eng == NULL || anime_url == NULL || anime_image_path == NULL)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to get object item in json.\n");
			return NULL;
		}

		push_result = strarr_push(arr_anime_names, anime_title->valuestring);
		push_result += strarr_push(arr_anime_names_eng, anime_title_eng->valuestring);
		push_result += strarr_push(arr_anime_urls, anime_url->valuestring);
		push_result += strarr_push(arr_anime_images_paths, anime_image_path->valuestring);
		if(push_result > 0)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to push to strarr %i times.\n", push_result);
			return NULL;
		}

		child = child->next;
		if(child == NULL)
		{
			break;
		}
	}

	strarr_print_all(arr_anime_names);
	strarr_print_all(arr_anime_names_eng);

	AnimeArrays * result = malloc(sizeof(AnimeArrays));
	result->arr_anime_names = arr_anime_names;
	result->arr_anime_names_eng = arr_anime_names_eng;
	result->arr_anime_images_paths = arr_anime_images_paths;
	result->arr_anime_urls = arr_anime_urls;

	return result;
}
