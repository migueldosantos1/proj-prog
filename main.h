#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define MAX_WORD 100
#define MAX_LINE 100

bool output(int argc, char *argv[]);

int compare(const void *arg1, const void *arg2);

int binary_search(char *word, char **dictionary, int size);

void remove_newline(char *line);

void clean_word(char *word);

void print_help();

void suggestions(int counter, int alt, char *word, char *token, char **dictionary);

void mode1(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[]);

void mode2(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, char *word);

#endif