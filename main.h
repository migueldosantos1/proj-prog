#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_WORD 100
#define MAX_LINE 100

void mode1(FILE *input_file, FILE *output_file, char **dictionary, int counter, char *argv[]);

int compare(const void *arg1, const void *arg2);

int binary_search(char *word, char **dictionary, int size);

void remove_newline(char *line);

void clean_word(char *word);

void print_help();

#endif