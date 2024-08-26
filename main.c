#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <gtk/gtk.h>
#include <time.h>
#include <locale.h>

#include "cJSON/cJSON.h"

#include "ptrarr.h"
#include "ui/gtk_builder_ui.h"
#include "curl_wrapper.h"
#include "text_parser.h"
#include "process_anime.h"
#include "random.h"
#include "image.h"

#define MARKUP_FORMAT_JP "<span font_weight=\"bold\" font_size=\"16000\">%s</span>"
#define MARKUP_FORMAT_EN "<span font_size=\"12000\">%s</span>"

typedef struct
{
	const char * username; //in
	int status; //in
	GtkBuilder * builder; //in
	pthread_t thread_id; //in
	AnimeArrays * anime_arrays; //out
} DataToParseMal;

DataToParseMal * global_data_to_parse_mal = NULL;
int index_anime = -1;

void clean()
{
	fprintf(stderr, "[INFO] Cleaning up.\n");

	if(global_data_to_parse_mal != NULL)
	{
		if(global_data_to_parse_mal->anime_arrays != NULL)
		{
			if(global_data_to_parse_mal->anime_arrays->arr_anime_names != NULL) ptrarr_destroy(global_data_to_parse_mal->anime_arrays->arr_anime_names);

			if(global_data_to_parse_mal->anime_arrays->arr_anime_names_eng != NULL) ptrarr_destroy(global_data_to_parse_mal->anime_arrays->arr_anime_names_eng);

			if(global_data_to_parse_mal->anime_arrays->arr_anime_images_paths != NULL) ptrarr_destroy(global_data_to_parse_mal->anime_arrays->arr_anime_images_paths);

			if(global_data_to_parse_mal->anime_arrays->arr_anime_urls != NULL) ptrarr_destroy(global_data_to_parse_mal->anime_arrays->arr_anime_urls);

			free(global_data_to_parse_mal->anime_arrays);
		}
	}

	curl_global_cleanup();
}

void show_error_page(GtkBuilder * builder, char * error)
{
	GObject * stack = gtk_builder_get_object(builder, "stack");
	GObject * page_error = gtk_builder_get_object(builder, "pageError");
	GObject * label_error = gtk_builder_get_object(builder, "errorLabel");
	
	gtk_label_set_text(GTK_LABEL(label_error), error);
	
	gtk_stack_set_visible_child(GTK_STACK(stack), GTK_WIDGET(page_error));
}

int open_anime_in_browser()
{
	if(global_data_to_parse_mal == NULL)
	{
		fprintf(stderr, "[ERROR] global_data_to_parse_mal: global_data_to_parse_mal is null.\n");
		return 1;
	}

	if(index_anime == -1)
	{
		fprintf(stderr, "[ERROR] global_data_to_parse_mal: anime index isn't set yet.\n");
		return 1;
	}

	char * url_base = "https://myanimelist.net%s";
	char * anime_url = ptrarr_get(global_data_to_parse_mal->anime_arrays->arr_anime_urls, index_anime);
	if(anime_url == NULL)
	{
		fprintf(stderr, "[ERROR] global_data_to_parse_mal: Failed to get anime_id.\n");
		return 1;
	}

	char * url = malloc(sizeof(char) * (strlen(url_base) + strlen(anime_url) + 1));
	sprintf(url, url_base, anime_url);

	GObject * window = gtk_builder_get_object(global_data_to_parse_mal->builder, "window");

	GError * error = NULL;

	gtk_show_uri_on_window(GTK_WINDOW(window), url, GDK_CURRENT_TIME, &error);

	if(error != NULL)
	{
		fprintf(stderr, "[ERROR] global_data_to_parse_mal: gtk_show_uri_on_window: %s.\n", error->message);
		return 1;
	}

	free(url);

	return 0;
}

