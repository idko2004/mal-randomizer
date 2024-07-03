#! /bin/bash
BUILD="build"
MATERIALS="${BUILD}/materials"

YELLOW="\033[1;33m"
GREEN="\033[1;32m"
RED="\033[1;31m"
NOCOLOR="\033[0m"

mkdir -p $MATERIALS

echo -e "${YELLOW}Compiling curl_wrapper${NOCOLOR}"
gcc -c curl_wrapper.c -o "${MATERIALS}/curl_wrapper.o"

echo -e "${YELLOW}Compiling text_parser${NOCOLOR}"
gcc -c text_parser.c -o "${MATERIALS}/text_parser.o"

echo -e "${YELLOW}Compiling cJSON${NOCOLOR}"
gcc -c cJSON/cJSON.c -o "${MATERIALS}/cJSON.o"

echo -e "${YELLOW}Compiling main${NOCOLOR}"
gcc -lcurl "${MATERIALS}/cJSON.o" "${MATERIALS}/curl_wrapper.o" "${MATERIALS}/text_parser.o" main.c -o "${BUILD}/mal-randomizer"

if [ $? -eq 0 ]; then
	echo -e "${GREEN}Done!${NOCOLOR}"
	echo "The program should be in ${BUILD}/mal-randomizer"
else
	echo -e "${RED}Failed to compile :c${NOCOLOR}"
fi
