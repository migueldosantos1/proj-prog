#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_WORD 100 /*tamanho máximo de uma palavra; ajustável*/

/*função que compara dois argumentos sem ter em conta maiúsculas*/
int compare(const void *arg1, const void *arg2){
    return strcasecmp(*(const char **)arg1, *(const char **)arg2);
}

int binary_search(char *word, char **dictionary, int size){
    char *key = word;
    return(bsearch(&key, dictionary, size, sizeof(char *), compare) != NULL);
}

void print_help(){
    printf("-h              mostra a ajuda para o utilizador e termina\n");
    printf("-i filename     nome do ficheiro de entrada, em alternativa a stdin\n");
    printf("-o filename     nome do ficheiro de saída, em alternativa a stdout\n");
    printf("-d filename     nome do ficheiro de dicionário a usar\n");
    printf("-a nn           o número máximo de alternativas a mostrar com cada erro ortográfico deve ser nn\n");
    printf("-n nn           o número máximo de diferenças a considerar nos erros ortográficos deve ser nn\n");
    printf("-m nn           o modo de funcionamento do programa deve ser nn\n");
}

int main(int argc, char *argv[]){

    char *dictionary_filename = NULL;
    char *input_filename = NULL;
    char *output_filename = NULL;
    FILE *input_file = stdin; /*por padrão, toma o nosso input como texto para avaliar*/
    FILE *output_file = stdout; /*stdout, caso padrão*/

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-h") == 0){
            print_help();
        }
        if(strcmp(argv[i], "-d") == 0){
            /*o argumento "-d" exige um argumento com o nome do dicionário posteriormente*/
            dictionary_filename = argv[i + 1];
        }
        if(strcmp(argv[i], "-i") == 0){
            input_filename = argv[i + 1];
        }
        if(strcmp(argv[i], "-o") == 0){
            output_filename = argv[i + 1];
        }
    }

    if(dictionary_filename == NULL){
        printf("Nenhum dicionário introduzido.\n");
        return 1;
    }

    /*caso o input file seja atribuído, abre o ficheiro fornecido*/
    if(input_filename != NULL){
        input_file = fopen(input_filename, "r");
        if(input_file == NULL){
            printf("Erro ao abrir o ficheiro de input.\n");
            return 1;
        }
    }

    /*caso o output file seja atribuído, abre o ficheiro fornecido*/
    if(output_filename != NULL){
        output_file = fopen(output_filename, "r");
        if(output_file == NULL){
            printf("Erro ao abrir o ficheiro de input.\n");
            return 1;
        }
    }
    
    FILE *file = fopen(argv[2], "r");
    if(file == NULL){
        printf("Erro ao abrir o dicionário.\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char **dictionary = (char **)malloc(size * sizeof(char *));
    if(dictionary == NULL){
        printf("Erro ao alocar memória para o dicionário.\n");
        fclose(file);
        return 1;
    }

    char word[MAX_WORD]; /*buffer temporário para armazenar as palavras*/
    int counter = 0;

    while(fscanf(file, "%s", word) == 1){  
        /*parar de ler a palavra ao encontrar espaço, tab, mudança de linha, ou o caracter '/'*/
        if(strchr(word, (' ' || '\t' || '\n' || '/' )) != NULL){
            continue;
        }

        /*guarda as palavras no dicionário; strdup aloca memória suficiente para uma cópia da string, e retona um pointer para a mesma*/
        dictionary[counter] = strdup(word);
        if(dictionary[counter] == NULL){
            printf("Erro ao alocar memória para a palavra.\n");
            fclose(file);
            return 1;
        }

        counter++;
    }

    /*uso do quick sort para ordenar o dicionário de input*/
    qsort(dictionary, counter, sizeof(char *), compare);

    while(fscanf(input_file, "%s", word) == 1){
        if(!(binary_search(word, dictionary, counter))){
            printf("Erro na palavra: \"%s\"\n", word);
        }
    }

    /*se "-i" e "-o" não forem emitidos, fecha os ficheiros fornecidos*/
    if(input_file != stdin){
        fclose(input_file);
    }
    if(output_file != stdout){
        fclose(output_file);
    }

    fclose(file);

    /*free da memória alocada*/
    for(int i = 0; i < counter; i++){
        free(dictionary[i]);
    }
    free(dictionary);

    return 0;
}