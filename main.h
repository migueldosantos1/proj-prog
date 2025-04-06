#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#define MAX_WORD 200
#define MAX_LINE 200

typedef struct{
    char *word;
    int differences;
    int index;  /*posição no dicionário*/
} Suggestion;

bool output(int argc, char *argv[]);

void add_suggestion(Suggestion *suggestions, int *suggestion_count, char *word, int differences, int index);

int already_exists(Suggestion *suggestions, int count, char *word);

int compare(const void *arg1, const void *arg2);

int binary_search(char *word, char **dictionary, int size);

void remove_newline(char *line);

void clean_word(char *word);

void print_help();

void split(char *word, char **dictionary, int counter, Suggestion *suggestions, int *suggestion_count, int offset, int alt);

void find_suggestions(FILE *input_file, FILE *output_file, char* token, char* word, int offset, Suggestion *suggestions, int *suggestion_count, int alt, int index);

void find_suggestions_reversed(FILE *input_file, FILE *output_file, char* token, char* word, int offset, Suggestion *suggestions, int *suggestion_count, int alt, int index);

void mode1(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[]);

void mode2(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, char *word, int diffs);

void mode3(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, int diffs);

#endif