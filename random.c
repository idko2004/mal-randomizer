#include "random.h"

#include <stdlib.h>

#include "process_anime.h"

int random_zero_to_max(int max)
{
	int r = rand() % max;
	if(r < 0) r *= -1;
	return r;
}

int get_index_of_random_anime(AnimeArrays * anime_arrays)
{
	int length = anime_arrays->arr_anime_names->length;
	int r = random_zero_to_max(length);
	return r;
}

