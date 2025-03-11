#include "main.h"

/*função que compara dois argumentos sem ter em conta maiúsculas*/
int compare(const void *arg1, const void *arg2){
    return strcasecmp(*(const char **)arg1, *(const char **)arg2);
}

int binary_search(char *word, char **dictionary, int size){
    char *key = word;
    return(bsearch(&key, dictionary, size, sizeof(char *), compare) != NULL);
}

void remove_newline(char *line){
    size_t len = strlen(line);
    if(len > 0 && line[len - 1] == '\n'){
        line[len - 1] = '\0';
    }
}

void clean_word(char *word){
    char temp[MAX_WORD]; /*criação de uma string temporária*/
    char *ptchars = "àáéíóúãõâêôçÀÁÉÍÓÚÃÕÂÊÔÇ"; /*string com os caracteres específicos do dicionário português*/
    int j = 0;

    for(int i = 0; word[i] != '\0'; i++){
        if((word[i] >= 'A' && word[i] <= 'Z') || (word[i] >= 'a' && word[i] <= 'z')){
            temp[j++] = word[i];
        }
        else if(word[i] >= '0' && word[i] <= '9'){ /*permitir números*/
            temp[j++] = word[i];
        }
        else if(strchr(ptchars, word[i]) != NULL){ /*permitir letras acentuadas - para o dicionário português*/
            temp[j++] = word[i];
        }        
        else if(word[i] == '\'' && i > 0 && word[i+1] != '\0' &&
            ((word[i-1] >= 'A' && word[i-1] <= 'Z') || (word[i-1] >= 'a' && word[i-1] <= 'z')) &&
            ((word[i+1] >= 'A' && word[i+1] <= 'Z') || (word[i+1] >= 'a' && word[i+1] <= 'z'))){
            temp[j++] = word[i];
        }
    }
    temp[j] = '\0'; /*adiciona o fim da string*/

    /*copiar a string temporária de volta para word*/
    strcpy(word, temp);
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

    char *dictionary_filename = "words";
    char *input_filename = NULL;
    char *output_filename = NULL;
    FILE *input_file = stdin; /*por padrão, toma o nosso input como texto para avaliar*/
    FILE *output_file = stdout; /*stdout, caso padrão*/

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-h") == 0){
            print_help();
            return 0;
        }
        /*if(strcmp(argv[i], "-d") == 0){
            dictionary_filename = argv[i + 1];
        }*/
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
        output_file = fopen(output_filename, "w");
        if(output_file == NULL){
            printf("Erro ao abrir o ficheiro de output.\n");
            return 1;
        }
    }
    
    FILE *file = fopen(dictionary_filename, "r");
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
        if(strchr(word, (' ' || '\t' || '\n' || '/' || '-')) != NULL){
            continue;
        }

        /*guarda as palavras no dicionário; strdup aloca memória suficiente para uma cópia da string, e retorna um pointer para a mesma*/
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

    char line[MAX_LINE];
    int line_number = 0;

    while(fgets(line, sizeof(line), input_file)){
        line_number++;
        remove_newline(line);

        /*copiar a linha para uma string; porque o strtok substitui o primeiro espaço/tab por \0, acabando por aí a variável line*/
        char line_copy[MAX_LINE]; 
        strcpy(line_copy, line);

        int has_error = 0;
        /*uso da função strtok - string token, que divide a string, neste caso a nossa linha, em palavras separadas a partir do que nós considerarmos como fim de palavra*/
        char *token = strtok(line, " \t-"); /*espaço, tab e hífen; definidos como fim de palavra*/

        while(token != NULL){
            clean_word(token);

            if(!(strspn(token, "0123456789") == strlen(token)) && !binary_search(token, dictionary, counter)){
                if(argv[5] != NULL){
                    if(!has_error){
                        fprintf(output_file, "%d: %s\n", line_number, line_copy);
                        has_error = 1;
                    }
                    fprintf(output_file, "Erro na palavra \"%s\"\n", token);
                }
                else{
                    if(!has_error){
                        printf("%d: %s\n", line_number, line_copy);
                        has_error = 1;
                    }
                    printf("Erro na palavra \"%s\"\n", token);
                }
            }
            token = strtok(NULL, " \t-");
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