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
    const Suggestion *s1 = (const Suggestion *)a;
    const Suggestion *s2 = (const Suggestion *)b;

    if(s1->differences != s2->differences){
        return s1->differences - s2->differences; /*ordenar pelas diferenças*/
    }
    return s1->index - s2->index; /*se as tiverem as mesmas diferenças, ordena pelo índice no dicionário*/
}

/*função que adiciona as informações das sugestões encontradas à estrutura*/
void add_suggestion(Suggestion *suggestions, int *suggestion_count, char *word, int differences, int index){
    if(!already_exists(suggestions, *suggestion_count, word)){
        suggestions[*suggestion_count].word = strdup(word);
        suggestions[*suggestion_count].differences = differences;
        suggestions[*suggestion_count].index = index;
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

void split(char *word, char **dictionary, int counter, Suggestion *suggestions, int *suggestion_count, int offset, int alt){
    int wordlen = strlen(word);
    
    for(int i = 1; i <= wordlen - 1; i++){
        char left[MAX_WORD], right[MAX_WORD];
        char left_upper[MAX_WORD], right_upper[MAX_WORD], left_lower[MAX_WORD];
        char left_capitalized[MAX_WORD], right_lower[MAX_WORD];

        /* Copia as partes */
        strncpy(left, word, i);
        left[i] = '\0';
        strcpy(right, &word[i]);

        /* Cria versões em maiúsculas */
        strcpy(left_upper, left);
        strcpy(right_upper, right);
        for(char *p = left_upper; *p; p++) *p = toupper(*p);

        /* Cria versões capitalizadas (primeira letra maiúscula) para esquerda */
        if(left[0]){
            strcpy(left_capitalized, left);
            left_capitalized[0] = toupper(left[0]);
            for(char *p = left_capitalized+1; *p; p++) *p = tolower(*p);
        }

        /* Cria versão em minúsculas para direita */
        strcpy(right_lower, right);
        for(char *p = right_lower; *p; p++) *p = tolower(*p);
        /* Cria versão em minúsculas para esquerda */
        strcpy(left_lower, left);
        for(char *p = left_lower; *p; p++) *p = tolower(*p);

        /* Variáveis para armazenar as melhores correspondências */
        char *best_left = NULL;
        char *best_right = NULL;
        int best_left_index = counter;
        int best_right_index = counter;

        /* Busca no dicionário com prioridades */
        for(int j = 0; j < counter; j++){
            /* Verifica CAPS LOCK primeiro (maior prioridade) para esquerda */
            if(strcmp(left_upper, dictionary[j]) == 0 && j < best_left_index){
                best_left = dictionary[j];
                best_left_index = j;
            }
            /* Depois verifica capitalizado (segunda prioridade) para esquerda */
            else if(strcmp(left_capitalized, dictionary[j]) == 0 && j < best_left_index && best_left == NULL){
                best_left = dictionary[j];
                best_left_index = j;
            }
            /* Por último verifica a versão original (terceira prioridade) para esquerda */
            else if(strcmp(left, dictionary[j]) == 0 && j < best_left_index && best_left == NULL){
                best_left = dictionary[j];
                best_left_index = j;
            }
            else if(strcmp(left_lower, dictionary[j]) == 0 && j < best_left_index){
                best_left = dictionary[j];
                best_left_index = j;
            }
            /* Para a direita: verifica versão original primeiro */
            if(strcmp(right, dictionary[j]) == 0 && j < best_right_index){
                best_right = dictionary[j];
                best_right_index = j;
            }
            /* Por último verifica versão em minúsculas */
            else if(strcmp(right_lower, dictionary[j]) == 0 && j < best_right_index && best_right == NULL){
                best_right = dictionary[j];
                best_right_index = j;
            }
        }
        /* Se encontrou ambas as partes */
        if(best_left && best_right){
            char combined[MAX_WORD * 2];
            snprintf(combined, sizeof(combined), "%s %s", best_left, best_right);
            
            add_suggestion(suggestions, suggestion_count, combined, 1, best_left_index);
        }
    }
}

void find_suggestions(FILE *input_file, FILE *output_file, char* token, char* word, int offset, Suggestion *suggestions, int *suggestion_count, int alt, int index){
    if(!token || !word || !suggestions || !suggestion_count) return;
    int tokenlen = token ? strlen(token) : 0;
    int wordlen = word ? strlen(word) : 0;
    int i = 0, j = 0, happened = 0, differences = 0, storeI = 0, storeJ = 0, storeD = 0, kdot = 0;
    if(tokenlen == 0 || wordlen == 0 || token[0] == '\0' || word[0] == '\0') return;
    
    while(tolower(token[i]) == tolower(word[j]) && i < tokenlen && j < wordlen){
        i++;
        j++;
        if(i == (tokenlen - 1) || j == (wordlen - 1)){
            kdot = 1;
            continue;
        }
    }
    /*para encontrar o cent --- j == wordlen pq já sai do ciclo acima com os índices incrementados, não contabilizando a fim das palavras*/
    if(i != (tokenlen - 1) && j == (wordlen) && (tokenlen != wordlen)){
        differences += abs(tokenlen - wordlen);
        if(!already_exists(suggestions, *suggestion_count, word) && (differences <= offset)){
            add_suggestion(suggestions, suggestion_count, word, differences, index);
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

            /*averigurar se a diferenças entre os comprimentos das palavras é maior que 1, se sim, incrementa 1*/
            for(int l = i + 1; l < i + offset; l++){
                if((tolower(token[l]) != tolower(word[j])) && abs(tokenlen - wordlen) > 1){
                    differences++;
                    storeD++;
                }
            }

            i += offset;
            /*aumentar o índice da palavra errada --- token*/
            while(tolower(token[i]) == tolower(word[j])){  //ERRO
                if(kdot == 0){
                    i++;
                    j++;
                }
                if(i < tokenlen && j < wordlen && (tolower(token[i]) != tolower(word[j]))){
                    differences++;
                }
                if(differences > offset){
                    continue;
                }
                if(!already_exists(suggestions, *suggestion_count, word) && i == (tokenlen - 1) && j == (wordlen - 1) && (tokenlen == wordlen) && (differences <= offset)){
                    add_suggestion(suggestions, suggestion_count, word, differences, index);
                    return;
                }
                else if(!already_exists(suggestions, *suggestion_count, word) && i == (tokenlen - 1) && j == (wordlen - 1) && (tokenlen != wordlen)){
                    add_suggestion(suggestions, suggestion_count, word, differences, index);
                    return;
                }
                if(kdot == 1){
                    i++;
                    j++;
                }
            }
            storeJ += offset;
            /*aumentar o índice da palavra do dicionário --- word*/
            while(storeI < tokenlen && storeJ < wordlen && tolower(token[storeI]) == tolower(word[storeJ])){
                if(kdot == 0){
                    storeI++;
                    storeJ++;
                }
                if(storeI < tokenlen && storeJ < wordlen && (tolower(token[storeI]) != tolower(word[storeJ]))){
                    storeD++;
                }
                if(storeD > offset){
                    continue;
                }
                if(!already_exists(suggestions, *suggestion_count, word) && storeI == (tokenlen - 1) && storeJ == (wordlen - 1) && (tokenlen == wordlen) && (storeD <= offset)){
                    add_suggestion(suggestions, suggestion_count, word, storeD, index);
                    return;
                }
                else if(!already_exists(suggestions, *suggestion_count, word) && storeI == (tokenlen - 1) && storeJ == (wordlen - 1) && (tokenlen != wordlen)){
                    add_suggestion(suggestions, suggestion_count, word, storeD, index);
                    return;
                }
                if(kdot == 1){
                    storeI++;
                    storeJ++;
                }
            }
            happened = 1;
        }
        continue;
    }
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
        k = 0;
        i = 0;
        j = 0;
        new_differences = 0; /*garantir que está inicializado*/
        
        while(k < wordlen) {
            /*verificar limites antes de acessar token[i] e word[j]*/
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
    /*algoritmo para encontrar o centrist*/
    i = 0, j = 0, differences = 0;
    while(i < tokenlen && j < wordlen && tolower(token[i]) == tolower(word[j])){
        i++;
        j++;
    }
    while(i < tokenlen && j < wordlen && (tolower(token[i]) != tolower(word[j]))){
        differences++;
        if(!already_exists(suggestions, *suggestion_count, word) && i == (tokenlen - 1) && (wordlen > tokenlen)){
            differences += abs(tokenlen - wordlen);
            if(differences <= offset){
                add_suggestion(suggestions, suggestion_count, word, differences, index);
                return;
            }
        }
        i++;
        j++;
    }
}

void find_suggestions_reversed(FILE *input_file, FILE *output_file, char* token, char* word, int offset, Suggestion *suggestions, int *suggestion_count, int alt, int index){
    int tokenlen = strlen(token);
    int wordlen = strlen(word);
    int i = (tokenlen - 1), j = (wordlen - 1), differences = 0;
    
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
        if(!already_exists(suggestions, *suggestion_count, word) && differences == offset){ /*para resolver a cena das 3 diferenças com o enured é só pôr <= em vez de ==, mas não é o que o enunciado diz*/
            add_suggestion(suggestions, suggestion_count, word, differences, index);
            return;
        }
    }
    /*reset das variáveis*/
    i = (tokenlen - 1), j = (wordlen - 1), differences = 0;
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

                Suggestion *suggestions = (Suggestion *)malloc(alt * MAX_WORD * sizeof(Suggestion));
                int suggestion_count = 0;

                split(token, dictionary, counter, suggestions, &suggestion_count, diffs, alt);

                for(int offset = 1; offset <= diffs; offset++){
                    for(int j = 0; j < counter; j++){
                        find_suggestions(input_file, output_file, token, dictionary[j], offset, suggestions, &suggestion_count, alt, j);
                    }
                }
                
                /*chamada da função que percorre as palavras do fim para o início*/
                for(int p = 0; p < counter; p++){
                    find_suggestions_reversed(input_file, output_file, token, dictionary[p], diffs, suggestions, &suggestion_count, alt, p);
                }

                qsort(suggestions, suggestion_count, sizeof(Suggestion), compare_suggestions);

                int print_count = (suggestion_count < alt) ? suggestion_count : alt;

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
                    //printf("Sugestão: %s (Dif: %d, Index: %d)\n", suggestions[i].word, suggestions[i].differences, suggestions[i].index);

                }
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
            if(strlen(tokens[i]) == 1 && strchr(".,():;\"?!", tokens[i][0])) {
                continue;
            }

            char original_token[MAX_WORD];
            strcpy(original_token, tokens[i]);
            clean_word(tokens[i]);

            if(!(strspn(tokens[i], "0123456789") == strlen(tokens[i])) && !binary_search(tokens[i], dictionary, counter)) {
                Suggestion *suggestions = (Suggestion *)malloc(alt * MAX_WORD * sizeof(Suggestion));
                int suggestion_count = 0;

                split(tokens[i], dictionary, counter, suggestions, &suggestion_count, diffs, alt);

                for(int offset = 1; offset <= diffs; offset++) {
                    for(int j = 0; j < counter; j++) {
                        find_suggestions(input_file, output_file, tokens[i], dictionary[j], offset, suggestions, &suggestion_count, alt, j);
                    }
                }

                for(int p = 0; p < counter; p++) {
                    find_suggestions_reversed(input_file, output_file, tokens[i], dictionary[p], diffs, suggestions, &suggestion_count, alt, p);
                }

                qsort(suggestions, suggestion_count, sizeof(Suggestion), compare_suggestions);

                if(suggestion_count > 0) {
                    free(tokens[i]);
                    tokens[i] = strdup(suggestions[0].word);
                }

                for(int k = 0; k < suggestion_count; k++) {
                    free(suggestions[k].word);
                }
                free(suggestions);
            }
        }

        int quote_open = 0; // 0 -> próxima aspa é de abertura, 1 -> próxima é de fecho
        // Imprime os tokens
        for (int i = 0; i < token_count; i++) {
            if (output(argc, argv) == 1) {
                fprintf(output_file, "%s", tokens[i]);
            } else {
                printf("%s", tokens[i]);
            }
        
            // Adiciona o separador, exceto após o último token
            if (i < token_count && separators[i] != NULL) {
                char current_sep = separators[i][0];
                char next_char = (i + 1 < token_count && tokens[i + 1][0]) ? tokens[i + 1][0] : '\0';
                char prev_token_last_char = (i > 0 && tokens[i-1]) ? tokens[i-1][strlen(tokens[i-1])-1] : '\0';
        
                if (output(argc, argv) == 1) {
                    fprintf(output_file, "%c", current_sep);
                } else {
                    printf("%c", current_sep);
                }
        
                // GESTÃO DAS ASPAS
                if (current_sep == '"') {
                    if (quote_open) {
                        if (output(argc, argv) == 1) fprintf(output_file, " ");
                        else printf(" ");
                        quote_open = 0;
                    } else {
                        if (prev_token_last_char == '.' || prev_token_last_char == '!' || prev_token_last_char == '?') {
                            if (output(argc, argv) == 1) fprintf(output_file, " ");
                            else printf(" ");
                        }
                        quote_open = 1;
                    }
                } 
                else if ((current_sep == '.' || current_sep == ',' || current_sep == '?' || current_sep == '!' || current_sep == ';' || current_sep == ':') && next_char != '"' && next_char != '.') {
                    if (output(argc, argv) == 1) fprintf(output_file, " ");
                    else printf(" ");
                }
            }
        
            free(tokens[i]);
            if (separators[i] != NULL) {
                free(separators[i]);
            }
        }
        
        // APENAS AQUI imprimes a quebra de linha final da linha original
        if (output(argc, argv) == 1) {
            fprintf(output_file, "\n");
        } else {
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

    if(dictionary_filename == NULL){
        dictionary_filename = "words";
    }

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