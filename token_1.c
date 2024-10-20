/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_1.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: albealva <albealva@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/08 18:10:59 by albealva          #+#    #+#             */
/*   Updated: 2024/10/20 17:55:16 by albealva         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void extract_tokens(const char *section, t_general *info);


const char *get_token_type_name(int type) {
    static const char *token_type_names[] = {
        "EMPTY",  // 0
        "CMD",    // 1
        "ARG",    // 2
        "TRUNC",  // 3
        "APPEND", // 4
        "INPUT",  // 5
        "FIL",    // 6
        "PIPE",   // 7
        "END"     // 8
    };

    if (type >= 0 && type <= 8) {
        return token_type_names[type];
    } else {
        return "UNKNOWN";
    }
}

// Libera la memoria ocupada por la lista de tokens
void free_tokens_list_tokenize(t_general *info) {
    t_token *current = info->tokens_list;
    t_token *next;

    while (current != NULL) {
        next = current->next;
        free(current->str);
        free(current);
        current = next;
    }
    info->tokens_list = NULL;
    info->number_of_tokens = 0;
}

void quoting_choice(t_quote_state *state, char c) {
    if (c == '"' && state->sq == 0) {  // Dentro de comillas simples
        if (state->dq == 0) {
            // No estamos dentro de comillas dobles, así que cambiamos el estado
            state->dq = 1;
        } else {
            // Estamos dentro de comillas dobles, así que cerramos
            state->dq = 0;
        }
    } else if (c == '\'' && state->dq == 0) {  // Dentro de comillas dobles
        if (state->sq == 0) {
            // No estamos dentro de comillas simples, así que cambiamos el estado
            state->sq = 1;
        } else {
            // Estamos dentro de comillas simples, así que cerramos
            state->sq = 0;
        }
    }
}


int open_quote(char *line, t_quote_state *state) {
    int i = 0;
    state->dq = 0;
    state->sq = 0;

    while (line[i] != '\0') {
        quoting_choice(state, line[i]);
        i++;
    }

    // Verificar el estado final
    if (state->dq != 0 || state->sq != 0) {
        // Hay comillas abiertas que no se han cerrado
        printf("unclosed quotes error \n");
	return 1;  // Error: comillas no cerradas
    }

    return 0;  // Sin errores
}


//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


// Extrae una sección del texto de entrada, considerando cada pipe como una sección independiente


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Función para extraer una sección basada en delimitadores, manejando comillas
char *extract_section(char **start, const char *delimiters) {
    char *end;
    char *section;
    size_t section_length;
    int in_single_quotes = 0; // Controla el estado de las comillas simples
    int in_double_quotes = 0; // Controla el estado de las comillas dobles

    // Caso especial para manejar pipes (`|`) como secciones independientes
    if (**start == '|') {
        section = malloc(2); // Espacio para la pipe y el terminador nulo
        if (!section) {
            perror("Malloc failed");
            exit(EXIT_FAILURE);
        }
        section[0] = '|';
        section[1] = '\0';

        // Mueve el puntero para saltar la pipe
        (*start)++;

        // Imprime el contenido de la sección
       // printf("Sección: %s\n", section);

        return section;
    }

    end = *start;
    while (*end && (!strchr(delimiters, *end) || in_single_quotes || in_double_quotes)) {
        if (*end == '\'') {
            in_single_quotes = !in_single_quotes; // Alterna el estado de las comillas simples
        } else if (*end == '\"') {
            in_double_quotes = !in_double_quotes; // Alterna el estado de las comillas dobles
        }
        end++;
    }

    // Calcula la longitud de la sección
    section_length = end - *start;

    // Asigna memoria para la sección
    section = malloc(section_length + 1); // +1 para el terminador nulo
    if (!section) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    // Copia la sección actual
    strncpy(section, *start, section_length);
    section[section_length] = '\0'; // Termina la cadena con '\0'

    // Mueve el puntero al final de la sección
    *start = end;

    // Imprime el contenido de la sección
//    printf("Sección: %s\n", section);

    return section;
}








/*
// Añade un token a la lista de tokens
void add_token_to_list(t_general *info, const char *str, int type) {
    t_token *new_token = malloc(sizeof(t_token));
    if (!new_token) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    new_token->str = strdup(str);
    if (!new_token->str) {
        perror("Strdup failed");
        exit(EXIT_FAILURE);
    }

    new_token->type = type;
    new_token->prev = NULL;
    new_token->next = NULL;

    if (!info->tokens_list) {
        info->tokens_list = new_token;
    } else {
        t_token *last = info->tokens_list;
        while (last->next) {
            last = last->next;
        }
        last->next = new_token;
        new_token->prev = last;
    }

    info->number_of_tokens++;
}
*/
char *append_string(char *dest, char *src) {
    if (dest == NULL || src == NULL) {
        return NULL; // Manejo de error
    }

    // Calcular la longitud de los strings
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);

    // Reasignar memoria para dest, asegurando que hay suficiente espacio
    dest = realloc(dest, dest_len + src_len + 1); // +1 para el terminador nulo
    if (dest == NULL) {
        fprintf(stderr, "Error reallocating memory\n");
        return NULL; // Manejo de error
    }

    // Añadir el contenido de src al final de dest
    strcat(dest, src);

    return dest; // Devolver el nuevo string
}


