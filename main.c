#include "main.h"

bool output(int argc, char *argv[]){
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-o") == 0){
            return 1;
        }
    }
    return 0;
}

int already_exists(char **suggestions, int count, char *word){
    for(int i = 0; i < count; i++){
        if(strcmp(suggestions[i], word) == 0){
            /*palavra já foi adicionada*/
            return 1;
        }
    }
    /*palavra não foi adicionada*/
    return 0;
}

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
    char temp[MAX_WORD];
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

void split(char *word, char **dictionary, int counter, char **suggestions, int *suggestion_count, int offset, int alt){
    int wordlen = strlen(word);
    
    for(int i = 1; i <= wordlen - 1; i++){
        char left[MAX_WORD], right[MAX_WORD];

        /*copia a parte esquerda da string*/
        strncpy(left, word, i);
        left[i] = '\0';
        
        /*copia a parte direita da string*/
        strcpy(right, &word[i]);

        /*verifica se ambas as partes da string estão no dicionário*/
        if(binary_search(left, dictionary, counter) && binary_search(right, dictionary, counter)){
            /*dá merge às duas partes e guarda em memória com um espaço*/
            char combined[MAX_WORD * 2];
            snprintf(combined, sizeof(combined), "%s %s", left, right);

            suggestions[*suggestion_count] = strdup(combined);
            (*suggestion_count)++;
        }
    }
}

void find_suggestions(char* token, char* word, int offset, char **suggestions, int *suggestion_count, int alt){
    int tokenlen = strlen(token);
    int wordlen = strlen(word);
    int i = 0, j = 0, happened = 0, differences = 0, storeI = 0, storeJ = 0, storeD = 0;

    while(tolower(token[i]) == tolower(word[j])){
        i++;
        j++;
    }
    if(i != (tokenlen - 1) && j == (wordlen) && (tokenlen != wordlen)){
        differences += abs(tokenlen - wordlen);
        if(!already_exists(suggestions, *suggestion_count, word) && (differences <= offset)){
            suggestions[*suggestion_count] = strdup(word);
            (*suggestion_count)++;
            return;
        }
    }
    while(1){
        if((tolower(token[i]) != tolower(word[j]))){
            differences++;
            if(happened == 1){
                break;
            }
            /*guarda os índices do i e do j e das diferenças na primeira letra errada*/
            storeI = i;
            storeJ = j;
            storeD = differences;

            /*averigurar as diferenças entre caracteres não lidos quando o offset é incrementado*/
            for(int l = i + 1; l < i + offset; l++){
                if((tolower(token[l]) != tolower(word[j]))){
                    differences++;
                }
            }

            i += offset;
            /*aumentar o índice da palavra errada --- token*/
            while(tolower(token[i]) == tolower(word[j])){
                i++;
                j++;
                if((tolower(token[i]) != tolower(word[j]))){
                    differences++;
                }
                if(differences > offset){
                    continue;
                }
                if(i == (tokenlen - 1) && j == (wordlen - 1) && (tokenlen == wordlen) && (differences <= offset)){
                    suggestions[*suggestion_count] = strdup(word);
                    (*suggestion_count)++;
                    return;
                }
                else if(i == (tokenlen - 1) && j == (wordlen - 1) && (tokenlen != wordlen)){
                    suggestions[*suggestion_count] = strdup(word);
                    (*suggestion_count)++;
                    return;
                }
            }
            storeJ += offset;
            /*aumentar o índice da palavra do dicionário --- word*/
            while(tolower(token[storeI]) == tolower(word[storeJ])){
                storeI++;
                storeJ++;
                if((tolower(token[storeI]) != tolower(word[storeJ]))){
                    storeD++;
                }
                if(storeD > offset){
                    continue;
                }
                if(storeI == (tokenlen - 1) && storeJ == (wordlen - 1) && (tokenlen == wordlen) && (storeD <= offset)){
                    suggestions[*suggestion_count] = strdup(word);
                    (*suggestion_count)++;
                    return;
                }
                else if(storeI == (tokenlen - 1) && storeJ == (wordlen - 1) && (tokenlen != wordlen)){
                    suggestions[*suggestion_count] = strdup(word);
                    (*suggestion_count)++;
                    return;
                }
            }
            happened = 1;
        }
    }
    /*reset às variáveis*/
    int new_differences = 0, k = 0;
    i = 0, j = 0;
    if(tokenlen == wordlen){
        while(k < tokenlen){
            if(tolower(token[i]) == tolower(word[j])){
                i++;
                j++;
            }
            else{
                new_differences++;
                i++;
                j++;
            }
            k++;
        }
        if(new_differences <= offset){
            suggestions[*suggestion_count] = strdup(word);
            (*suggestion_count)++;
            return;
        }
    }
    else if(tokenlen > wordlen){
        while(k < tokenlen){
            if(tolower(token[i]) == tolower(word[j])){
                i++;
                j++;
            }
            else{
                new_differences++;
                i++;
                j++;
            }
            k++;
        }
        new_differences += abs(tokenlen - wordlen);
        if(new_differences <= offset){
            suggestions[*suggestion_count] = strdup(word);
            (*suggestion_count)++;
            return;
        }
    }
    else{
        while(k < wordlen){
            if(tolower(token[i]) == tolower(word[j])){
                i++;
                j++;
            }
            else{
                new_differences++;
                i++;
                j++;
            }
            k++;
        }
        new_differences += abs(tokenlen - wordlen);
        if(new_differences <= offset){
            suggestions[*suggestion_count] = strdup(word);
            (*suggestion_count)++;
            return;
        }
    }
}

