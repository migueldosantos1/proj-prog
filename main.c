#include "main.h"

/*função que verifica se foi fornecido ficheiro de output*/
bool output(int argc, char *argv[]){
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-o") == 0){
            return 1;
        }
    }
    return 0;
}

int compare_suggestions(const void *a, const void *b){
    const Suggestion *s1 = (const Suggestion *)a;
    const Suggestion *s2 = (const Suggestion *)b;

    if(s1->differences != s2->differences){
        return s1->differences - s2->differences; /*ordenar pelas diferenças*/
    }
    return s1->index - s2->index; /*se tiverem o mesmo número de diferenças, ordena pelo índice no dicionário (guardado na estrutura)*/
}

/*função que adiciona as informações das sugestões encontradas à estrutura*/
void add_suggestion(Suggestion *suggestions, int *suggestion_count, char *word, int differences, int index){
    /*se a sugestão não tiver sido encontrada, guarda as informações da mesma*/
    if(!already_exists(suggestions, *suggestion_count, word)){
        suggestions[*suggestion_count].word = strdup(word);
        suggestions[*suggestion_count].differences = differences;
        suggestions[*suggestion_count].index = index;
        /*counter para as sugestões, para não ultrapassar o valor fornecido na linha de comandos*/
        (*suggestion_count)++;
    }
}

