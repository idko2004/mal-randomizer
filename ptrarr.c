#include "ptrarr.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PRINT_DEBUG 0

int calculate_pointer_arr_size(int arr_capacity) //Generalmente usado para calcular el tamaño de ptr_arr
{
	return sizeof(void*) * arr_capacity;
}

int calculate_int_arr_size(int arr_capacity) //Generalmente usado para calcular el tamaño de arr_spaces_reserved
{
	return sizeof(int) * arr_capacity;
}

Ptrarr * ptrarr_new(int capacity)
{
	if(capacity <= 0)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_new: capacity can't be that low\n");
		return NULL;
	}

	Ptrarr * ptrarr = malloc(sizeof(Ptrarr));

	if(ptrarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_new: failed to create Strarr\n");
		return NULL;		
	}

	ptrarr->ptr_arr = malloc(calculate_pointer_arr_size(capacity));
	if(ptrarr->ptr_arr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_new: failed to allocate memory for the array\n");
		return NULL;
	}

	for(int i = 0; i < capacity; i++) //Limpiar la array para que no tenga ram sucia
	{
		ptrarr->ptr_arr[i] = NULL;
	}

	ptrarr->spaces_reserved = capacity;
	ptrarr->length = 0;

	return ptrarr;
}

int ptrarr_expand(Ptrarr * ptrarr, int new_capacity)
{
	if(ptrarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_expand_array: ptrarr is null\n");
		return 1;
	}

	if(new_capacity < 1 || new_capacity < ptrarr->spaces_reserved)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_expand_array: this should be used to expand an array and not to shrink it\n");
		return 1;
	}

	void * new_ptr_arr = realloc(ptrarr->ptr_arr, calculate_pointer_arr_size(new_capacity));

	if(new_ptr_arr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_expand_array: failed to expand array\n");
		return 1;
	}

	ptrarr->ptr_arr = new_ptr_arr;
	ptrarr->spaces_reserved = new_capacity;

	return 0;
}

int ptrarr_push(Ptrarr * ptrarr, void * p)
{
	if(ptrarr == NULL || p == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_push: ptrarr or p are null\n");
		return 1;
	}

	int index = ptrarr->length;

	if(index + 1 > ptrarr->spaces_reserved) //Expandir el array en caso de que no quepa
	{
		if(ptrarr_expand(ptrarr, index + 1) == 1)
		{
			//Hubo un problema expandiendo el array
			if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_push: array is too small to add a new pointer, tried to expand but failed\n");
			return 1;
		}
	}

	ptrarr->ptr_arr[index] = p;
	ptrarr->length++;

	return 0;
}

void * ptrarr_get(Ptrarr * ptrarr, int index)
{
	if(ptrarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_get: ptrarr is null\n");
		return NULL;
	}

	if(index < 0 || index >= ptrarr->length)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_get: index out of bounds\n");
		return NULL;
	}

	return ptrarr->ptr_arr[index];
}

int ptrarr_set(Ptrarr * ptrarr, int index, void * p)
{
	if(ptrarr == NULL || p == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[INFO] ptrarr_set: ptrarr or p are null\n");
		return 1;
	}

	if(index < 0)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_set: index out of bounds\n");
		return 1;
	}

	if(index == ptrarr->length)
	{
		//Hacer push, ya que vendría a ser lo mismo
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[INFO] ptrarr_set: just pushing...\n");
		ptrarr_push(ptrarr, p);
		return 0;
	}

	if(index > ptrarr->length)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_set: index is greater than length, please use ptrarr_push\n");
		return 1;
	}

	ptrarr->ptr_arr[index] = p;
	return 0;
}


int ptrarr_destroy(Ptrarr * ptrarr)
{
	if(ptrarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_destroy: can't destroy anything if you give me a null pointer :c\n");
		return 1;
	}

	free(ptrarr);

	return 0;
}

int ptrarr_destroy_everything(Ptrarr * ptrarr)
{
	fprintf(stderr, "[INFO] ptrarr_destroy_everything: Destroying everything...\n");

	if(ptrarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_destroy_everything: can't destroy anything if you give me a null pointer :c\n");
		return 1;
	}

	void * ptr = NULL;

	for(int i = 0; i < ptrarr->length; i++)
	{
		ptr = ptrarr_get(ptrarr, i);
		if(ptr == NULL)
		{
			fprintf(stderr, "[WARN] ptrarr_destroy_everything: pointer in ptrarr at index %i is NULL.\n", i);
			continue;
		} 
		free(ptr);
	}

	return ptrarr_destroy(ptrarr);
}

void ptrarr_print_pointers(Ptrarr * ptrarr) //Imprimir punteros.
{
	if(ptrarr == NULL)
	{
		if(PRINT_DEBUG >= 1) fprintf(stderr, "[ERROR] ptrarr_print_pointers: ptrarr is NULL\n");
		return;
	}

	printf("[PRINTING POINTERS OF PTRARR]\n");

	for(int i = 0; i < ptrarr->length; i++)
	{
		printf(">%i = %p\n", i, ptrarr_get(ptrarr, i));
	}
}