int show_random_anime()
{
	DataToParseMal * data_to_parse_mal = global_data_to_parse_mal;

	if(data_to_parse_mal == NULL)
	{
		fprintf(stderr, "[ERROR] show_random_anime: global_data_to_parse_mal is null.\n");
		return 1;
	}

	GObject * label_jp = gtk_builder_get_object(GTK_BUILDER(data_to_parse_mal->builder), "anime_name_jp");
	GObject * label_en = gtk_builder_get_object(GTK_BUILDER(data_to_parse_mal->builder), "anime_name_en");

	int i = get_index_of_random_anime(data_to_parse_mal->anime_arrays);

	fprintf(stderr, "[INFO] show_random_anime: %i\n", i);

	char * jp_name_bad = ptrarr_get(data_to_parse_mal->anime_arrays->arr_anime_names, i);
	char * en_name_bad = ptrarr_get(data_to_parse_mal->anime_arrays->arr_anime_names_eng, i);

	if(jp_name_bad == NULL || strcmp(jp_name_bad, "\0") == 0)
	{
		fprintf(stderr, "[ERROR] show_random_anime: Failed to get japanese anime name of index %i\n", i);
		jp_name_bad = "Failed to get an anime :c";
	}
	if(en_name_bad == NULL || strcmp(en_name_bad, "\0") == 0)
	{
		fprintf(stderr, "[ERROR] show_random_anime: Failed to get english anime name of index %i\n", i);
		en_name_bad = "Failed to get english name :c";
	}

	fprintf(stderr, "[INFO] anime name jp: %s (%p)\n", jp_name_bad, jp_name_bad);
	fprintf(stderr, "[INFO] anime name en: %s (%p)\n", en_name_bad, en_name_bad);

	char * jp_name = replace_all("&#039;", "'", jp_name_bad);
	char * en_name = replace_all("&#039;", "'", en_name_bad);

	fprintf(stderr, "[INFO] anime name jp fixed: %s\n", jp_name);
	fprintf(stderr, "[INFO] anime name en fixed: %s\n", en_name);

	char * markup_jp = g_markup_printf_escaped(MARKUP_FORMAT_JP, jp_name);
	gtk_label_set_markup(GTK_LABEL(label_jp), markup_jp);

	char * markup_en = g_markup_printf_escaped(MARKUP_FORMAT_EN, en_name);
	gtk_label_set_markup(GTK_LABEL(label_en), markup_en);

	char * image_url = ptrarr_get(data_to_parse_mal->anime_arrays->arr_anime_images_paths, i);
	if(image_url != NULL || strcmp(image_url, "\0") != 0)
	{
		download_and_show_image(image_url, data_to_parse_mal->builder);
	}
	else fprintf(stderr, "[WARN] Failed to get image url.\n");

	index_anime = i;

	g_free(markup_jp);
	g_free(markup_en);
	free(jp_name);
	free(en_name);

	return 0;
}

void change_page_and_show_result(DataToParseMal * data_to_parse_mal)
{
	global_data_to_parse_mal = data_to_parse_mal;

	show_random_anime();

	GObject * stack = gtk_builder_get_object(GTK_BUILDER(data_to_parse_mal->builder), "stack");
	GObject * page3 = gtk_builder_get_object(GTK_BUILDER(data_to_parse_mal->builder), "page3");
	gtk_stack_set_visible_child(GTK_STACK(stack), GTK_WIDGET(page3));
	GObject * spinner = gtk_builder_get_object(GTK_BUILDER(data_to_parse_mal->builder), "loadingPageSpinner");
	gtk_spinner_stop(GTK_SPINNER(spinner));
}

int show_result_page_spinner(GtkBuilder * builder, int show)
{
	GObject * spinner = gtk_builder_get_object(builder, "resultPageSpinner");
	GObject * resultPage = gtk_builder_get_object(builder, "resultScreenShow");

	if(show == 0) //0 = show
	{
		gtk_widget_set_visible(GTK_WIDGET(resultPage), FALSE);
		gtk_widget_set_visible(GTK_WIDGET(spinner), TRUE);
		gtk_spinner_start(GTK_SPINNER(spinner));
	}
	else if(show == -1) // 0 = hide
	{
		gtk_spinner_stop(GTK_SPINNER(spinner));
		gtk_widget_set_visible(GTK_WIDGET(spinner), FALSE);
		gtk_widget_set_visible(GTK_WIDGET(resultPage), TRUE);
	}
	else
	{
		fprintf(stderr, "[ERROR] show_result_page_spinner: invalid show value.\n");
		return 1;
	}

	return 0;
}

void * download_and_parse_mal(void * data_to_parse_mal_ptr)
{
	DataToParseMal * data_to_parse_mal = (DataToParseMal *) data_to_parse_mal_ptr;
	const char * username = data_to_parse_mal->username;
	int status = data_to_parse_mal->status;

	if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
	{
		fprintf(stderr, "[ERROR] main: failed to initialize curl globally.\n");
		show_error_page(GTK_BUILDER(data_to_parse_mal->builder), "No se pudo iniciar libcurl");
		return NULL;
	}

	char * url_base = "https://myanimelist.net/animelist/%s?status=%i";

	char * url = malloc(sizeof(char) * (strlen(url_base) + strlen(username) + 1));
	sprintf(url, url_base, username, status);

	fprintf(stderr, "[INFO] url = %s\n", url);

	CurlResponse * mal_page = curlw_get_as_text(url);

	if(mal_page == NULL)
	{
		fprintf(stderr, "[ERROR] mal_page is NULL, maybe the download failed.\n");
		show_error_page(GTK_BUILDER(data_to_parse_mal->builder), "¿La descarga falló?");
		return NULL;
	}

	//printf("%s", mal_page->content);

	long int mal_data_items_start = find_in_text("data-items=\"", mal_page->content, 0);
	long int mal_data_items_end = find_in_text("\" data-broadcasts", mal_page->content, mal_data_items_start);
	if(mal_data_items_start == -1 || mal_data_items_end == -1)
	{
		fprintf(stderr, "[ERROR] main: Failed to find start or end of data items.\n");
		show_error_page(GTK_BUILDER(data_to_parse_mal->builder), "No se pudo procesar la lista de animes.\nEs posible que el nombre de usuario no sea correcto.");
		return NULL;
	}

	char * mal_data_items = slice_text(mal_data_items_start + strlen("data-items=\""), mal_data_items_end, mal_page->content);
	if(mal_data_items == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to slice data items.\n");
		show_error_page(GTK_BUILDER(data_to_parse_mal->builder), "Failed to slice data items.");
		return NULL;
	}

	char * mal_data_items_parsed = replace_all("&quot;", "\"", mal_data_items);
	if(mal_data_items_parsed == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to replace &quot;\n");
		show_error_page(GTK_BUILDER(data_to_parse_mal->builder), "Failed to replace &quot;.");
		return NULL;
	}
	
	long int mal_data_items_parsed_length = strlen(mal_data_items_parsed) + 1;
	fprintf(stderr, "[INFO] length of mal_data_items_parsed is %li\n", mal_data_items_parsed_length);

	fprintf(stderr, "[INFO] starting to parse json.\n");
	setlocale( LC_NUMERIC, "C");
	cJSON * mal_json = cJSON_ParseWithLength(mal_data_items_parsed, mal_data_items_parsed_length);
	if(mal_json == NULL)
	{
		fprintf(stderr, "[ERROR] main: Failed to parse json.\n");
		show_error_page(GTK_BUILDER(data_to_parse_mal->builder), "No se pudo procesar json.");
		fprintf(stderr, "[ERROR] cJSON_GetErrorPtr: %s\n", cJSON_GetErrorPtr());
		return NULL;
	}
	fprintf(stderr, "[INFO] json parsed.\n");

	AnimeArrays * anime_arrays = process_anime(mal_json);

	data_to_parse_mal->anime_arrays = anime_arrays;

	change_page_and_show_result(data_to_parse_mal);

	free(url);
	free(mal_data_items);
	free(mal_data_items_parsed);
	cJSON_Delete(mal_json);
	free_CurlResponse(mal_page);

	return NULL;
}