int already_exists(Suggestion *suggestions, int count, char *word){
    for(int i = 0; i < count; i++){
        if(strcasecmp(suggestions[i].word, word) == 0){
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
    /*cálcula o comprimento da string*/
    size_t len = strlen(line);
    /*remove o caracter de nova linha das strins do ficheiro de input*/
    if(len > 0 && line[len - 1] == '\n'){
        line[len - 1] = '\0';
    }
}

void clean_word(char *word){
    char temp[MAX_WORD];
    char *ptchars = "àáéíóúãõâêôçÀÁÉÍÓÚÃÕÂÊÔÇ"; /*string com os caracteres específicos do dicionário português*/
    int j = 0;

    for(int i = 0; word[i] != '\0'; i++){
        if((word[i] >= 'A' && word[i] <= 'Z') || (word[i] >= 'a' && word[i] <= 'z')){ /*guardam as letras normais*/
            temp[j++] = word[i];
        }
        else if(word[i] >= '0' && word[i] <= '9'){ /*permite números*/
            temp[j++] = word[i];
        }
        else if(strchr(ptchars, word[i]) != NULL){ /*permite letras acentuadas - para o dicionário português*/
            temp[j++] = word[i];
        }
        else if(word[i] == '\'' && i > 0 && word[i+1] != '\0' &&
            ((word[i-1] >= 'A' && word[i-1] <= 'Z') || (word[i-1] >= 'a' && word[i-1] <= 'z')) &&
            ((word[i+1] >= 'A' && word[i+1] <= 'Z') || (word[i+1] >= 'a' && word[i+1] <= 'z'))){
            temp[j++] = word[i];
        }
    }
    temp[j] = '\0'; /*termina a string*/

    /*copiar a string temporária de volta para word*/
    strcpy(word, temp);
}

/*interface de ajuda*/
void print_help(){
    printf("-h              mostra a ajuda para o utilizador e termina\n");
    printf("-i filename     nome do ficheiro de entrada, em alternativa a stdin\n");
    printf("-o filename     nome do ficheiro de saída, em alternativa a stdout\n");
    printf("-d filename     nome do ficheiro de dicionário a usar\n");
    printf("-a nn           o número máximo de alternativas a mostrar com cada erro ortográfico deve ser nn\n");
    printf("-n nn           o número máximo de diferenças a considerar nos erros ortográficos deve ser nn\n");
    printf("-m nn           o modo de funcionamento do programa deve ser nn\n");
}

void split(char *word, char **dictionary, int counter, Suggestion *suggestions, int *suggestion_count, int offset, int alt){
    int wordlen = strlen(word);
    
    for(int i = 1; i <= wordlen - 1; i++){
        char left[MAX_WORD], right[MAX_WORD];
        char left_upper[MAX_WORD], right_upper[MAX_WORD], left_lower[MAX_WORD];
        char left_capitalized[MAX_WORD], right_lower[MAX_WORD];

        /*copia as partes, esquerda e direita*/
        strncpy(left, word, i);
        left[i] = '\0';
        strcpy(right, &word[i]);

        /*cria versões em maiúsculas*/
        strcpy(left_upper, left);
        strcpy(right_upper, right);
        for(char *p = left_upper; *p; p++) *p = toupper(*p);

        /*cria versões em caps lock (primeira letra maiúscula) para string esquerda*/
        if(left[0]){
            strcpy(left_capitalized, left);
            left_capitalized[0] = toupper(left[0]);
            for(char *p = left_capitalized+1; *p; p++) *p = tolower(*p);
        }

        /*cria versão em minúsculas para a string direita*/
        strcpy(right_lower, right);
        for(char *p = right_lower; *p; p++) *p = tolower(*p);
        /*cria versão em minúsculas para a string esquerda*/
        strcpy(left_lower, left);
        for(char *p = left_lower; *p; p++) *p = tolower(*p);

        /*variáveis para armazenar as melhores correspondências*/
        char *best_left = NULL;
        char *best_right = NULL;
        int best_left_index = counter;
        int best_right_index = counter;

        /*busca no dicionário com prioridades*/
        for(int j = 0; j < counter; j++){
            /*verifica caps lock primeiro (maior prioridade) para a string esquerda*/
            if(strcmp(left_upper, dictionary[j]) == 0 && j < best_left_index){
                best_left = dictionary[j];
                best_left_index = j;
            }
            /*depois verifica caps lock (segunda prioridade) para a string esquerda*/
            else if(strcmp(left_capitalized, dictionary[j]) == 0 && j < best_left_index && best_left == NULL){
                best_left = dictionary[j];
                best_left_index = j;
            }
            /*por último verifica a versão original (terceira prioridade) para a string esquerda*/
            else if(strcmp(left, dictionary[j]) == 0 && j < best_left_index && best_left == NULL){
                best_left = dictionary[j];
                best_left_index = j;
            }
            else if(strcmp(left_lower, dictionary[j]) == 0 && j < best_left_index){
                best_left = dictionary[j];
                best_left_index = j;
            }
            /*para a direita, verifica versão original primeiro*/
            if(strcmp(right, dictionary[j]) == 0 && j < best_right_index){
                best_right = dictionary[j];
                best_right_index = j;
            }
            /*por último verifica versão em minúsculas*/
            else if(strcmp(right_lower, dictionary[j]) == 0 && j < best_right_index && best_right == NULL){
                best_right = dictionary[j];
                best_right_index = j;
            }
        }
        /*se encontrou ambas as partes, junta as duas strings numa só*/
        if(best_left && best_right){
            char combined[MAX_WORD * 2];
            snprintf(combined, sizeof(combined), "%s %s", best_left, best_right);
            /*adiciona à estrutura*/
            add_suggestion(suggestions, suggestion_count, combined, 1, best_left_index);
        }
    }
}

void find_suggestions(FILE *input_file, FILE *output_file, char* token, char* word, int offset, Suggestion *suggestions, int *suggestion_count, int alt, int index){
    /*resolve conflitos de memória*/
    if(!token || !word || !suggestions || !suggestion_count){
        return;
    }
    int tokenlen = token ? strlen(token) : 0;
    int wordlen = word ? strlen(word) : 0;
    int i = 0, j = 0, happened = 0, differences = 0, flag = 0;
    if(tokenlen == 0 || wordlen == 0 || token[0] == '\0' || word[0] == '\0'){
        return;
    }
    
    /*---------------------------------------------------------------------------------CONSIDERA-OFFSET-------------------------------------------------------------------------------------------------*/
    /*mesmo tamanho*/
    if(tokenlen > wordlen){
        for(int p = 0; p < tokenlen; p++){
            if((tolower(token[i]) == tolower(word[j])) && i < tokenlen && j < wordlen){
                /*se a palavra acabar, sai do for e continua*/
                if(i == tokenlen && j == wordlen){
                    continue;
                }
                i++;
                j++;
                /*flag para controlar iterações*/
                flag = 1;
            }
            else if((tolower(token[i]) != tolower(word[j])) && happened == 1 && i < tokenlen && j < wordlen){
                differences++;
                /*se a palavra acabar, sai do for e continua*/
                if(i == tokenlen && j == wordlen){
                    continue;
                }
                i++;
                j++;
                /*flag para incrementar p caso entre no primeiro if - para não fazer duas comparações numa só iteração*/
                if(flag == 1){
                    p++;
                }
            }
            else if((tolower(token[i]) != tolower(word[j])) && happened == 0 && i < tokenlen && j < wordlen){
                differences++;
                /*se a palavra acabar, sai do for e continua*/
                if(i == tokenlen && j == wordlen){
                    continue;
                }
                /*salta caracteres apenas uma vez*/
                happened = 1;
                /*averigurar se a diferenças entre os comprimentos das palavras é maior que 1, se sim, incrementa 1*/
                for(int l = i + 1; l < i + offset; l++){
                    if((tolower(token[l]) != tolower(word[j])) || abs(tokenlen - wordlen) > 1){
                        differences++;
                    }
                }
                /*salta offset caracteres*/
                i += offset;
                /*flag para incrementar p caso entre no primeiro if - para não fazer duas comparações numa só iteração*/
                if(flag == 1){
                    p++;
                }
            }
            /*reset da flag de controlo de iterações*/
            flag = 0;
        }
        /*se não for duplicada e se ambas as palavras tiverem acabado e número de diferenças menor ou igual que offset, guarda na estrutura*/
        if(!already_exists(suggestions, *suggestion_count, word) && differences <= offset && i == tokenlen && j == wordlen){
            add_suggestion(suggestions, suggestion_count, word, differences, index);
            return;
        }
    }
    if(wordlen > tokenlen){
        for(int p = 0; p < wordlen; p++){
            if((tolower(token[i]) == tolower(word[j])) && i < tokenlen && j < wordlen){
                if(i == tokenlen && j == wordlen){
                    continue;
                }
                i++;
                j++;
                flag = 1;
            }
            else if((tolower(token[i]) != tolower(word[j])) && happened == 1 && i < tokenlen && j < wordlen){
                differences++;
                if(i == tokenlen && j == wordlen){
                    continue;
                }
                i++;
                j++;
                if(flag == 1){
                    p++;
                }
            }
            else if((tolower(token[i]) != tolower(word[j])) && happened == 0 && i < tokenlen && j < wordlen){
                differences++;
                if(i == tokenlen && j == wordlen){
                    continue;
                }
                happened = 1;
                /*averigurar se a diferenças entre os comprimentos das palavras é maior que 1, se sim, incrementa 1*/
                for(int l = i + 1; l < i + offset; l++){
                    if((tolower(token[l]) != tolower(word[j])) || abs(tokenlen - wordlen) > 1){
                        differences++;
                    }
                }
                j += offset;
                if(flag == 1){
                    p++;
                }
            }
            flag = 0;
        }
        /*se não for duplicada e se ambas as palavras tiverem acabado e número de diferenças menor ou igual que offset, guarda na estrutura*/
        if(!already_exists(suggestions, *suggestion_count, word) && differences <= offset && i == tokenlen && j == wordlen){
            add_suggestion(suggestions, suggestion_count, word, differences, index);
            return;
        }
    }

    /*----------------------------------------------------------------------------------NÃO-CONSIDERA-OFFSET-------------------------------------------------------------------------------------------*/
    /*reset às variáveis*/
    int new_differences = 0, k = 0;
    i = 0, j = 0;
    /*palavras com tamanho igual*/
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
        if(!already_exists(suggestions, *suggestion_count, word) && new_differences <= offset){
            add_suggestion(suggestions, suggestion_count, word, new_differences, index);
            return;
        }
    }
    /*palavra errada maior que a palavra do dicionário*/
    else if(tokenlen > wordlen){
        while(k < tokenlen){
            if(i < tokenlen && j < wordlen && tolower(token[i]) == tolower(word[j])){
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
        if(!already_exists(suggestions, *suggestion_count, word) && new_differences <= offset){
            add_suggestion(suggestions, suggestion_count, word, new_differences, index);
            return;
        }
    }
    /*palavra do dicionário maior que a palavra errada*/
    else{
        k = 0, i = 0, j = 0, new_differences = 0; /*garantir que está inicializado*/
        
        while(k < wordlen){
            /*verificar limites antes de aceder a token[i] e word[j]*/
            if(i < strlen(token) && j < strlen(word)){
                if(tolower(token[i]) == tolower(word[j])){
                    i++;
                    j++;
                }
                else {
                    new_differences++;
                    i++;
                    j++;
                }
            }
            else{
                new_differences++;
            }
            k++;
        }
        if(!already_exists(suggestions, *suggestion_count, word) && new_differences <= offset){
            add_suggestion(suggestions, suggestion_count, word, new_differences, index);
            return;
        }
    }
}

void find_suggestions_reversed(FILE *input_file, FILE *output_file, char* token, char* word, int offset, Suggestion *suggestions, int *suggestion_count, int alt, int index){
    int tokenlen = strlen(token);
    int wordlen = strlen(word);
    int i = (tokenlen - 1), j = (wordlen - 1), differences = 0;
    
    /*mesmo tamanho*/
    if(tokenlen == wordlen){
        for(int k = 0; k < tokenlen; k++){
            if(tolower(token[i]) == tolower(word[j])){
                i--;
                j--;
            }
            else{
                differences++;
                i--;
                j--;
            }
        }
        /*apenas encontra uma palavra alternativa se o número de diferenças de que andamos à procura for igual ao valor especificado na linha de comandos*/
        if(!already_exists(suggestions, *suggestion_count, word) && differences == offset){
            add_suggestion(suggestions, suggestion_count, word, differences, index);
            return;
        }
    }
    /*reset das variáveis*/
    i = (tokenlen - 1), j = (wordlen - 1), differences = 0;
    /*token maior do que a palavra do dicionário*/
    if(tokenlen > wordlen){
        for(int k = 0; k < wordlen; k++){
            if(tolower(token[i]) == tolower(word[j])){
                i--;
                j--;
            }
            else{
                differences++;
                i--;
                j--;
            }
        }
        differences += abs(tokenlen - wordlen);
        /*apenas encontra uma palavra alternativa se o número de diferenças de que andamos à procura for igual ao valor especificado na linha de comandos*/
        if(!already_exists(suggestions, *suggestion_count, word) && differences == offset){
            add_suggestion(suggestions, suggestion_count, word, differences, index);
            return;
        }
    }
    /*reset das variáveis*/
    i = (tokenlen - 1), j = (wordlen - 1), differences = 0;
    /*palavra do dicionário maior que o token*/
    if(wordlen > tokenlen){
        for(int k = 0; k < tokenlen; k++){
            if(tolower(token[i]) == tolower(word[j])){
                i--;
                j--;
            }
            else{
                differences++;
                i--;
                j--;
            }
        }
        differences += abs(tokenlen - wordlen);
        /*apenas encontra uma palavra alternativa se o número de diferenças de que andamos à procura for igual ao valor especificado na linha de comandos*/
        if(!already_exists(suggestions, *suggestion_count, word) && differences == offset){
            add_suggestion(suggestions, suggestion_count, word, differences, index);
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
            /*faz a procura pelo dicionário*/
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
            /*faz a procura pelo dicionário*/
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
                /*aloca memória para as sugestões*/
                Suggestion *suggestions = (Suggestion *)malloc(alt * MAX_WORD * sizeof(Suggestion));
                int suggestion_count = 0;
                /*chama a função split primeiro*/
                split(token, dictionary, counter, suggestions, &suggestion_count, diffs, alt);
                /*para 1 offset até n fornecido, percorre o as palavras do dicionário*/
                for(int offset = 1; offset <= diffs; offset++){
                    for(int j = 0; j < counter; j++){
                        find_suggestions(input_file, output_file, token, dictionary[j], offset, suggestions, &suggestion_count, alt, j);
                    }
                }
                
                /*chamada da função que percorre as palavras do fim para o início*/
                for(int p = 0; p < counter; p++){
                    find_suggestions_reversed(input_file, output_file, token, dictionary[p], diffs, suggestions, &suggestion_count, alt, p);
                }
                /*ordena as sugestões de acordo com as prioridades definidas na função compare_suggestions*/
                qsort(suggestions, suggestion_count, sizeof(Suggestion), compare_suggestions);

                int print_count = (suggestion_count < alt) ? suggestion_count : alt;
                /*print das sugestões*/
                for(int i = 0; i < print_count; i++){
                    if(output(argc, argv) == 1){
                        fprintf(output_file, "%s", suggestions[i].word);
                    } 
                    else{
                        printf("%s", suggestions[i].word);
                    }
                    if(i < (print_count - 1)){
                        if(output(argc, argv) == 1){
                            fprintf(output_file, ", ");
                        } 
                        else{
                            printf(", ");
                        }
                    }
                }
                /*free da memória alocada para as sugestões*/
                for(int i = 0; i < suggestion_count; i++){
                    free(suggestions[i].word);
                }
                free(suggestions);
                fprintf(output_file, "\n");
            }
            token = strtok(NULL, " \t-");
        }
    }
}

void mode3(FILE *input_file, FILE *output_file, char **dictionary, int counter, int argc, char *argv[], int alt, int diffs) {
    char line[MAX_LINE];
    int line_number = 0;

    while(fgets(line, sizeof(line), input_file)) {
        line_number++;
        remove_newline(line);

        char line_copy[MAX_LINE];
        strcpy(line_copy, line);

        char *tokens[MAX_LINE];
        char *separators[MAX_LINE];
        int token_count = 0;

        char *ptr = line_copy;
        while (*ptr != '\0') {
            // Trata pontuação como token separado
            if (strchr(".,():;\"?!", *ptr) != NULL) {
                tokens[token_count] = (char *)malloc(2 * sizeof(char));
                tokens[token_count][0] = *ptr;
                tokens[token_count][1] = '\0';
                separators[token_count] = NULL;
                token_count++;
                ptr++;
                continue;
            }

            while (*ptr == ' ' || *ptr == '\t' || *ptr == '-') {
                ptr++;
            }
            if (*ptr == '\0') break;

            char *token_start = ptr;
            while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '-' && strchr(".,():;?!", *ptr) == NULL) {
                ptr++;
            }

            if (*ptr != '\0') {
                separators[token_count] = (char *)malloc(2 * sizeof(char));
                separators[token_count][0] = *ptr;
                separators[token_count][1] = '\0';
                ptr++;
            } else {
                separators[token_count] = NULL;
            }

            // Extrai o token
            int token_len = ptr - token_start;
            if (token_len > 0) {
                tokens[token_count] = (char *)malloc((token_len + 1) * sizeof(char));
                strncpy(tokens[token_count], token_start, token_len);
                tokens[token_count][token_len] = '\0';
                token_count++;
            }
        }

        // Processa cada token (exceto pontuação)
        for(int i = 0; i < token_count; i++) {
            // Ignora tokens que são apenas pontuação
            if(strlen(tokens[i]) == 1 && strchr(".,():;\"?!", tokens[i][0])){
                continue;
            }

            char original_token[MAX_WORD];
            strcpy(original_token, tokens[i]);
            clean_word(tokens[i]);

            /*faz a procura pelo dicionário*/
            if(!(strspn(tokens[i], "0123456789") == strlen(tokens[i])) && !binary_search(tokens[i], dictionary, counter)){
                /*aloca memória para as sugestões*/
                Suggestion *suggestions = (Suggestion *)malloc(alt * MAX_WORD * sizeof(Suggestion));
                int suggestion_count = 0;

                /*chama a função split primeiro*/
                split(tokens[i], dictionary, counter, suggestions, &suggestion_count, diffs, alt);

                /*para 1 offset até n fornecido, percorre o as palavras do dicionário*/
                for(int offset = 1; offset <= diffs; offset++) {
                    for(int j = 0; j < counter; j++) {
                        find_suggestions(input_file, output_file, tokens[i], dictionary[j], offset, suggestions, &suggestion_count, alt, j);
                    }
                }
                 /*chamada da função que percorre as palavras do fim para o início*/
                for(int p = 0; p < counter; p++) {
                    find_suggestions_reversed(input_file, output_file, tokens[i], dictionary[p], diffs, suggestions, &suggestion_count, alt, p);
                }
                /*ordena as sugestões de acordo com as prioridades definidas na função compare_suggestions*/
                qsort(suggestions, suggestion_count, sizeof(Suggestion), compare_suggestions);

                /*frees das memórias alocadas ao longo da função*/
                if(suggestion_count > 0){
                    free(tokens[i]);
                    tokens[i] = strdup(suggestions[0].word);
                }

                for(int k = 0; k < suggestion_count; k++){
                    free(suggestions[k].word);
                }
                free(suggestions);
            }
        }

        int quote_open = 0; /*análise das aspas aos pares: 0 -> próxima aspa é de abertura, 1 -> próxima é de fecho*/
        /*print dos tokens*/
        for(int i = 0; i < token_count; i++){
            if(output(argc, argv) == 1){
                fprintf(output_file, "%s", tokens[i]);
            } 
            else{
                printf("%s", tokens[i]);
            }
        
            /*adiciona o separador, exceto após o último token*/
            if(i < token_count && separators[i] != NULL){
                char current_sep = separators[i][0];
                char next_char = (i + 1 < token_count && tokens[i + 1][0]) ? tokens[i + 1][0] : '\0';
                char prev_token_last_char = (i > 0 && tokens[i-1]) ? tokens[i-1][strlen(tokens[i-1])-1] : '\0';
        
                if(output(argc, argv) == 1){
                    fprintf(output_file, "%c", current_sep);
                } 
                else{
                    printf("%c", current_sep);
                }
        
                /*tratamento das aspas*/
                if (current_sep == '"') {
                    if (quote_open) {
                        if (output(argc, argv) == 1) fprintf(output_file, " ");
                        else printf(" ");
                        quote_open = 0;
                    } else {
                        /*adiciona-se um espaço após pontuação*/
                        if (prev_token_last_char == '.' || prev_token_last_char == '!' || prev_token_last_char == '?') {
                            if (output(argc, argv) == 1) fprintf(output_file, " ");
                            else printf(" ");
                        }
                        quote_open = 1;
                    }
                } 
                else if((current_sep == '.' || current_sep == ',' || current_sep == '?' || current_sep == '!' || current_sep == ';' || current_sep == ':') && next_char != '"' && next_char != '.'){
                    if(output(argc, argv) == 1){
                        fprintf(output_file, " ");
                    }
                    else{
                        printf(" ");
                    }
                }
            }
        
            free(tokens[i]);
            if(separators[i] != NULL){
                free(separators[i]);
            }
        }
        if (output(argc, argv) == 1) {
            fprintf(output_file, "\n");
        } 
        else{
            printf("\n");
        }
    }
}

int main(int argc, char *argv[]){
    char *dictionary_filename = NULL;
    char *input_filename = NULL;
    char *output_filename = NULL;
    FILE *input_file = stdin; /*por padrão, toma o nosso input como stdin*/
    FILE *output_file = stdout; /*stdout, caso padrão*/

    int alt = 10; /*por omissão, o número máximo de alternativas a mostrar é definido a 10*/
    int diffs = 2; /*por omissão, o número máximo de diferenças a considerar é definido a 2*/
    int mode = 1; /*por omissão, o modo de funcionamento é definido como 1*/
    int flag = 0; /*flag*/

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
            flag = 1;
        }
    }

    /*por omissão, o nome do dicionário é "words"*/
    if(dictionary_filename == NULL){
        dictionary_filename = "words";
    }
    /*flag para correr só com o ./ortografia e o modo à frente*/
    if(flag == 0){
        if(argc > 1){
            mode = atoi(argv[1]);
        }
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
            for(int i = 0; i < counter; i++){
                free(dictionary[i]);
            }
            free(dictionary);
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
    else if(mode == 3){
        mode3(input_file, output_file, dictionary, counter, argc, argv, alt, diffs);
    }
    else{
        printf("Modo inválido.\n");
    }

    /*free da memória alocada*/
    for(int i = 0; i < counter; i++){
        free(dictionary[i]);
    }
    free(dictionary);

    /*se "-i" e "-o" não forem emitidos, fecha os ficheiros fornecidos*/
    if(input_file != stdin){
        fclose(input_file);
    }
    if(output_file != stdout){
        fclose(output_file);
    }

    fclose(file);

    return 0;
}