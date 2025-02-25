#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    
    if(strcmp(argv[1], "-h") == 0){
        print_help();
    }

    return 0;
}