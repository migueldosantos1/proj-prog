#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_WORD 100 /*tamanho máximo de uma palavra; ajustável*/

/*função que compara dois argumentos sem ter em conta maiúsculas*/
int compare(const void *arg1, const void *arg2){
    return strcasecmp(*(const char **)arg1, *(const char **)arg2);
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

    char *filename = NULL;

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-h") == 0){
            print_help();
        }
        if(strcmp(argv[i], "-d") == 0){
            if((i + 1) < argc){ /*o argumento "-d" exige um argumento com o nome do dicionário posteriormente*/
                filename = argv[i + 1];
            }
            /*
            else{
                printf("Erro: -d requer um nome de ficheiro.\n");
                return 1;
            }
            */
        }
    }

    if(filename == NULL){
        printf("Nenhum dicionário introduzido.\n");
        return 1;
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
    /*
    for(int i = 0; i < 10; i++){
        printf("%s\n", dictionary[i]);
    }
    */

    qsort(dictionary, counter, sizeof(char *), compare);

    /*
    for(int i = 0; i < 100; i++){
        printf("%s\n", dictionary[i]);
    }
    */

    

    fclose(file);

    /*free da memória alocada*/
    for(int i = 0; i < counter; i++){
        free(dictionary[i]);
    }
    free(dictionary);

    return 0;
}