void add_token_to_list(t_general *info, const char *str, int type) {
    // Crear un nuevo token y verificar que se ha podido reservar memoria
    t_token *new_token = malloc(sizeof(t_token));
    if (!new_token) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    // Asignar la cadena al nuevo token
    new_token->str = strdup(str);
    if (!new_token->str) {
        perror("Strdup failed");
        exit(EXIT_FAILURE);
    }

    // Asignar el tipo y preparar los punteros del nuevo token
    new_token->type = type;
    new_token->prev = NULL;
    new_token->next = NULL;

    // Si la lista de tokens está vacía, el nuevo token será el primero
    if (!info->tokens_list) {
        info->tokens_list = new_token;
    } else {
        // Si no está vacía, buscar el último token y añadir el nuevo al final
        t_token *last = info->tokens_list;
        while (last->next) {
            last = last->next;
        }
        last->next = new_token;
        new_token->prev = last;
    }

    // Incrementar el contador de tokens en la estructura general
    info->number_of_tokens++;
}

// Función para añadir un carácter a un string
char *add_char_to_token2(char *token, char c) {
    //printf("soy add_char_to_token2!,\n"); 
    int len = token ? strlen(token) : 0; // Calcular longitud actual del token
    char *new_token = malloc(len + 2);      // Reservar espacio para nuevo carácter y '\0'
    if (!new_token) {
        // Manejo de error de memoria (opcional)
        return NULL;
    }
    if (token) {
        strcpy(new_token, token);           // Copiar token existente
        free(token);                        // Liberar memoria del token anterior
    }
    //if (is_marked(c))
    //    c=unmark_char(c);
    new_token[len] = c;                     // Añadir nuevo carácter
    new_token[len + 1] = '\0';              // Terminar el string con '\0'
    return new_token;
}

char *add_char_to_token(char *token, char c) {
    int len = token ? strlen(token) : 0; // Calcular longitud actual del token
    char *new_token = malloc(len + 2);      // Reservar espacio para nuevo carácter y '\0'
    //printf("soy add_char_to_token1!,\n"); 
    if (!new_token) {
        // Manejo de error de memoria (opcional)
        return NULL;
    }
    if (token) {
        strcpy(new_token, token);           // Copiar token existente
        free(token);                        // Liberar memoria del token anterior
    }
    if (is_marked(c))
    {
        //printf("estan marcada!,\n");
        c=unmark_char(c);
    }
    new_token[len] = c;                     // Añadir nuevo carácter
    new_token[len + 1] = '\0';              // Terminar el string con '\0'
    return new_token;
}
// Función para verificar si un carácter es un espacio o tabulador
int ft_isspace(char c) {
    return (c == ' ' || c == '\t' || c== '\r');
}





bool should_expand(bool in_single_quotes) {
    // Si estamos dentro de comillas simples, no expandir
    return !in_single_quotes;
}

typedef enum {
    NONE,
    SINGLE_QUOTE,
    DOUBLE_QUOTE
} QuoteState;


/*
char *get_env_var(const char *var_name) {
   // printf("get_env_var called with: %s\n", var_name); // Imprimir el nombre de la variable recibida
   // fflush(stdout); // Asegúrate de que se imprime inmediatamente
    char *value = getenv(var_name);
   // printf("get_env_var returning: %s\n", value ? value : "espacio"); // Imprimir el valor devuelto o "NULL"
   // fflush(stdout); // Asegúrate de que se imprime inmediatamente
    return value ? value : "";  // Si la variable no está definida, devuelve una cadena vacía.
}
*/

char *get_env_var(t_general *info, const char *var_name) {
    int i = 0;  // Índice inicial
    while (info->env[i] != NULL) {  // Continúa hasta encontrar NULL
        // Busca el signo igual en la cadena actual
        char *equal_sign = strchr(info->env[i], '=');  // Busca el signo igual
        if (equal_sign != NULL) {
            // Compara el nombre de la variable con var_name
            size_t name_length = equal_sign - info->env[i];  // Longitud del nombre
            if (strncmp(info->env[i], var_name, name_length) == 0 && name_length == strlen(var_name)) {
                return equal_sign + 1;  // Devuelve el valor (después del signo igual)
            }
        }
        i++;  // Avanza al siguiente elemento
    }
    return "";  // Si no se encuentra la variable, devuelve una cadena vacía
}

int is_special_separator(char c) {
    return c == ' ' || c == '\t' || c == '\'' || c == '\"' || c == '<' || c == '>' || c == '|' || c == '$';
}





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

// Variables globales
int last_exit_status = 0; // Variable global para almacenar el último código de salida

char* itoa(int value, char* str, int base) {
    static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* p = str;
    int num = value;

    if (base < 2 || base > 36) {
        *p = '\0';
        return NULL;
    }

    if (num < 0) {
        *p++ = '-';
        num = -num;
    }

    char temp[33];
    char* q = temp;

    do {
        *q++ = digits[num % base];
        num /= base;
    } while (num);

    while (q > temp) {
        *p++ = *--q;
    }
    *p = '\0';

    return str;
}

// Función para establecer el último código de salida
void set_last_exit_status(int status) {
    last_exit_status = status;
}

// Función para expandir "$?"
char* expand_dollar_question(t_general *info) {
    char* result = (char*)malloc(12); // Asegúrate de tener suficiente espacio
    if (result == NULL) {
        return NULL;
    }

    itoa(info->exit_status, result, 10);

    return result;
}