void * rerollAnime(void * builder_ptr)
{
	show_random_anime();
	show_result_page_spinner(GTK_BUILDER(builder_ptr), -1);

	return NULL;
}

int is_username_valid(const char * username)
{
	//Está vacío
	if(strcmp(username, "") == 0) return -1;

	//Tiene un espacio
	if(find_in_text(" ", username, 0) != -1) return -1;

	return 0;
}

void click_go_button(GtkWidget * widget, void * callback_arg)
{
	gtk_widget_set_sensitive(widget, FALSE); //Desactivar el botón para que no se pueda activar otra vez, ya que, por ejemplo, si se pone el foco sobre el botón con el teclado y se spamea la tecla enter, en lo que dura la animación es posible activar el botón varias veces.

	fprintf(stderr, "[INFO] Go button was clicked.\n");

	GObject * stack = gtk_builder_get_object(GTK_BUILDER(callback_arg), "stack");
	GObject * page2 = gtk_builder_get_object(GTK_BUILDER(callback_arg), "page2");
	GObject * spinner = gtk_builder_get_object(GTK_BUILDER(callback_arg), "loadingPageSpinner");
	gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

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

	//Si el nombre de usuario está vacío, no hacer nada
	if(is_username_valid(username) != 0)
	{
		fprintf(stderr, "[INFO] username is not valid.\n");
		gtk_widget_grab_focus(GTK_WIDGET(username_entry));
		gtk_widget_set_sensitive(widget, TRUE);
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

	//Hacer la transición
	gtk_spinner_start(GTK_SPINNER(spinner));
	gtk_stack_set_visible_child(GTK_STACK(stack), GTK_WIDGET(page2));

	//Crear la estructura en la que se van a guardar los datos
	DataToParseMal * data_to_parse_mal = malloc(sizeof(DataToParseMal));
	data_to_parse_mal->builder = (GtkBuilder *) callback_arg;
	data_to_parse_mal->status = list_selected;
	data_to_parse_mal->username = username;
	data_to_parse_mal->anime_arrays = NULL;

	//Descargar y procesar los datos en otro hilo
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, download_and_parse_mal, (void *)data_to_parse_mal);
	pthread_detach(thread_id);
}

void click_reroll_button(GtkWidget * widget, void * callback_arg)
{
	fprintf(stderr, "[INFO] NEW ROLL\n");
	if(global_data_to_parse_mal == NULL)
	{
		fprintf(stderr, "[ERROR] click_reroll_button: global_data_to_parse_mal was not defined so can't proceed.\n");
		return;
	}

	show_result_page_spinner(global_data_to_parse_mal->builder, 0);

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, rerollAnime, callback_arg);
	pthread_detach(thread_id);
}

int main(int argc, char ** argv)
{
	fprintf(stderr, "hi :3\n");

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

	GObject * reroll_button = gtk_builder_get_object(GTK_BUILDER(builder), "rerollButton");
	g_signal_connect(reroll_button, "clicked", G_CALLBACK(click_reroll_button), builder);

	GObject * browser_button = gtk_builder_get_object(GTK_BUILDER(builder), "linkButton");
	g_signal_connect(browser_button, "clicked", G_CALLBACK(open_anime_in_browser), NULL);

	GObject * error_button = gtk_builder_get_object(GTK_BUILDER(builder), "errorButton");
	g_signal_connect(error_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

	gtk_main();

	clean();

	return 0;
}
