#ifndef STRARR_H  /* Include guard */
#define STRARR_H

typedef struct
{
	int spaces_reserved; //La cantidad de punteros que puede guardar str_arr, se cuenta de la misma forma que length
	int length; //La cantidad de punteros que str_arr tiene actualmente
	int * arr_spaces_reserved; //La array donde se guarda la memoria reservada para cada string en str_arr
	char ** str_arr; //La array donde se guardan los punteros hacia los strings
} Strarr;

Strarr * strarr_new(int capacity); //Inicializa Strarr, debe ejecutarse justo después de crear Strarr, capacity debe indicar la cantidad de espacios a reservar, se cuenta de la misma forma que length

int strarr_expand_array(Strarr * strarr, int new_capacity); //Expande el array (str_arr) a la capacidad indicada, la capacidad indicada será el nuevo spaces_reserved

int strarr_expand_string(Strarr * strarr, int index, int new_capacity); //Expande un string ya establecido en el index indicado, con la capacidad indicada, la capacidad indicada ya debe incluir el caracter NULL

int strarr_push(Strarr * strarr, char * str); //Añade un string al final del array

char * strarr_get(Strarr * strarr, int index); //Obtiene el string guardado en el index especificado del array

int strarr_set(Strarr * strarr, int index, char * text); //Reemplaza un string que ha sido guardado en el array anteriormente en el index indicado

int strarr_destroy_string(Strarr * strarr, int index); //Destruye el string en el index especificado y reacomoda el array

int strarr_destroy_everything(Strarr * strarr); //Libera la memoria del struct y de todos los strings que contenía

void strarr_print_all(Strarr * strarr); //Imprime en stderr todo el contenido del array

#endif
