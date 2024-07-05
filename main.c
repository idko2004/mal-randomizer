#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <gtk/gtk.h>
#include <time.h>

#include "cJSON/cJSON.h"

#include "ui/gtk_builder_ui.h"
#include "curl_wrapper.h"
#include "text_parser.h"
#include "process_anime.h"
#include "strarr.h"

typedef struct
{
	const char * username; //in
	int status; //in
	GtkBuilder * builder; //in
	pthread_t thread_id; //in
	AnimeArrays * anime_arrays; //out
} DataToParseMal;

int change_page_and_show_result(DataToParseMal * data_to_parse_mal)
{
	GObject * stack = gtk_builder_get_object(GTK_BUILDER(data_to_parse_mal->builder), "stack");
	GObject * page3 = gtk_builder_get_object(GTK_BUILDER(data_to_parse_mal->builder), "page3");
	gtk_stack_set_visible_child(GTK_STACK(stack), GTK_WIDGET(page3));
	GObject * spinner = gtk_builder_get_object(GTK_BUILDER(data_to_parse_mal->builder), "loadingPageSpinner");
	gtk_spinner_stop(GTK_SPINNER(spinner));
}

void * download_and_parse_mal(void * data_to_parse_mal_ptr)
{
	DataToParseMal * data_to_parse_mal = (DataToParseMal *) data_to_parse_mal_ptr;
	const char * username = data_to_parse_mal->username;
	int status = data_to_parse_mal->status;

	if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
	{
		fprintf(stderr, "[ERROR] main: failed to initialize curl globally.\n");
		return NULL;
	}

	char * url_base = "https://myanimelist.net/animelist/%s?status=%i";

	char * url = malloc(sizeof(char) * strlen(url_base) + strlen(username));
	sprintf(url, url_base, username, status);

	fprintf(stderr, "[INFO] url = %s", url);

	CurlResponse * mal_page = curlw_get_as_text(url);

	if(mal_page == NULL)
	{
		fprintf(stderr, "[ERROR] mal_page is NULL, maybe the download failed.\n");
		return NULL;
	}

	//printf("%s", mal_page->content);

	long int mal_data_items_start = find_in_text("data-items=\"", mal_page->content, 0);
	long int mal_data_items_end = find_in_text("\" data-broadcasts", mal_page->content, mal_data_items_start);
	if(mal_data_items_start == -1 || mal_data_items_end == -1)
	{
		fprintf(stderr, "[ERROR] main: Failed to find start or end of data items.\n");
		return NULL;
	}

	char * mal_data_items = slice_text(mal_data_items_start + strlen("data-items=\""), mal_data_items_end, mal_page->content);
	if(mal_data_items == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to slice data items.\n");
		return NULL;
	}

	mal_data_items = replace_all("&quot;", "\"", mal_data_items);
	if(mal_data_items == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to replace &quot;\n");
		return NULL;
	}

	fprintf(stderr, "[INFO] starting to parse json.\n");
	cJSON * mal_json = cJSON_Parse(mal_data_items);
	if(mal_json == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to parse json.\n");
		return NULL;
	}
	fprintf(stderr, "[INFO] json parsed.\n");

	AnimeArrays * anime_arrays = process_anime(mal_json);

	data_to_parse_mal->anime_arrays = anime_arrays;

	change_page_and_show_result(data_to_parse_mal);
/*
	strarr_destroy_everything(anime_arrays->arr_anime_names);
	strarr_destroy_everything(anime_arrays->arr_anime_names_eng);
	strarr_destroy_everything(anime_arrays->arr_anime_images_paths);
	strarr_destroy_everything(anime_arrays->arr_anime_urls);
	free(anime_arrays);
	free(mal_data_items);
	cJSON_Delete(mal_json);
	free_CurlResponse(mal_page);
	curl_global_cleanup();
*/
}

void click_go_button(GtkWidget * widget, void * callback_arg)
{
	fprintf(stderr, "[INFO] Go button was clicked.\n");

	GObject * stack = gtk_builder_get_object(GTK_BUILDER(callback_arg), "stack");
	GObject * page2 = gtk_builder_get_object(GTK_BUILDER(callback_arg), "page2");
	GObject * spinner = gtk_builder_get_object(GTK_BUILDER(callback_arg), "loadingPageSpinner");
	gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
	gtk_spinner_start(GTK_SPINNER(spinner));
	gtk_stack_set_visible_child(GTK_STACK(stack), GTK_WIDGET(page2));
	

	GObject * username_entry = gtk_builder_get_object(GTK_BUILDER(callback_arg), "username-entry");
	if(username_entry == NULL)
	{
		fprintf(stderr, "[ERROR] clickGoButton: Failed to get username entry from builder.\n");
		return;
	}

	const char * username = gtk_entry_get_text(GTK_ENTRY(username_entry));
	if(username == NULL)
	{
		fprintf(stderr, "[ERROR] clickGoButton: Failed to get the contents of username entry.\n");
		return;
	}
	
	GObject * list_combobox = gtk_builder_get_object(GTK_BUILDER(callback_arg), "anime_list_type");
	if(list_combobox == NULL)
	{
		fprintf(stderr, "[ERROR] clickGoButton: Failed to get list entry from builder.\n");
		return;
	}
	int list_selected = gtk_combo_box_get_active(GTK_COMBO_BOX(list_combobox));
	if(list_selected == -1)
	{
		fprintf(stderr, "[ERROR] clickGoButton: Invalid combobox active.\n");
		return;
	}

	if(list_selected == 5) list_selected = 6; //Por algún motivo, en myanimelist el status 5 está vacío.
	fprintf(stderr, "[INFO] username: %s\n[INFO] list: %i\n", username, list_selected);

	pthread_t thread_id;

	DataToParseMal * data_to_parse_mal = malloc(sizeof(DataToParseMal));
	data_to_parse_mal->builder = (GtkBuilder *) callback_arg;
	data_to_parse_mal->status = list_selected;
	data_to_parse_mal->username = username;
	data_to_parse_mal->anime_arrays = NULL;

	pthread_create(&thread_id, NULL, download_and_parse_mal, (void *)data_to_parse_mal);
	
	//startDoingTheStuff(username, list_selected, GTK_BUILDER(callback_arg));
}

int main(int argc, char ** argv)
{
	GtkBuilder * builder;
	GError * error = NULL;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();
	if(gtk_builder_add_from_string(builder, gtk_main_ui_str, strlen(gtk_main_ui_str), &error) == 0)
	{
		fprintf(stderr, "[ERROR] Failed to create gtk builder: %s.\n", error->message);
		return 1;
	}

	GObject * window = gtk_builder_get_object(builder, "window");
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	GObject * button = gtk_builder_get_object(builder, "goButton");
	g_signal_connect(button, "clicked", G_CALLBACK(click_go_button), builder);

	gtk_main();

	return 0;
}