// Función para ejecutar un comando y actualizar el código de salida
void execute_command(const char* command) {
    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo
        execlp(command, command, NULL);
        // Si execlp falla, exit con un código de salida 1
        exit(127); // Código de salida para comando no encontrado
    } else if (pid > 0) {
        // Proceso padre
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            set_last_exit_status(WEXITSTATUS(status));
        } else {
            set_last_exit_status(1); // En caso de error inesperado
        }
    } else {
        perror("fork failed");
        exit(1);
    }
}

// Función para expandir variables




int count_dollars(const char *section) {
    int count = 0;
    for (int i = 0; section[i] != '\0'; i++) {
        if (section[i] == '$') {
            count++;
        }
    }
    return count;
}

void print_start_pos(int *start_pos) {
//    printf("Contenido de start_pos: ");
    int i = 0;
    while (start_pos[i]) {
  //      printf("%d ", start_pos[i]);
        i++;
    }
   // printf("\n");
}




int reset_positions(int *start_pos, int size_malloc) {
    int k;
    if(start_pos == NULL) {
        return (-1);
    }
    for(k = 0; k < size_malloc; k++){
            start_pos[k] = -1; // Inicializar todos los valores en start_pos a -1
        }
    return (0);
}



char	mark_char(char c)
{
	return (c | 0x80);
}

char	unmark_char(char c)
{
	return (c & 0x7F);
}

int	is_marked(char c)
{
	//return c & 0x80;
    return (c & 0x80) != 0;
}




char *expand_variable2(const char *input, int start_pos, t_general *info) {
    char *result = NULL;
    int len = strlen(input);
    char temp[1024] = {0};  // Buffer temporal para construir el resultado expandido
    char var_name[256] = {0};
    int temp_index = 0;  // Índice para el buffer temporal
    int var_index = 0;
    int expanded = 0;  // Bandera para saber si ya hemos expandido una variable

    // Copia los caracteres iniciales antes de start_pos
    for (int j = 0; j < start_pos - 1; j++) {
        temp[temp_index++] = input[j];
    }
    if (start_pos == 0) {
        temp[temp_index++] = input[0];
    }

    for (int i = start_pos - 1; i < len; i++) {
        // Expandir solo a partir de start_pos y solo una vez
        if (input[i] == '$' && input[i + 1] == '?' && !expanded) {
            i += 1;  // Avanza para saltarse el '?'
            // Expande "$?"
            char* exit_status_str = expand_dollar_question(info);
            if (exit_status_str != NULL) {
                strcpy(temp + temp_index, exit_status_str);
                temp_index += strlen(exit_status_str);
                free(exit_status_str);
            }
            expanded = 1;  // Marca que ya hemos expandido una variable
            continue;
        }
        
        if (input[i] == '$' && !expanded) {
            i++;  // Avanza para saltarse el '$'
            var_index = 0;
            // Captura el nombre de la variable de entorno
            while (i < len && !is_special_separator(input[i]) && (isalnum(input[i]) || input[i] == '_')) {
                var_name[var_index++] = input[i++];
            }
            i--;  // Retrocede un carácter porque el bucle ha avanzado uno de más.
            var_name[var_index] = '\0';

            // Obtiene el valor de la variable de entorno
            char *value = get_env_var(info, var_name);
            if (value) {
                // Marca comillas dentro del valor de la variable
                for (int k = 0; value[k] != '\0'; k++) {
                    if (value[k] == '"' || value[k] == '\'' || value[k] == '$') {
                        temp[temp_index++] = mark_char(value[k]);  // Marca comillas
                    } else {
                        temp[temp_index++] = value[k];  // Copia el carácter normalmente
                    }
                }
            }
            expanded = 1;  // Marca que ya hemos expandido una variable
        } else {
            // Copia el contenido literal al buffer temporal
            temp[temp_index++] = input[i];
        }
    }

    // Duplica el resultado expandido
    result = strdup(temp);
    return result;
}


char *expand_variable(const char *input, int start_pos,t_general *info) {
   // printf("expand_variable called with input: %s, start_pos: %d\n", input, start_pos);
   // fflush(stdout);

    char *result = NULL;
    int len = strlen(input);
    char temp[1024] = {0};  // Buffer temporal para construir el resultado expandido
    char var_name[256] = {0};
    int temp_index = 0;  // Índice para el buffer temporal
    int var_index = 0;
    int expanded = 0;  // Bandera para saber si ya hemos expandido una variable

    // Copia los caracteres iniciales antes de start_pos
    for (int j = 0; j < start_pos - 1; j++) {
        temp[temp_index++] = input[j];
    }
    if (start_pos == 0) {
        temp[temp_index++] = input[0];
    }
    for (int i = start_pos - 1; i < len; i++) {
        // Expandir solo a partir de start_pos y solo una vez
        if (input[i] == '$' && input[i + 1] == '?' && !expanded) {
            i += 1;  // Avanza para saltarse el '?'
            // Expande "$?"
            char* exit_status_str = expand_dollar_question(info);
            if (exit_status_str != NULL) {
                strcpy(temp + temp_index, exit_status_str);
                temp_index += strlen(exit_status_str);
                free(exit_status_str);
            }
            expanded = 1;  // Marca que ya hemos expandido una variable
            //result = strdup(temp);
            //return result;
            i++;  // Avanza para saltarse el '?'
        }
        if (input[i] == '$' && !expanded) {
            i++;  // Avanza para saltarse el '$'
            var_index = 0;
            // Captura el nombre de la variable de entorno
            while (i < len && !is_special_separator(input[i]) && (isalnum(input[i]) || input[i] == '_')) {
                var_name[var_index++] = input[i++];
            }
            i--;  // Retrocede un carácter porque el bucle ha avanzado uno de más.
            var_name[var_index] = '\0';

            // Obtiene el valor de la variable de entorno
            char *value = get_env_var(info,var_name);
            if (value) {
                strcpy(temp + temp_index, value);
                temp_index += strlen(value);
            }
            expanded = 1;  // Marca que ya hemos expandido una variable
        } else {
            // Copia el contenido literal al buffer temporal
            //if(input[i] == '?') {
                //i += 1;  // Avanza para saltarse el '?'
            //}
            temp[temp_index++] = input[i];
        }
    }

    // Duplica el resultado expandido
    result = strdup(temp);
   // printf("expand_variable returning: %s\n", result);
   // fflush(stdout);
    return result;
}

