#include "text_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long int find_in_text(char * s, char * input, long int start_at)
{
	//fprintf(stderr, "[INFO] find_in_text: Trying to find %s\n", s);

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
					//fprintf(stderr, "[INFO] find_in_text: Found something at index %li\n", i);
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

char * replace_text(char * target, char * replace_with, char * text)
{
	long int target_start = find_in_text(target, text, 0);
	if(target_start == -1)
	{
		fprintf(stderr, "[INFO] replace_text: No %s left in text.\n", target);
		return NULL;
	}
	long int target_end = target_start + strlen(target);

	char * new_text = malloc(sizeof(char) * strlen(text));
	if(new_text == NULL)
	{
		fprintf(stderr, "[ERROR] replace_text: Couldn't allocate memory for new text.\n");
		return NULL;
	}

	char * before_text = slice_text(0, target_start, text);
	char * after_text = slice_text(target_end, strlen(text), text);
	if(before_text == NULL || after_text == NULL)
	{
		fprintf(stderr, "[ERROR] replace_text: Couldn't allocate memory for slicing text.\n");
		return NULL;
	}

	strcpy(new_text, before_text);
	int length = target_start;
	strcpy(new_text + length, replace_with);
	length += strlen(replace_with);
	strcpy(new_text + length, after_text);
	length += strlen(after_text);
	new_text[length] = '\0';

	free(before_text);
	free(after_text);

	return new_text;
}

char * replace_all(char * target, char * replace_with, char * text)
{
	fprintf(stderr, "[INFO] replace_all: starting to replace %s\n", target);
	char * last_iteration = text;
	int i = 0;
	while(1)
	{
		char * result = replace_text(target, replace_with, last_iteration);
		if(result == NULL)
		{
			fprintf(stderr, "[INFO] replace_all: seems like there is no more %s, at least i hope so.\n[INFO] replace_all: replaced %i times.\n", target, i);
			return last_iteration;
		}
		free(last_iteration);
		last_iteration = result;
		i++;
	}
}
