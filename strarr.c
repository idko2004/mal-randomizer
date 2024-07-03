#include "strarr.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PRINT_DEBUG 0

int calculate_pointer_arr_size(int arr_capacity) //Generalmente usado para calcular el tamaño de str_arr
{
	return sizeof(void*) * arr_capacity;
}

int calculate_int_arr_size(int arr_capacity) //Generalmente usado para calcular el tamaño de arr_spaces_reserved
{
	return sizeof(int) * arr_capacity;
}

Strarr * strarr_new(int capacity)
{
	if(capacity <= 0)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_new: capacity can't be that low\n");
		return NULL;
	}

	Strarr * strarr = malloc(sizeof(Strarr));

	if(strarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_new: failed to create Strarr\n");
		return NULL;		
	}

	strarr->str_arr = malloc(calculate_pointer_arr_size(capacity));
	strarr->arr_spaces_reserved = malloc(calculate_int_arr_size(capacity));
	if(strarr->str_arr == NULL || strarr->arr_spaces_reserved == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_new: failed to allocate memory for the array\n");
		return NULL;
	}

	for(int i = 0; i < capacity; i++) //Limpiar la array para que no tenga ram sucia
	{
		strarr->str_arr[i] = NULL;
		strarr->arr_spaces_reserved[i] = 0;
	}

	strarr->spaces_reserved = capacity;
	strarr->length = 0;

	return strarr;
}

int strarr_expand_array(Strarr * strarr, int new_capacity)
{
	if(strarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_expand_array: strarr is null\n");
		return 1;
	}

	if(new_capacity < 1 || new_capacity < strarr->spaces_reserved)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_expand_array: this should be used to expand an array and not to shrink it\n");
		return 1;
	}

	void * new_str_arr = realloc(strarr->str_arr, calculate_pointer_arr_size(new_capacity));
	void * new_arr_spaces_reserved = realloc(strarr->arr_spaces_reserved, calculate_int_arr_size(new_capacity));

	if(new_str_arr == NULL || new_arr_spaces_reserved == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_expand_array: failed to expand arrays\n");
		return 1;
	}

	strarr->str_arr = new_str_arr;
	strarr->arr_spaces_reserved = new_arr_spaces_reserved;
	strarr->spaces_reserved = new_capacity;

	return 0;
}

int strarr_expand_string(Strarr * strarr, int index, int new_capacity)
{
	if(strarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_expand_string: strarr is null\n");
		return 1;
	}

	if(index < 0 || index > strarr->length)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_expand_string: index out of bounds\n");
		return 1;
	}

	char * string = strarr->str_arr[index];
	int current_capacity = strarr->arr_spaces_reserved[index];

	if(string == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_expand_string: string is NULL\n");
		return 1;
	}

	if(new_capacity <= current_capacity)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_expand_string: this should be used to expand an string and not to shrink it\n");
		return 1;
	}

	char * new_string = realloc(string, new_capacity); //La nueva capacidad ya debe incluir el caracter NULL

	if(new_string == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_expand_string: failed to expand string\n");
		return 1;
	}

	strarr->str_arr[index] = new_string;
	strarr->arr_spaces_reserved[index] = new_capacity;

	return 0;
}


int strarr_push(Strarr * strarr, char * str)
{
	if(strarr == NULL || str == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_push: strarr or str are null\n");
		return 1;
	}

	int index = strarr->length;

	if(index + 1 > strarr->spaces_reserved) //Expandir el array en caso de que no quepa
	{
		if(strarr_expand_array(strarr, index + 1) == 1)
		{
			//Hubo un problema expandiendo el array
			if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_push: array is too small to add a new string, tried to expand but failed\n");
			return 1;
		}
	}

	int str_length = strlen(str) + 1;

	char * copy = malloc(str_length); //Reservar espacio en memoria para el string
	if(copy == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_push: failed to allocate string\n");
		return 1;
	}

	strcpy(copy, str); //Copiar el string al nuevo espacio reservado

	strarr->str_arr[index] = copy; //Guardar el puntero en el array
	strarr->arr_spaces_reserved[index] = str_length; //Guardar el espacio reservado para el string
	strarr->length++;

	return 0;
}

char * strarr_get(Strarr * strarr, int index)
{
	if(strarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_get: strarr is null\n");
		return NULL;
	}

	if(index < 0 || index >= strarr->length)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_get: index out of bounds\n");
		return NULL;
	}

	char * result = strarr->str_arr[index];
	if(PRINT_DEBUG >= 1 && result == NULL) fprintf(stderr, "[WARN] strarr_get: obtained string is null\n");

	return result;
}

