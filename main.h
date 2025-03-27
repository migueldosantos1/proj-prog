#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#define MAX_WORD 100
#define MAX_LINE 100

typedef struct{
    char *word;
    int differences;
} suggestion_data;

bool output(int argc, char *argv[]);

int compare_suggestions(const void *a, const void *b);

int compare(const void *arg1, const void *arg2);

int binary_search(char *word, char **dictionary, int size);

void remove_newline(char *line);

void clean_word(char *word);

void print_help();

void split(char *word, char **dictionary, int counter, suggestion_data *list, int *found, int maxdiffs);

char* calculate_differences(char *token, char *word, int kanye);

//int calculate_differences_reverse(char *token, char *word);

void suggestions(int counter, int alt, char *token, char **dictionary, int maxdiffs, int argc, char *argv[], FILE *output_file);

void mode1(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[]);

void mode2(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, char *word, int diffs);

//void mode3(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, int diffs);

#endif