int calculate_length_difference(const char *input, int start_pos,t_general *info) {
   // printf("calculate_length_difference called with input: %s, start_pos: %d\n", input, start_pos);
   // fflush(stdout);

    int original_len = strlen(input);
    char temp[1024] = {0};  // Buffer temporal para construir el resultado expandido
    char var_name[256] = {0};
    int temp_index = 0;  // Índice para el buffer temporal
    int var_index = 0;
    int expanded = 0;  // Bandera para saber si ya hemos expandido una variable

    // Copia los caracteres iniciales antes de start_pos
    for (int j = 0; j < start_pos - 1; j++) {
        temp[temp_index++] = input[j];
    }
    if (start_pos == 0) {
        temp[temp_index++] = input[0];
    }
    for (int i = start_pos - 1; i < original_len; i++) {
        // Expandir solo a partir de start_pos y solo una vez
        if (input[i] == '$' && input[i + 1] == '?' && !expanded) {
            i += 1;  // Avanza para saltarse el '?'
            // Expande "$?"
            char* exit_status_str = expand_dollar_question(info);
            if (exit_status_str != NULL) {
                strcpy(temp + temp_index, exit_status_str);
                temp_index += strlen(exit_status_str);
                free(exit_status_str);
            }
            expanded = 1;  // Marca que ya hemos expandido una variable
            i++;  // Avanza para saltarse el '?'
        }
        if (input[i] == '$' && !expanded) {
            i++;  // Avanza para saltarse el '$'
            var_index = 0;
            // Captura el nombre de la variable de entorno
            while (i < original_len && !is_special_separator(input[i]) && (isalnum(input[i]) || input[i] == '_')) {
                var_name[var_index++] = input[i++];
            }
            i--;  // Retrocede un carácter porque el bucle ha avanzado uno de más.
            var_name[var_index] = '\0';

            // Obtiene el valor de la variable de entorno
            char *value = get_env_var(info,var_name);
            if (value) {
                strcpy(temp + temp_index, value);
                temp_index += strlen(value);
            }
            expanded = 1;  // Marca que ya hemos expandido una variable
        } else {
            // Copia el contenido literal al buffer temporal
            //if(input[i] == '?') {
                //i += 1;  // Avanza para saltarse el '?'
            //}
            temp[temp_index++] = input[i];
        }
    }

    int expanded_len = strlen(temp);  // Longitud del texto expandido
    int length_difference = expanded_len - original_len;

   // printf("calculate_length_difference returning: %d\n", length_difference);
   // fflush(stdout);

    return length_difference;
}








// Función para buscar un comando en los directorios de PATH
int is_valid_command(const char *command, char **env) {
    (void)env;
    char *path = getenv("PATH");  // Obtener el PATH del entorno
    if (!path) {
        fprintf(stderr, "Error: PATH variable not found.\n");
        return 0;
    }

    char *path_copy = strdup(path);  // Hacemos una copia de PATH para no modificarlo
    char *dir = strtok(path_copy, ":");  // Dividimos PATH por ":"

    // Creamos un buffer para las rutas completas de los comandos
    char full_path[512];

    while (dir != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);  // Construimos la ruta completa

        if (access(full_path, X_OK) == 0) {  // Verificamos si el comando es ejecutable
            free(path_copy);
            return 1;  // El comando existe y es ejecutable
        }

        dir = strtok(NULL, ":");  // Siguiente directorio en PATH
    }

    free(path_copy);
    return 0;  // El comando no se encontró
}

// Función para verificar el primer token de la lista

