#! /bin/bash
BUILD="build"
MATERIALS="${BUILD}/materials"

YELLOW="\033[1;33m"
GREEN="\033[1;32m"
RED="\033[1;31m"
NOCOLOR="\033[0m"

mkdir -p $MATERIALS

if [[ "${SKIP_NODE}" != "1" ]]; then
	echo -e "${YELLOW}Generating GTK UI${NOCOLOR}"
	#This generates the file ui/gtk_builder_ui.h from gtkbuilder.ui, basically embeds the xml ui file into the program.
	node ui/generate_ui_in_header.js
fi

echo -e "${YELLOW}Compiling cJSON${NOCOLOR}"
gcc -c cJSON/cJSON.c -o "${MATERIALS}/cJSON.o"

echo -e "${YELLOW}Compiling ptrarr${NOCOLOR}"
gcc -c -Wall -Werror -ggdb ptrarr.c -o "${MATERIALS}/ptrarr.o"

echo -e "${YELLOW}Compiling curl_wrapper${NOCOLOR}"
gcc -c -Wall -Werror -ggdb curl_wrapper.c -o "${MATERIALS}/curl_wrapper.o"

echo -e "${YELLOW}Compiling text_parser${NOCOLOR}"
gcc -c -Wall -Werror -ggdb text_parser.c -o "${MATERIALS}/text_parser.o"

echo -e "${YELLOW}Compiling process_anime${NOCOLOR}"
gcc -c -Wall -Werror -ggdb process_anime.c -o "${MATERIALS}/process_anime.o"

echo -e "${YELLOW}Compiling seed${NOCOLOR}"
gcc -c -Wall -Werror -ggdb seed.c -o "${MATERIALS}/seed.o"

echo -e "${YELLOW}Compiling random${NOCOLOR}"
gcc -c -Wall -Werror -ggdb random.c -o "${MATERIALS}/random.o"

echo -e "${YELLOW}Compiling image${NOCOLOR}"
gcc -c -Wall -Werror -ggdb image.c -o "${MATERIALS}/image.o" `pkg-config --cflags --libs gtk+-3.0`

echo -e "${YELLOW}Compiling main${NOCOLOR}"
gcc -Wall -Werror -ggdb "${MATERIALS}/cJSON.o" "${MATERIALS}/ptrarr.o" "${MATERIALS}/curl_wrapper.o" "${MATERIALS}/text_parser.o" "${MATERIALS}/process_anime.o" "${MATERIALS}/seed.o" "${MATERIALS}/random.o" "${MATERIALS}/image.o" main.c -o "${BUILD}/mal-randomizer" `pkg-config --cflags --libs gtk+-3.0` `curl-config --cflags --libs`

if [ $? -eq 0 ]; then
	echo -e "${GREEN}Done!${NOCOLOR}"
	echo "The program should be in ${BUILD}/mal-randomizer"
else
	echo -e "${RED}Failed to compile :c${NOCOLOR}"
fi
