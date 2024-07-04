#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <gtk/gtk.h>

#include "cJSON/cJSON.h"

#include "ui/gtk_builder_ui.h"
#include "curl_wrapper.h"
#include "text_parser.h"
#include "process_anime.h"
#include "strarr.h"

void clickGoButton(GtkWidget * widget, void * callback_arg)
{
	fprintf(stderr, "[INFO] Go button was clicked.\n");

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
	if(username_entry == NULL)
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
	fprintf(stderr, "%s\n%i\n", username, list_selected);
}

int startDoingTheStuff(char * username, int status)
{
	if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
	{
		fprintf(stderr, "[ERROR] main: failed to initialize curl globally.\n");
		return 1;
	}

	CurlResponse * mal_page = curlw_get_as_text("https://myanimelist.net/animelist/idko2004?status=6");

	if(mal_page == NULL)
	{
		fprintf(stderr, "[ERROR] mal_page is NULL, maybe the download failed.\n");
		return 1;
	}

	//printf("%s", mal_page->content);

	long int mal_data_items_start = find_in_text("data-items=\"", mal_page->content, 0);
	long int mal_data_items_end = find_in_text("\" data-broadcasts", mal_page->content, mal_data_items_start);
	if(mal_data_items_start == -1 || mal_data_items_end == -1)
	{
		fprintf(stderr, "[ERROR] main: Failed to find start or end of data items.\n");
		return 1;
	}

	char * mal_data_items = slice_text(mal_data_items_start + strlen("data-items=\""), mal_data_items_end, mal_page->content);
	if(mal_data_items == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to slice data items.\n");
		return 1;
	}

	mal_data_items = replace_all("&quot;", "\"", mal_data_items);
	if(mal_data_items == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to replace &quot;\n");
		return 1;
	}

	fprintf(stderr, "[INFO] starting to parse json.\n");
	cJSON * mal_json = cJSON_Parse(mal_data_items);
	if(mal_json == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to parse json.\n");
		return 1;
	}
	fprintf(stderr, "[INFO] json parsed.\n");

	AnimeArrays * anime_arrays = process_anime(mal_json);

	strarr_destroy_everything(anime_arrays->arr_anime_names);
	strarr_destroy_everything(anime_arrays->arr_anime_names_eng);
	strarr_destroy_everything(anime_arrays->arr_anime_images_paths);
	strarr_destroy_everything(anime_arrays->arr_anime_urls);
	free(anime_arrays);
	free(mal_data_items);
	cJSON_Delete(mal_json);
	free_CurlResponse(mal_page);
	curl_global_cleanup();

	return 0;
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
	g_signal_connect(button, "clicked", G_CALLBACK(clickGoButton), builder);

	gtk_main();

	return 0;
}
