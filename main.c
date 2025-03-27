#include "main.h"

bool output(int argc, char *argv[]){
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-o") == 0){
            return 1;
        }
    }
    return 0;
}

int compare_suggestions(const void *a, const void *b){
    suggestion_data *s1 = (suggestion_data *)a;
    suggestion_data *s2 = (suggestion_data *)b;

    /*comparar pelo número de diferenças primeiro*/
    if(s1->differences != s2->differences){
        return s1->differences - s2->differences;
    }
    /*se o número de diferenças for o mesmo, começar a ordenar pela ordem do dicionário*/
    return strcasecmp(s1->word, s2->word);
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

void split(char *word, char **dictionary, int counter, suggestion_data *list, int *found, int maxdiffs){

    for(int i = 1; i < strlen(word) - 1; i++){
        char left[MAX_WORD], right[MAX_WORD];

        /*strncpy copia os primeiros i caracteres de uma string para a memória de outra string - left neste caso*/
        strncpy(left, word, i);
        left[i] = '\0'; /*finaliza a string*/
        strcpy(right, &word[i]); /*copia o resto que ficou, para a outra parte*/

        /*só se ambas as partes existirem no dicionário é que as junta num combined*/
        if(binary_search(left, dictionary, counter) && binary_search(right, dictionary, counter)){
            /*combinar as novas strings numa só*/
            char combined[MAX_WORD * 2];
            snprintf(combined, sizeof(combined), "%s %s", left, right);

            /*verifica se a palavra é duplicada*/
            int duplicate = 0;
            for(int j = 0; j < *found; j++){
                if(strcasecmp(list[j].word, combined) == 0){
                    duplicate = 1;
                    break;
                }
            }

            /*armazena as informações da string combined na estrutura para depois conseguir comparar com as restantes sugestões*/
            if(!duplicate && *found < counter){
                list[*found].word = strdup(combined);
                list[*found].differences = 1;
                (*found)++;
            }
        }
    }
}

char* calculate_differences(char *token, char *word, int kanye){
    int tokenlen = strlen(token);
    int wordlen = strlen(word);
    int i = 0, j = 0;
    int num_diffs = 0;
    int offset = kanye;

    while(i < tokenlen && j < wordlen){
        if(tolower(token[i]) == tolower(word[j])){
            i++;
            j++;
        } 
        else{
            num_diffs++;
            if(num_diffs > kanye){
                return NULL;
            }

            if(offset > 0){
                if(tokenlen > wordlen){
                    i += offset;
                }
                else if(wordlen > tokenlen){
                    j += offset;
                }
                else{
                    i += offset;
                    j += offset;
                }
                offset = 0;
            }
            else{
                i++;
                j++;
            }
        }
    }

    num_diffs += abs((tokenlen - i) - (wordlen - j));

    if(num_diffs <= kanye){
        return word;
    }
    return NULL;
}

/*int calculate_differences_reverse(char *token, char *word){
    int tokenlen = strlen(token);
    int wordlen = strlen(word);
    int i = tokenlen - 1;
    int j = wordlen - 1;
    int numberofdiffs = 0;

    while(i >= 0 && j >= 0){
        if(tolower(token[i]) != tolower(word[j])){
            numberofdiffs++;
        }
        i--;
        j--;
    }

    numberofdiffs += abs((i + 1) - (j + 1));

    //contar as restantes diferenças quando uma palavra acaba primeira que a outra
    return numberofdiffs;
}*/

void suggestions(int counter, int alt, char *token, char **dictionary, int maxdiffs, int argc, char *argv[], FILE *output_file){
    suggestion_data *list = malloc(counter * sizeof(suggestion_data));
    if(list == NULL){
        printf("Erro ao alocar memória para as sugestões.\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < counter; i++) {
        for (int diffs = 1; diffs <= maxdiffs; diffs++) {
            char *sugestion = calculate_differences(token, dictionary[i], diffs);
            if (sugestion) {
                // Verificar se a sugestão já foi adicionada
                int duplicado = 0;
                for (int j = 0; j < found; j++) {
                    if (strcasecmp(list[j].word, sugestion) == 0) {
                        duplicado = 1;
                        break;
                    }
                }

                if (!duplicado && found < counter) {
                    list[found].word = strdup(sugestion);
                    list[found].differences = diffs;
                    found++;
                }
            }
        }
    }
    /*chamada da função split que verifica se a palavra errada pode ser formada a partir de outras duas palavras*/
    split(token, dictionary, counter, list, &found, maxdiffs);
    
    /*qsort para ordenar a list - para printar pela ordem especificada as sugestões*/
    qsort(list, found, sizeof(suggestion_data), compare_suggestions);

    for(int i = 0; i < found && i < alt; i++){
        if(i > 0){
            if(output(argc, argv) == 1){
                fprintf(output_file, ", ");
            } 
            else{
                printf(", ");
            }
        }
        if(output(argc, argv) == 1){
            fprintf(output_file, "%s", list[i].word);
        } 
        else{
            printf("%s", list[i].word);
        }
    }
    /*free da memória alocadad para as palavras combinadas (combined)*/
    for (int i = 0; i < found; i++) {
        free(list[i].word);
    }
    free(list);
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
                /*chamada da função suggestions para começar a procura de sugestões para a palavra errada*/
                suggestions(counter, alt, token, dictionary, diffs, argc, argv, output_file);
            }
            token = strtok(NULL, " \t-");
        }
    }
}

/*void mode3(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, int diffs){
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), input_file)) {
        remove_newline(line);
        char line_copy[MAX_LINE];
        strcpy(line_copy, line);

        char *token = strtok(line, " \t");
        char corrected_line[MAX_LINE] = "";

        while (token != NULL) {
            char original_word[MAX_WORD];
            strcpy(original_word, token);

            int has_hyphen = (strchr(original_word, '-') != NULL);

            clean_word(token);

            if (strspn(token, "0123456789") == strlen(token) || binary_search(token, dictionary, counter)) {
                strcat(corrected_line, original_word);
            } 
            else {
                suggestion_data list[alt];
                int found = 0;
                for (int i = 0; i < counter && found < alt; i++) {
                    int diffs_count = calculate_differences(token, dictionary[i]);
                    if (diffs_count <= diffs) {
                        list[found].word = dictionary[i];
                        list[found].differences = diffs_count;
                        found++;
                    }
                }

                qsort(list, found, sizeof(suggestion_data), compare_suggestions);

                if (found > 0) {
                    // Se a palavra original tinha hífen, preservamos isso
                    if (has_hyphen) {
                        strcpy(original_word, list[0].word);  // Mantemos a estrutura
                    } else {
                        strcat(corrected_line, list[0].word);
                    }
                } 
                else {
                    strcat(corrected_line, original_word);
                }
            }

            token = strtok(NULL, " \t");
            if (token != NULL) {
                strcat(corrected_line, " ");
            }
        }

        if (output(argc, argv)) {
            fprintf(output_file, "%s\n", corrected_line);
        } 
        else {
            printf("%s\n", corrected_line);
        }
    }
}
*/

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