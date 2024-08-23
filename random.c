#include "random.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "process_anime.h"

int srand_configured = -1;

int random_zero_to_max(int max)
{
	if(srand_configured != 0) generate_seed();

	int r = rand();
	fprintf(stderr, "[INFO] random_zero_to_max: rand = %i\n", r);
	r = r % max;
	if(r < 0) r *= -1;
	return r;
}

int get_index_of_random_anime(AnimeArrays * anime_arrays)
{
	int length = anime_arrays->arr_anime_names->length;
	int r = random_zero_to_max(length);
	return r;
}

void generate_seed()
{

	time_t t = time(NULL);
	fprintf(stderr, "[INFO] generate_seed: time = %i\n", (int) t);
	srand(t);
}