int strarr_set(Strarr * strarr, int index, char * text)
{
	if(strarr == NULL || text == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[INFO] strarr_set: strarr or text are null\n");
		return 1;
	}

	if(index == strarr->length)
	{
		//Hacer push, ya que vendría a ser lo mismo
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[INFO] strarr_set: just pushing...\n");
		strarr_push(strarr, text);
		return 0;
	}

	if(strarr->str_arr[index] == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_set: there is not an string in that index, for adding strings in empty spaces use strarr_push\n");
		return 1;
	}

	int str_length = strlen(text) + 1;

	if(str_length > strarr->arr_spaces_reserved[index]) //Reservar más memoria si el string es más largo que el que está actualmente en ese lugar
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[INFO] strarr_set: expanding string\n");
		strarr_expand_string(strarr, index, str_length);
	}

	strcpy(strarr->str_arr[index], text);

	return 0;
}

int strarr_destroy_string(Strarr * strarr, int index)
{
	if(strarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_destroy_string: strarr is null\n");
		return 1;
	}

	if(index < 0 || index >= strarr->length)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_destroy_string: index out of bounds\n");
		return 1;
	}

	//Delete the string
	char * string_to_delete = strarr->str_arr[index];
	free(string_to_delete);
	
	//Mark the deleted string as NULL for deleting it from the array later
	strarr->str_arr[index] = NULL;
	strarr->arr_spaces_reserved[index] = -1;

	//Create a new array
	int new_capacity = strarr->length - 1;
	char ** new_str_arr = malloc(calculate_pointer_arr_size(new_capacity));
	int * new_arr_spaces_reserved = malloc(calculate_int_arr_size(new_capacity));

	if(new_str_arr == NULL || new_arr_spaces_reserved == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_destroy_string: The string was deleted, but there was a problem reconstructing the array, so now you have a NULL pointer in your strings array, or/and a string of -1 capacity, take care.\n");
		return 1;
	}

	//Copy all the char pointers to the new array
	int new_i = 0;
	for(int old_i = 0; old_i < strarr->length; old_i++)
	{
		if(strarr->str_arr[old_i] == NULL) continue;

		new_str_arr[new_i] = strarr->str_arr[old_i];
		new_arr_spaces_reserved[new_i] = strarr->arr_spaces_reserved[old_i];

		new_i++;
	}

	//Free the old array and assign the new one
	free(strarr->str_arr);
	free(strarr->arr_spaces_reserved);
	strarr->str_arr = new_str_arr;
	strarr->arr_spaces_reserved = new_arr_spaces_reserved;

	strarr->spaces_reserved = new_capacity; //Use the estimated capacity for the capacity value
	strarr->length = new_i; //Use the actual iterations we used to assign to the array as the length

	return 0;
}

int strarr_destroy_everything(Strarr * strarr)
{
	if(strarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] strarr_destroy_everything: can't destroy anything if you give me a null pointer :c\n");
		return 1;
	}

	//Free the strings
	for(int i = 0; i < strarr->length; i++)
	{
		free(strarr->str_arr[i]);
	}

	free(strarr->arr_spaces_reserved);
	free(strarr);

	return 0;
}

void strarr_print_all(Strarr * strarr)
{
	fprintf(stderr, "[INFO] PRINTING STRARR\n");
	for(int i = 0; i < strarr->length; i++)
	{
		fprintf(stderr, "[STRARR %i] %s\n", i, strarr_get(strarr, i));
	}
}

/*
int main()
{
		char * lyrics[] = 
	{
        "Twinkle", "twinkle", "little", "star",
        "How", "I", "wonder", "what", "you", "are",
        "Up", "above", "the", "world", "so", "high",
        "Like", "a", "diamond", "in", "the", "sky",
        "Twinkle", "twinkle", "little", "star",
        "How", "I", "wonder", "what", "you", "are"
    };

    if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: initialize\n");
	Strarr * arr = strarr_new(1);
	
	if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: loop\n");
	for(int i = 0 ; i < 32; i++)
	{
		printf(">>>%i\n", i);
		strarr_push(arr, lyrics[i]);

		for(int j = 0; j < arr->length; j++)
		{
			printf("%i: %s\n", j, strarr_get(arr, j));
		}
	}

	if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: destroy some\n");
	for(int i = 0; i < 16; i++)
	{
		printf("Destroying number %i\n", i);
		strarr_destroy_string(arr, 0);
	}

	if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: print everything again\n");
	for(int j = 0; j < arr->length; j++)
	{
		printf("%i: %s\n", j, strarr_get(arr, j));
	}

	if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: push\n");
	strarr_push(arr, "Enemy lasagna robust below wax");

	if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: get\n");
	printf("%s\n", strarr_get(arr, arr->length - 1));

	if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: set\n");
	strarr_set(arr, arr->length - 1, "tapa la patata");

	if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: get again\n");
	printf("%s\n", strarr_get(arr, arr->length - 1));

	if(PRINT_DEBUG >= 1) fprintf(stderr, "> main: destroy everything\n");
	strarr_destroy_everything(arr);
	return 0;
}
*/
