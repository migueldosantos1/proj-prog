#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_WORD 100 /*tamanho máximo de uma palavra*/

void print_help(){
    printf("-h      mostra a ajuda para o utilizador e termina\n");
    printf("-i      filename nome do ficheiro de entrada, em alternativa a stdin\n");
    printf("-o      filename nome do ficheiro de saída, em alternativa a stdout\n");
    printf("-d      filename nome do ficheiro de dicionário a usar\n");
    printf("-a      nn o número máximo de alternativas a mostrar com cada erro ortográfico deve ser nn\n");
    printf("-n      nn o número máximo de diferenças a considerar nos erros ortográficos deve ser nn\n");
    printf("-m      nn o modo de funcionamento do programa deve ser nn\n");
}

int main(int argc, char *argv[]){

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-h") == 0){
            print_help();
        }
        if(strcmp(argv[i], "-d") == 0){

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

    char **dicionario = (char **)malloc(size * sizeof(char *));
    if(dicionario == NULL){
        printf("Erro ao alocar memória do dicionário.\n");
        fclose(file);
        return 1;
    }

    char word[MAX_WORD];  /*buffer temporário para armazenar as palavras*/
    int counter = 0;

    // Ler palavras do ficheiro respeitando os delimitadores (espaço, tab, '\n', '/')
    while(fscanf(file, "%s", word) == 1){  
        /*parar de ler a palavra ao encontrar espaço, tab, mudança de linha, ou o caracter '/'*/
        if(strchr(word, (' ' || '\t' || '\n' || '/' )) != NULL){
            continue;
        }

        // Armazenar palavra na lista de palavras
        dicionario[counter] = strdup(word);
        if(dicionario[counter] == NULL){
            printf("Erro ao alocar memória para uma palavra.\n");
            fclose(file);
            return 1;
        }

        counter++;
    }

    fclose(file);

    /*free da memória alocada*/
    for (int i = 0; i < counter; i++) {
        free(dicionario[i]);
    }
    free(dicionario);

    return 0;
}