int check_syntax_errors(t_general *info) {
    t_token *current_token = info->tokens_list;
    int last_type = 0;
    int error_found = 0;

    // Verificar el primer token
    if (current_token && current_token->type == CMD) {
        if (!is_valid_command(current_token->str, info->env)) {
            fprintf(stderr, "Error: Command '%s' not found.\n", current_token->str);
            return 1;  // Error de sintaxis
        }
        last_type = CMD; // Actualiza el tipo del último token
    } else {
        fprintf(stderr, "Error: No command found.\n");
        return 1;  // Error de sintaxis
    }

    // Avanzar al siguiente token
    current_token = current_token->next;

    while (current_token != NULL) {
        // Verifica el tipo de token actual
        if (current_token->type == CMD) {
            if (!is_valid_command(current_token->str, info->env)) {
                fprintf(stderr, "Error: Command '%s' not found.\n", current_token->str);
                return 1;  // Error de sintaxis
            }
            last_type = CMD; // Actualiza el tipo del último token
        } else if (current_token->type == ARG) {
            // Argumento puede seguir a un comando, otro argumento, o un operador de redirección
            if (last_type == PIPE) {
                fprintf(stderr, "Error: Argument '%s' in invalid position after pipe.\n", current_token->str);
                error_found = 1;  // Error de sintaxis
                break;
            }
            last_type = ARG; // Actualiza el tipo del último token
        } else if (current_token->type == TRUNC || current_token->type == APPEND) {
            // Redirección debe seguir a un comando o argumento
            if (last_type != CMD && last_type != ARG) {
                fprintf(stderr, "Error: Redirection operator '%d' in invalid position.\n", current_token->type);
                error_found = 1;  // Error de sintaxis
                break;
            }
            // Verifica que el siguiente token sea un archivo
            if (current_token->next == NULL || current_token->next->type != FIL) {
                fprintf(stderr, "Error: No file specified for redirection '%d'.\n", current_token->type);
                error_found = 1;  // Error de sintaxis
                break;
            }
            // Actualiza last_type después de procesar el archivo
            last_type = FIL;
            current_token = current_token->next; // Avanza al siguiente token
        } else if (current_token->type == INPUT) {
            // Redirección de entrada debe seguir a un comando o argumento
            if (last_type != CMD && last_type != ARG) {
                fprintf(stderr, "Error: Input redirection operator '%d' in invalid position.\n", current_token->type);
                error_found = 1;  // Error de sintaxis
                break;
            }
            // Verifica que el siguiente token sea un archivo
            if (current_token->next == NULL || current_token->next->type != FIL) {
                fprintf(stderr, "Error: No file specified for input redirection.\n");
                error_found = 1;  // Error de sintaxis
                break;
            }
            // Actualiza last_type después de procesar el archivo
            last_type = FIL;
            current_token = current_token->next; // Avanza al siguiente token
        } else if (current_token->type == PIPE) {
            // La tubería debe seguir a un comando o argumento
            if (last_type == PIPE || last_type == TRUNC || last_type == APPEND || last_type == INPUT) {
                fprintf(stderr, "Error: Pipe operator in invalid position.\n");
                error_found = 1;  // Error de sintaxis
                break;
            }
            last_type = PIPE; // Actualiza el tipo del último token
        } else {
            fprintf(stderr, "Error: Unknown token type '%d'.\n", current_token->type);
            error_found = 1;  // Error de sintaxis
            break;
        }

        // Avanza al siguiente token
        current_token = current_token->next;
    }

    // Verifica que el último token no sea un operador sin un archivo o comando siguiente
    if (!error_found && (last_type == TRUNC || last_type == APPEND || last_type == INPUT || last_type == PIPE)) {
        fprintf(stderr, "Error: Command ends with an invalid operator.\n");
        error_found = 1;  // Error de sintaxis
    }

    return error_found ? 1 : 0;  // Retorna 1 si hubo un error, 0 si no
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void process_section(char *section, t_general *info) {
    if (section) {
        //printf("Sección: %s\n", section);
        extract_tokens(section, info);
        free(section);
    }
}


// Tokeniza la entrada dividiéndola en secciones y luego en tokens
// Tokeniza la entrada dividiéndola en secciones y luego en tokens
t_token *tokenize_input(t_general *info, char *input) {
    t_quote_state state;
    open_quote(input, &state); // Verifica las comillas antes de tokenizar

    char *start = input;
    const char *section_delimiters = "|\n\r";

    info->tokens_list = NULL;
    //printf("start: %s\n", start);
    while (*start) {
        char *section = extract_section(&start, section_delimiters);
        if (section) {
            process_section(section, info);
        }
    }

    return info->tokens_list;
}

char *append_to_current_section(char *current_section, const char *token) {
    if (current_section == NULL) {
        current_section = strdup(token); // Inicializa con el primer token
    } else {
        char *temp = malloc(strlen(current_section) + strlen(token) + 2); // +2 por el espacio y el terminador nulo
        if (temp == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            exit(EXIT_FAILURE);
        }
        sprintf(temp, "%s %s", current_section, token); // Concatenar con un espacio
        free(current_section); // Liberar la sección anterior
        current_section = temp; // Asignar la nueva sección
    }
    return current_section; // Devolver la nueva cadena concatenada
}




/*

void previo_extract_tokens(const char *section, t_general *info) {
    char *current_token = NULL;  // Única variable para construcción de tokens
    int i = 0;
    int is_first_token = 1;      // Indicador de primer token en la sección
    int expect_file = 0;         // Indicador para identificar si el próximo token es un FILE
    int in_single_quotes = 0;    // Controla el estado de las comillas simples
    int in_double_quotes = 0;    // Controla el estado de las comillas dobles
    QuoteState quote_state = NONE; // Inicialización de la variable de estado
    int *start_pos = NULL;
    int size_malloc;
    int j = 0;
    int z = 0;
    int length_difference = 0;
    int h=0;
    char *current_section = NULL;
    //size_t pos_ini_8 = 8;  // Posición inicial fija para pruebas

    if (section && count_dollars(section) > 0) {
        size_malloc = count_dollars(section);
        start_pos = malloc(size_malloc * sizeof(int)); // size_t
        if (start_pos == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            exit(EXIT_FAILURE);
        }
        for (int k = 0; k < size_malloc; k++) {
            start_pos[k] = -1;
        }
        //print_start_pos(start_pos);
    }

    
    while (section[i] != '\0') {

         // Manejo del salto de línea ('\n')
    if (section[i] == '\n' && !in_single_quotes && !in_double_quotes) {
        if (current_token) {
            if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
            add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));
            current_section = append_to_current_section(current_section, current_token);
            free(current_token);
            current_token = NULL;
            j = reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        is_first_token = 0;
        expect_file = 0;
        i++;
        continue;  // Saltar el salto de línea para continuar
    }
        
        // Manejo de comillas dobles
        if (section[i] == '\"') {
            if (!in_single_quotes) {        
                in_double_quotes = !in_double_quotes;
                if (in_double_quotes) {
                    quote_state = DOUBLE_QUOTE;
                }
                // No cambiar `quote_state` al cerrar comillas dobles
            } else {
                current_token = add_char_to_token(current_token, section[i]);
                if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
                
            }
            i++;
            continue;
        }

        // Manejo de comillas simples
        if (section[i] == '\'') {
            if (!in_double_quotes) {
                in_single_quotes = !in_single_quotes;
                if (in_single_quotes) {
                    quote_state = SINGLE_QUOTE;
                }
                // No cambiar `quote_state` al cerrar comillas simples
            } else {
                current_token = add_char_to_token(current_token, section[i]);
                    if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
                
                
            }
            i++;
            continue;
        }

        // Manejo de >> como un token individual
        if (section[i] == '>' && section[i + 1] == '>' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
        
                add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            add_token_to_list(info, ">>", APPEND);
            current_section = append_to_current_section(current_section, ">>");
            free(current_token);
            i++;
            expect_file = 1;
            is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        // Manejo de > como token individual
        else if (section[i] == '>' && section[i + 1] != '>' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
                add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            add_token_to_list(info, ">", TRUNC);
            current_section = append_to_current_section(current_section, ">");
            free(current_token);
            expect_file = 1;
            is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        // Manejo de < como token individual
        else if (section[i] == '<' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
        if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
                add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            add_token_to_list(info, "<", INPUT);
            current_section = append_to_current_section(current_section, "<");
            free(current_token);
            expect_file = 1;
            is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        // Manejo de | solo si no estamos dentro de comillas
        else if (section[i] == '|' && !in_single_quotes && !in_double_quotes) {
            is_first_token = 1;
            if (current_token) {
        if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
       	             }
                    z++;
                    h = 0;
        }
        z = 0;
    }
                add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            while (section[i] == '|') {
                add_token_to_list(info, "|", PIPE);
                current_section = append_to_current_section(current_section, "|");
                free(current_token);
                i++;
                is_first_token = 1;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            continue;
        }
        // Manejo de espacios o tabuladores si no estamos dentro de comillas
        else if (ft_isspace(section[i]) && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
       // if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
          if (!in_single_quotes && count_dollars(section)) {    
		    while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
                add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state=NONE;
                is_first_token = 0;
            }
            
                //is_first_token = 0;
        
            //is_first_token = 0;
            //expect_file = 0;
        }
        else {
            // Añadir carácter a current_token
                current_token = add_char_to_token(current_token, section[i]);
                if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
            
        }

        i++;
    }
    //if (count_dollars(section))
        //print_start_pos(start_pos);
    // Manejo final del token si existe
if (current_token) {
    // Si hay variables de entorno a expandir y no estamos en comillas simples
       // if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
       if (!in_single_quotes && count_dollars(section)) {
		while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
    
    // Añadir el último token a la lista
    add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));
    current_section = append_to_current_section(current_section, current_token);

    // Liberar la memoria de current_token
    free(current_token);

    // Resetear posiciones
    j = reset_positions(start_pos, size_malloc);
    j = 0;
    quote_state = NONE;
    is_first_token = 0;
}

// Aquí puedes liberar start_pos si fue utilizado
if (count_dollars(section))
    free(start_pos);

printf("Tokens concatenados: %s\n", current_section);    
}
*/

char *extract_current_section(const char *section, t_general *info) {
    char *current_token = NULL;  // Única variable para construcción de tokens
    int i = 0;
    //int is_first_token = 1;      // Indicador de primer token en la sección
    //int expect_file = 0;         // Indicador para identificar si el próximo token es un FILE
    int in_single_quotes = 0;    // Controla el estado de las comillas simples
    int in_double_quotes = 0;    // Controla el estado de las comillas dobles
    QuoteState quote_state = NONE; // Inicialización de la variable de estado
    int *start_pos = NULL;
    int size_malloc;
    int j = 0;
    int z = 0;
    int length_difference = 0;
    int h=0;
    char *current_section = NULL;
    //size_t pos_ini_8 = 8;  // Posición inicial fija para pruebas

    if (section && count_dollars(section) > 0) {
        size_malloc = count_dollars(section);
        start_pos = malloc(size_malloc * sizeof(int)); // size_t
        if (start_pos == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            exit(EXIT_FAILURE);
        }
        for (int k = 0; k < size_malloc; k++) {
            start_pos[k] = -1;
        }
        //print_start_pos(start_pos);
    }

    
    while (section[i] != '\0') {

         // Manejo del salto de línea ('\n')
    if (section[i] == '\n' && !in_single_quotes && !in_double_quotes) {
        if (current_token) {
            if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
            //add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));
            current_section = append_to_current_section(current_section, current_token);
            free(current_token);
            current_token = NULL;
            j = reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        //is_first_token = 0;
        //expect_file = 0;
        i++;
        continue;  // Saltar el salto de línea para continuar
    }
        
        // Manejo de comillas dobles
        if (section[i] == '\"') {
            if (!in_single_quotes) {        
                in_double_quotes = !in_double_quotes;
                if (in_double_quotes) {
                    quote_state = DOUBLE_QUOTE;
                }
                current_token = add_char_to_token2(current_token, section[i]);
                // No cambiar `quote_state` al cerrar comillas dobles
            } else {
                current_token = add_char_to_token2(current_token, section[i]);
                if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
                
            }
            i++;
            continue;
        }

        // Manejo de comillas simples
        if (section[i] == '\'') {
            if (!in_double_quotes) {
                in_single_quotes = !in_single_quotes;
                if (in_single_quotes) {
                    quote_state = SINGLE_QUOTE;
                }
                current_token = add_char_to_token2(current_token, section[i]);
                // No cambiar `quote_state` al cerrar comillas simples
            } else {
                current_token = add_char_to_token2(current_token, section[i]);
                    if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
                
                
            }
            i++;
            continue;
        }

        // Manejo de >> como un token individual
        if (section[i] == '>' && section[i + 1] == '>' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
        
                //add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            //add_token_to_list(info, ">>", APPEND);
            current_section = append_to_current_section(current_section, ">>");
            free(current_token);
            i++;
            //expect_file = 1;
            //is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        // Manejo de > como token individual
        else if (section[i] == '>' && section[i + 1] != '>' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
                //add_to_list(info, current_token, is_first_token ? CMD : ARG);
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            //add_token_to_list(info, ">", TRUNC);
            current_section = append_to_current_section(current_section, ">");
            free(current_token);
            //expect_file = 1;
            //is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        // Manejo de < como token individual
        else if (section[i] == '<' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
        if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
                //add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            //add_token_to_list(info, "<", INPUT);
            current_section = append_to_current_section(current_section, "<");
            free(current_token);
            //expect_file = 1;
            //is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        // Manejo de | solo si no estamos dentro de comillas
        else if (section[i] == '|' && !in_single_quotes && !in_double_quotes) {
            //is_first_token = 1;
            if (current_token) {
        if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
       	             }
                    z++;
                    h = 0;
        }
        z = 0;
    }
                //add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            while (section[i] == '|') {
                //add_token_to_list(info, "|", PIPE);
                current_section = append_to_current_section(current_section, "|");
                free(current_token);
                i++;
                //is_first_token = 1;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state = NONE;
            }
            continue;
        }
        // Manejo de espacios o tabuladores si no estamos dentro de comillas
        else if (ft_isspace(section[i]) && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
       // if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
          if (!in_single_quotes && count_dollars(section)) {    
		    while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
                //add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));
                current_section = append_to_current_section(current_section, current_token);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                quote_state=NONE;
                //is_first_token = 0;
            }
            
                //is_first_token = 0;
        
            //is_first_token = 0;
            //expect_file = 0;
        }
        else {
            // Añadir carácter a current_token
                current_token = add_char_to_token2(current_token, section[i]);
                if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
            
        }

        i++;
    }
    //if (count_dollars(section))
        //print_start_pos(start_pos);
    // Manejo final del token si existe
if (current_token) {
    // Si hay variables de entorno a expandir y no estamos en comillas simples
       // if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
       if (!in_single_quotes && count_dollars(section)) {
		while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
    
    // Añadir el último token a la lista
    //add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));
    current_section = append_to_current_section(current_section, current_token);

    // Liberar la memoria de current_token
    free(current_token);

    // Resetear posiciones
    j = reset_positions(start_pos, size_malloc);
    j = 0;
    quote_state = NONE;
    //irst_token = 0;
}

// Aquí puedes liberar start_pos si fue utilizado
if (count_dollars(section))
    free(start_pos);


return current_section;    
}
//printf("Tokens concatenados2: %s\n", current_section);


void extract_tokens(const char *section, t_general *info) {
    char *current_token = NULL;  // Única variable para construcción de tokens
    int i = 0;
    int is_first_token = 1;      // Indicador de primer token en la sección
    int expect_file = 0;         // Indicador para identificar si el próximo token es un FILE
    int in_single_quotes = 0;    // Controla el estado de las comillas simples
    int in_double_quotes = 0;    // Controla el estado de las comillas dobles
    //QuoteState quote_state = NONE; // Inicialización de la variable de estado
    int *start_pos = NULL;
    int size_malloc;
    int j = 0;
    //int z = 0;
    //int length_difference = 0;
    //int h=0;
    //size_t pos_ini_8 = 8;  // Posición inicial fija para pruebas

    if (section && count_dollars(section) > 0) {
        size_malloc = count_dollars(section);
        start_pos = malloc(size_malloc * sizeof(int)); // size_t
        if (start_pos == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            exit(EXIT_FAILURE);
        }
        for (int k = 0; k < size_malloc; k++) {
            start_pos[k] = -1;
        }
        //print_start_pos(start_pos);
    }

    
    while (section[i] != '\0') {

         // Manejo del salto de línea ('\n')
    /*
    if (section[i] == '\n' && !in_single_quotes && !in_double_quotes) {
        if (current_token) {
            if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
            add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));
            free(current_token);
            current_token = NULL;
            j = reset_positions(start_pos, size_malloc);
            j = 0;
            quote_state = NONE;
        }
        is_first_token = 0;
        expect_file = 0;
        i++;
        continue;  // Saltar el salto de línea para continuar
    }
      */  
        // Manejo de comillas dobles
        if (section[i] == '\"') {
            if (!in_single_quotes) {        
                in_double_quotes = !in_double_quotes;
                if (in_double_quotes) {
                    //quote_state = DOUBLE_QUOTE;
                }
                // No cambiar `quote_state` al cerrar comillas dobles
            } else {
                current_token = add_char_to_token(current_token, section[i]);
                if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
                
            }
            i++;
            continue;
        }

        // Manejo de comillas simples
        if (section[i] == '\'') {
            if (!in_double_quotes) {
                in_single_quotes = !in_single_quotes;
                if (in_single_quotes) {
                    //quote_state = SINGLE_QUOTE;
                }
                // No cambiar `quote_state` al cerrar comillas simples
            } else {
                current_token = add_char_to_token(current_token, section[i]);
                    if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
                
                
            }
            i++;
            continue;
        }

        // Manejo de >> como un token individual
        if (section[i] == '>' && section[i + 1] == '>' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
        
                add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                //quote_state = NONE;
            }
            add_token_to_list(info, ">>", APPEND);
            free(current_token);
            i++;
            expect_file = 1;
            is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            //quote_state = NONE;
        }
        // Manejo de > como token individual
        else if (section[i] == '>' && section[i + 1] != '>' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
                add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                //quote_state = NONE;
            }
            add_token_to_list(info, ">", TRUNC);
            free(current_token);
            expect_file = 1;
            is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            //quote_state = NONE;
        }
        // Manejo de < como token individual
        else if (section[i] == '<' && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
        /*
        if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
    */
                add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                //quote_state = NONE;
            }
            add_token_to_list(info, "<", INPUT);
            free(current_token);
            expect_file = 1;
            is_first_token = 0;
            j=reset_positions(start_pos, size_malloc);
            j = 0;
            //quote_state = NONE;
        }
        // Manejo de | solo si no estamos dentro de comillas
        else if (section[i] == '|' && !in_single_quotes && !in_double_quotes) {
            is_first_token = 1;
            if (current_token) {
        /*
        if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
                while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
       	             }
                    z++;
                    h = 0;
        }
        z = 0;
    }
    */
                add_token_to_list(info, current_token, is_first_token ? CMD : ARG);
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                //quote_state = NONE;
            }
            while (section[i] == '|') {
                add_token_to_list(info, "|", PIPE);
                free(current_token);
                i++;
                is_first_token = 1;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                //quote_state = NONE;
            }
            continue;
        }
        // Manejo de espacios o tabuladores si no estamos dentro de comillas
        else if (ft_isspace(section[i]) && !in_single_quotes && !in_double_quotes) {
            if (current_token) {
       // if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
          /*
          if (!in_single_quotes && count_dollars(section)) {    
		    while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
    */
                add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));
                free(current_token);
                current_token = NULL;
                j=reset_positions(start_pos, size_malloc);
                j = 0;
                //quote_state=NONE;
                is_first_token = 0;
            }
            
                //is_first_token = 0;
        
            //is_first_token = 0;
            //expect_file = 0;
        }
        else {
            // Añadir carácter a current_token
                current_token = add_char_to_token(current_token, section[i]);
                if(section[i] == '$' && !in_single_quotes && current_token) {
                    start_pos[j] = strlen(current_token);
                    j++;
                }
            
        }

        i++;
    }
    //if (count_dollars(section))
        //print_start_pos(start_pos);
    // Manejo final del token si existe
if (current_token) {
    // Si hay variables de entorno a expandir y no estamos en comillas simples
       // if (quote_state != SINGLE_QUOTE && count_dollars(section)) {
       /*
       
       if (!in_single_quotes && count_dollars(section)) {
		while (z < size_malloc && start_pos[z] != -1) {
                    length_difference = calculate_length_difference(current_token, start_pos[z],info);
                    current_token = expand_variable2(current_token, start_pos[z],info);
                    while (h < size_malloc && start_pos[h] != -1) {
                        if(start_pos[h] + length_difference >= 0) {
                            start_pos[h] += length_difference;
                        }
                        
                        h++;
                    }
                    z++;
                    h = 0;
        }
        z = 0;
    }
    */
    // Añadir el último token a la lista
    add_token_to_list(info, current_token, is_first_token ? CMD : (expect_file ? FIL : ARG));

    // Liberar la memoria de current_token
    free(current_token);

    // Resetear posiciones
    j = reset_positions(start_pos, size_malloc);
    j = 0;
    //quote_state = NONE;
    is_first_token = 0;
}

// Aquí puedes liberar start_pos si fue utilizado
if (count_dollars(section))
    free(start_pos);
}