void mode1(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[]){
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
                if(output(argc, argv) == 1){
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
}

void mode2(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, char *word, int diffs){
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
                if(output(argc, argv) == 1){
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

                char **suggestions = (char **)malloc(alt * MAX_WORD * sizeof(char *));
                int suggestion_count = 0;

                split(token, dictionary, counter, suggestions, &suggestion_count, diffs, alt);

                for(int offset = 1; offset <= diffs; offset++){
                    for(int j = 0; j < counter; j++){
                        if(suggestion_count <= alt){
                            find_suggestions(token, dictionary[j], offset, suggestions, &suggestion_count, alt);
                        }
                    }
                }

                if(suggestion_count > 0){
                    for(int i = 0; i < suggestion_count; i++){
                        if(output(argc, argv) == 1){
                            fprintf(output_file, "%s", suggestions[i]);
                            if(i < (suggestion_count - 1)){
                                fprintf(output_file, ", ");
                            }
                        }
                        else{
                            printf("%s", suggestions[i]);
                            if(i < (suggestion_count - 1)){
                                printf(", ");
                            }
                        }
                        free(suggestions[i]);
                    }
                }
                free(suggestions);
                fprintf(output_file, "\n");
            }
            token = strtok(NULL, " \t-");
        }
    }
}

/*void mode3(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, int diffs){
}*/

int main(int argc, char *argv[]){
    char *dictionary_filename = NULL;
    char *input_filename = NULL;
    char *output_filename = NULL;
    FILE *input_file = stdin; /*por padrão, toma o nosso input como stdin*/
    FILE *output_file = stdout; /*stdout, caso padrão*/

    int alt = 10; /*por omissão, o número máximo de alternativas a mostrar é definido a 10*/
    int diffs = 2; /*por omissão, o número máximo de diferenças a considerar é definido a 2*/
    int mode = 1; /*por omissão, o modo de funcionamento é definido como 1*/

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-h") == 0){
            print_help();
            return 0;
        }
        /*o argumento "-d" exige um argumento com o nome do dicionário posteriormente*/
        else if(strcmp(argv[i], "-d") == 0 && i + 1 < argc){
            dictionary_filename = argv[++i];
        }
        else if(strcmp(argv[i], "-i") == 0 && i + 1 < argc){
            input_filename = argv[++i];
        }
        else if(strcmp(argv[i], "-o") == 0 && i + 1 < argc){
            output_filename = argv[++i];
        }
        else if(strcmp(argv[i], "-a") == 0 && i + 1 < argc){
            alt = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-n") == 0 && i + 1 < argc){
            diffs = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-m") == 0 && i + 1 < argc){
            mode = atoi(argv[++i]);
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

    /*chamada da função --- funcionamento 1, caso mode seja 1 como é óbvio*/
    if(mode == 1){
        mode1(input_file, output_file, dictionary, counter, argc, argv);
    }
    else if(mode == 2){
        mode2(input_file, output_file, dictionary, counter, argc, argv, alt, word, diffs);
    }
    /*else if(mode == 3){
        mode3(input_file, output_file, dictionary, counter, argc, argv, alt, diffs);
    }*/
    else{
        printf("Modo inválido.\n");
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