#include "process_anime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON/cJSON.h"

#include "ptrarr.h"

AnimeArrays * process_anime(cJSON * mal_data_items_json) //Guarda los animes del json en varias arrays
{
	Ptrarr * arr_anime_names = ptrarr_new(10);
	Ptrarr * arr_anime_names_eng = ptrarr_new(10);
	Ptrarr * arr_anime_urls = ptrarr_new(10);
	Ptrarr * arr_anime_images_paths = ptrarr_new(10);
	if(arr_anime_names == NULL || arr_anime_urls == NULL || arr_anime_images_paths == NULL)
	{
		fprintf(stderr, "[INFO] process_anime: Failed to create Ptrarr.\n");
		return NULL;
	}

	cJSON * child = mal_data_items_json->child;
	cJSON * anime_title;
	cJSON * anime_title_eng;
	cJSON * anime_url;
	cJSON * anime_image_path;
	int push_result = 0;
	int i = 0;

	printf("-- Listing all entries:\n");

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

		//printf("%i: %s | %s\n", i, anime_title->valuestring, anime_title_eng->valuestring);

		char * anime_name_jp_copy = malloc(sizeof(char) * strlen(anime_title->valuestring) +1);
		if(anime_name_jp_copy == NULL)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
			return NULL;
		}
		strcpy(anime_name_jp_copy, anime_title->valuestring);
		push_result = ptrarr_push(arr_anime_names, anime_name_jp_copy);

		if(strcmp(anime_title_eng->valuestring, "") == 0)
		{
			//Guardar el nombre en japonés si no hay nombre en inglés.
			char * anime_name_en_copy = malloc(sizeof(char) * strlen(anime_title->valuestring) + 1);
			if(anime_name_en_copy == NULL)
			{
				fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
				return NULL;
			}
			strcpy(anime_name_en_copy, anime_title->valuestring);
			push_result += ptrarr_push(arr_anime_names_eng, anime_name_en_copy);
			fprintf(stderr, "[INFO] process_anime: anime %s will use jp name only.\n", anime_name_en_copy);
		}
		else
		{
			char * anime_name_en_copy = malloc(sizeof(char) * strlen(anime_title_eng->valuestring) +1);
			if(anime_name_en_copy == NULL)
			{
				fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
				return NULL;
			}
			strcpy(anime_name_en_copy, anime_title_eng->valuestring);
			push_result += ptrarr_push(arr_anime_names_eng, anime_name_en_copy);
		}

		char * anime_url_copy = malloc(sizeof(char) * strlen(anime_url->valuestring) + 1);
		if(anime_url_copy == NULL)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
			return NULL;
		}
		strcpy(anime_url_copy, anime_url->valuestring);
		push_result += ptrarr_push(arr_anime_urls, anime_url_copy);

		char * image_path_copy = malloc(sizeof(char) * strlen(anime_image_path->valuestring) + 1);
		if(image_path_copy == NULL)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
			return NULL;
		}
		strcpy(image_path_copy, anime_image_path->valuestring);
		push_result += ptrarr_push(arr_anime_images_paths, image_path_copy);

		if(push_result > 0)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to push to ptrarr %i times.\n", push_result);
			return NULL;
		}

		printf("%i: (%p)%s | (%p)%s\n", i,
			ptrarr_get(arr_anime_names, i),
			(char *) ptrarr_get(arr_anime_names, i),
			ptrarr_get(arr_anime_names_eng, i),
			(char *) ptrarr_get(arr_anime_names_eng, i)
		);

		child = child->next;
		if(child == NULL)
		{
			break;
		}

		i++;
	}

	//strarr_print_all(arr_anime_names);
	//strarr_print_all(arr_anime_names_eng);

	AnimeArrays * result = malloc(sizeof(AnimeArrays));
	result->arr_anime_names = arr_anime_names;
	result->arr_anime_names_eng = arr_anime_names_eng;
	result->arr_anime_images_paths = arr_anime_images_paths;
	result->arr_anime_urls = arr_anime_urls;

	return result;
}
