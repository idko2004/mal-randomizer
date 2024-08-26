#include "text_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long int find_in_text(const char * target, const char * input, long int start_at)
{
	//fprintf(stderr, "[INFO] find_in_text: Trying to find %s\n", target);

	long int target_length = strlen(target);
	long int input_length = strlen(input);

	long int matches = 0; //Lleva la cuenta de cuántos caracteres son iguales en s y en input, si matches y s_length - 1 llegan a ser iguales retornamos i

	for(long int i = start_at; i < input_length; i++)
	{
		//fprintf(stderr, "big loop\n");
		for(long int j = 0; j < target_length; j++)
		{
			//fprintf(stderr, "Are %c and %c the same?\n", input[i+j], target[j]);
			if(input[i + j] == target[j])
			{
				matches++;
				//fprintf(stderr, "They are!! %li\n", matches);
				if(matches == target_length)
				{
					//fprintf(stderr, "[INFO] find_in_text: Found something at index %li\n", i);
					return i;
				}
			}
			else break;
		}
		matches = 0;
	}

	fprintf(stderr, "[INFO] find_in_text: Found nothing\n");
	return -1;
}

char * slice_text(long int from, long int to, char * input)
{
	//fprintf(stderr, "[INFO] slicing text, from %li to %li in '%s'\n", from, to, input);

	if(from == to)
	{
		fprintf(stderr, "[ERROR?] slice_text: start and end are the same, returning empty string\n");
		char * result = malloc(sizeof(char));
		result[0] = '\0';
		return result;
	}

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

char * replace_text(char * target, char * replace_with, char * text, long int * start_looking_at)
{
	long int target_start = find_in_text(target, text, *start_looking_at);
	//fprintf(stderr, "[INFO] replace_text: start of target is in %li\n", target_start);
	if(target_start == -1)
	{
		fprintf(stderr, "[INFO] replace_text: No %s left in text.\n", target);
		return NULL;
	}
	long int target_end = target_start + strlen(target);
	//fprintf(stderr, "[INFO] replace_text: end of target should be in %li\n", target_end);

	char * new_text = malloc(sizeof(char) * (strlen(text) + 1));
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
	//fprintf(stderr, "[INFO] replace_text: before text currently is: '%s'\n", before_text);
	//fprintf(stderr, "[INFO] replace_text: after text currently is: '%s'\n", after_text);

	strcpy(new_text, before_text);
	long int length = target_start;
	//fprintf(stderr, "[INFO] replace_text: before text copied, length is %i\n", length);
	strcpy(new_text + length, replace_with);
	length += strlen(replace_with);
	//fprintf(stderr, "[INFO] replace_text: replace with copied, length is %i\n", length);
	strcpy(new_text + length, after_text);
	length += strlen(after_text);
	//fprintf(stderr, "[INFO] replace_text: after text copied, length is %i\n", length);
	new_text[length] = '\0';
	//fprintf(stderr, "[INFO] replace_text: result: '%s'\n", new_text);

	free(before_text);
	free(after_text);

	*start_looking_at = target_start;

	return new_text;
}

char * replace_all(char * target, char * replace_with, char * text)
{
	fprintf(stderr, "[INFO] replace_all: starting to replace %s\n", target);

	char * text_copy = malloc(sizeof(char) * (strlen(text) + 1));
	if(text_copy == NULL)
	{
		fprintf(stderr, "[ERROR] replace_all: Failed to allocate for copy of text.\n");
		return NULL;
	}
	strcpy(text_copy, text);

	char * last_iteration = text_copy;
	long int last_position_found = 0;
	long int i = 0;
	while(1)
	{
		char * result = replace_text(target, replace_with, last_iteration, &last_position_found);
		if(result == NULL)
		{
			fprintf(stderr, "[INFO] replace_all: seems like there is no more %s, at least i hope so.\n[INFO] replace_all: replaced %li times.\n", target, i);
			return last_iteration;
		}

		free(last_iteration);
		last_iteration = result;
		i++;
	}
}
