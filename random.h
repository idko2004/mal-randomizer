#ifndef RANDOMH  /* Include guard */
#define RANDOMH

#include "process_anime.h"

int random_zero_to_max(int max);

int get_index_of_random_anime(AnimeArrays * anime_arrays);

void generate_seed();

#endif
