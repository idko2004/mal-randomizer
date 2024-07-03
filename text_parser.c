#include "text_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long int find_in_text(char * s, char * input, long int start_at)
{
	fprintf(stderr, "[INFO] find_in_text: Trying to find %s\n", s);

	long int s_length = strlen(s);
	long int input_length = strlen(input);

	long int matches = 0; //Lleva la cuenta de cuántos caracteres son iguales en s y en input, si matches y s_length - 1 llegan a ser iguales retornamos i

	for(long int i = start_at; i < input_length; i++)
	{
		for(long int j = 0; j < s_length; j++)
		{
			if(input[i + j] == s[j])
			{
				matches++;
				if(matches == s_length - 1)
				{
					fprintf(stderr, "[INFO] find_in_text: Found something at index %li\n", i);
					return i;
				}
			}
		}
		matches = 0;
	}

	fprintf(stderr, "[INFO] find_in_text: Found nothing\n");
	return -1;
}

char * slice_text(long int from, long int to, char * input)
{
	char * result = malloc(sizeof(char) * (to - from) + 1);

	if(result == NULL)
	{
		fprintf(stderr, "[ERROR] slice_text: Couldn't make space for the result.\n");
		return NULL;
	}

	strncpy(result, input + from, to - from);
	result[to - from] = '\0';

	return result;

}
