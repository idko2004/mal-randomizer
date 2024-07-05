#include "seed.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define PRINT_DEBUG 0

char * seedfile_path = NULL; //archivo que guarda la seed
char * ifile_path = NULL; //Archivo que guarda las iteraciones que hacer con esa seed

int create_program_directory(char * dir_path)
{
	if(access(dir_path, F_OK) == 0)
	{
		//El directorio existe
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- The directory '%s' alredy exists\n", dir_path);
		return 0; //No hay problemas, todo bien
	}

	if(mkdir(dir_path, 0777) != 0)
	{
		//Error al crear el directorio
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- Failed to create program directory '%s'\n", dir_path);
		return 1; //D:
	}

	if(PRINT_DEBUG == 1) fprintf(stderr, "-- The seedfile has a new home: '%s'\n", dir_path);
	return 0; //Supuestamente se creó todo bien
}


int set_seedfile_path(char * path, char * program_folder)
{
	char * filename_seed = "seed";
	char * filename_i = "i";

	if(path[strlen(path) - 1] == '/' || path[strlen(path) - 1] == '\\') //Remover el último caracter si es un / para que no haya un // sin querer
	{
		path[strlen(path) - 1] = '\0';
	}

	//Concatenar las dos partes
	seedfile_path = malloc(strlen(path) + strlen(program_folder) + strlen(filename_seed) + 1);
	ifile_path = malloc(strlen(path) + strlen(program_folder) + strlen(filename_i) + 1);

	if(seedfile_path == NULL || ifile_path == NULL)
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- failed to allocate memory for filepaths\n");
		return 1;
	}

	strcpy(seedfile_path, path); // "/home/user/.local/share"
	strcat(seedfile_path, program_folder); // "/home/user/.local/share/idko2004-random-c/"

	strcpy(ifile_path, seedfile_path); //Tener lo mismo en ambas variables

	if(create_program_directory(seedfile_path) != 0) //Antes de añadir el nombre del archivo al seedfile_path, verificar si el directorio existe, así si no existe se puede crear antes de añadirle el nombre del archivo.
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- Due to the seedfile being homeless, it will be freed an NULLED\n");
		free(seedfile_path);
		free(ifile_path);
		seedfile_path = NULL;
		ifile_path = NULL;
		return 1;
	}

	strcat(seedfile_path, filename_seed); // "/home/user/.local/share/idko2004-random-c/seed"
	strcat(ifile_path, filename_i); // "/home/user/.local/share/idko2004-random-c/i"

	if(PRINT_DEBUG == 1) fprintf(stderr, "-- Seedfile path set to '%s'\n--Iterations file set to '%s'\n", seedfile_path, ifile_path);

	return 0;
}

void find_seedfile_path()
{
	char * random_c_path = getenv("RANDOM_C_PATH");
	if(random_c_path != NULL)
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- RANDOM_C_PATH variable found: %s\n", random_c_path);
		set_seedfile_path(random_c_path, "/");
		return;
	}

	char * xdg_data_home = getenv("XDG_DATA_HOME");
	if(xdg_data_home != NULL)
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- XDG_DATA_HOME variable found: %s\n", xdg_data_home);
		set_seedfile_path(xdg_data_home, "/idko2004-random-c/");
		return;
	}

	char * home = getenv("HOME");
	if(home != NULL)
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- HOME variable found: %s\n", home);
		set_seedfile_path(home, "/.local/share/idko2004-random-c/");
		return;
	}

	if(PRINT_DEBUG == 1) fprintf(stderr, "-- No RANDOM_C_PATH, XDG_DATA_HOME, or HOME env vars found\n");
	seedfile_path = "seed"; //Así debería crear el archivo en el directorio del proyecto o en pwd, no sé cual
	ifile_path = "i";
}

int get_seed(time_t *seed, int * iterations)
{
	if(seedfile_path == NULL || ifile_path == NULL)
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- seedfile_path (or ifile_path) is NULL, so I can't get the seed.\n");
		return 1;
	}

	FILE *seedfileptr = fopen(seedfile_path, "rb");
	FILE *ifileptr = fopen(ifile_path, "rb");

	if(seedfileptr == NULL || ifileptr == NULL)
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- Failed to open seed file as readable, maybe it doesn't exist yet.\n");
		return 1;
	}

	fread(seed, sizeof(*seed), 1, seedfileptr);
	fread(iterations, sizeof(*iterations), 1, ifileptr);
	fclose(seedfileptr);
	fclose(ifileptr);

	return 0;
}

int save_seed(time_t *seed, int iterations)
{
	if(seedfile_path == NULL || ifile_path == NULL)
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- seedfile_path (or ifile_path) is NULL, so I can't save the seed.\n");
		return 1;
	}

	FILE *seedfileptr = fopen(seedfile_path, "wb");
	FILE *ifileptr = fopen(ifile_path, "wb");

	if(seedfileptr == NULL || ifileptr == NULL)
	{
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- Failed to open seed file as writable\n");
		return 1;
	}

	fwrite(seed, sizeof(*seed), 1, seedfileptr);
	fwrite(&iterations, sizeof(iterations), 1, ifileptr);
	fclose(seedfileptr);
	fclose(ifileptr);

	free(seedfile_path);
	free(ifile_path);
	seedfile_path = NULL;
	ifile_path = NULL;

	return 0;
}

void generate_seed()
{
	find_seedfile_path();

	time_t seed;
	int random_iterations = 0; //Si la seed coincide, generar un número aleatorio nuevo

	int result = get_seed(&seed, &random_iterations);

	if(result == 1) //File does not exist
	{
		time(&seed);
		save_seed(&seed, random_iterations);
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- Default seed: %ld\n", seed);
		
		srand(seed);
		return;
	}

	time_t new_seed = time(NULL);

	if(PRINT_DEBUG == 1)
	{
		fprintf(stderr, "-- Old seed: %ld\n", seed);
		fprintf(stderr, "-- New seed: %ld\n", new_seed);
	}

	if(seed == new_seed) //Same seed
	{
		random_iterations++;
		save_seed(&seed, random_iterations);
		if(PRINT_DEBUG == 1) fprintf(stderr, "-- Same seed!\n-- Doing random numbers %i times\n", random_iterations);
		srand(seed);

		for(int i = 0; i <= random_iterations; i++)
		{
			rand(); //Usar el random el número de veces que estuviese guardado para que no salga el mismo resultado, no sé que tan cutre sea esto.
		}

		return;
	}

	seed = new_seed;
	save_seed(&seed, 0);

	srand(seed);
}
