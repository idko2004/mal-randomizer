#include "process_anime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON/cJSON.h"

#include "ptrarr.h"

AnimeArrays * process_anime(cJSON * mal_data_items_json, int * error) //Guarda los animes del json en varias arrays
{
	*error = 0;

	Ptrarr * arr_anime_names = ptrarr_new(50);
	Ptrarr * arr_anime_names_eng = ptrarr_new(50);
	Ptrarr * arr_anime_urls = ptrarr_new(50);
	Ptrarr * arr_anime_images_paths = ptrarr_new(50);
	if(arr_anime_names == NULL || arr_anime_urls == NULL || arr_anime_images_paths == NULL)
	{
		fprintf(stderr, "[INFO] process_anime: Failed to create Ptrarr.\n");
		*error = 1;
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
			*error = 2;
			return NULL;
		}

		//printf("%i: %s | %s | %s | %s\n", i, anime_title->valuestring, anime_title_eng->valuestring, anime_url->valuestring, anime_image_path->valuestring);

		if(anime_title->valuestring == NULL)
		{
			fprintf(stderr, "[INFO] process_anime: anime title is not a string\n");

			if(cJSON_IsNumber(anime_title) == 1)
			{
				fprintf(stderr, "[INFO] process_anime: Anime title is a number!\n");
				int size = snprintf(NULL, 0, "%i", anime_title->valueint); //Escribir a null para saber cuánto mide
				fprintf(stderr, "[INFO] process_anime: got size %i\n", size);

				char * name = malloc(sizeof(char) * (size + 1));
				if(name == NULL)
				{
					fprintf(stderr, "[ERROR] process_anime: Failed to malloc!.\n");
					return NULL;
				}

				snprintf(name, size + 1, "%i", anime_title->valueint); //Ahora sí copiar de verdad

				//Borrar el objeto que era un número y crear uno que sea un string
				anime_title = cJSON_CreateString(name);
				cJSON_ReplaceItemInObjectCaseSensitive(child, "anime_title", anime_title);

				fprintf(stderr, "[INFO] process_anime: copied name to json: %s\n", anime_title->valuestring);

				free(name);
			}
			else
			{
				fprintf(stderr, "[ERROR] process_anime: I don't know what happened to this anime.\n");
				continue;
			}
		}

		char * anime_name_jp_copy = malloc(sizeof(char) * strlen(anime_title->valuestring) +1);
		if(anime_name_jp_copy == NULL)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
			*error = 1;
			return NULL;
		}
		strcpy(anime_name_jp_copy, anime_title->valuestring);
		push_result = ptrarr_push(arr_anime_names, anime_name_jp_copy);
		//fprintf(stderr, "[INFO] process_anime: Name copied\n");

		if(strcmp(anime_title_eng->valuestring, "") == 0)
		{
			//Guardar el nombre en japonés si no hay nombre en inglés.
			//fprintf(stderr, "[INFO] process_anime: copying en name from jp\n");
			char * anime_name_en_copy = malloc(sizeof(char) * strlen(anime_title->valuestring) + 1);
			if(anime_name_en_copy == NULL)
			{
				fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
				*error = 1;
				return NULL;
			}fprintf(stderr, "[INFO] process_anime: Name copied\n");
			strcpy(anime_name_en_copy, anime_title->valuestring);
			push_result += ptrarr_push(arr_anime_names_eng, anime_name_en_copy);
			//fprintf(stderr, "[INFO] process_anime: eng name copied from jp name\n");
		}
		else
		{
			//fprintf(stderr, "[INFO] process_anime: copying name\n");
			char * anime_name_en_copy = malloc(sizeof(char) * strlen(anime_title_eng->valuestring) +1);
			if(anime_name_en_copy == NULL)
			{
				fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
				*error = 1;
				return NULL;
			}
			strcpy(anime_name_en_copy, anime_title_eng->valuestring);
			push_result += ptrarr_push(arr_anime_names_eng, anime_name_en_copy);
			//fprintf(stderr, "[INFO] process_anime: jp name copied\n");
		}

		char * anime_url_copy = malloc(sizeof(char) * strlen(anime_url->valuestring) + 1);
		if(anime_url_copy == NULL)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
			*error = 1;
			return NULL;
		}
		strcpy(anime_url_copy, anime_url->valuestring);
		push_result += ptrarr_push(arr_anime_urls, anime_url_copy);
		//fprintf(stderr, "[INFO] process_anime: url copied\n");

		char * image_path_copy = malloc(sizeof(char) * strlen(anime_image_path->valuestring) + 1);
		if(image_path_copy == NULL)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to allocate space to copy string.\n");
			*error = 1;
			return NULL;
		}
		strcpy(image_path_copy, anime_image_path->valuestring);
		push_result += ptrarr_push(arr_anime_images_paths, image_path_copy);
		//fprintf(stderr, "[INFO] process_anime: image url copied\n");

		if(push_result > 0)
		{
			fprintf(stderr, "[ERROR] process_anime: Failed to push to ptrarr %i times.\n", push_result);
			*error = 1;
			return NULL;
		}
		//fprintf(stderr, "[INFO] process_anime: pushed\n");

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
	if(result == NULL)
	{
		fprintf(stderr, "[ERROR] process_anime: Failed to allocate space for AnimeArrays.\n");
		*error = 1;
		return NULL;
	}

	result->arr_anime_names = arr_anime_names;
	result->arr_anime_names_eng = arr_anime_names_eng;
	result->arr_anime_images_paths = arr_anime_images_paths;
	result->arr_anime_urls = arr_anime_urls;

	return result;
}
