#ifndef TEXTPARSERH  /* Include guard */
#define TEXTPARSERH

long int find_in_text(char * s, char * input, long int start_at);

char * slice_text(long int from, long int to, char * input);

char * replace_text(char * target, char * replace_with, char * text, long int * start_looking_at);

char * replace_all(char * target, char * replace_with, char * text);

#endif
