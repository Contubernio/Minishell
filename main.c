/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: albealva <albealva@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/17 16:26:16 by mgimon-c          #+#    #+#             */
/*   Updated: 2024/10/20 17:49:01 by albealva         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Inicializa la estructura general
int init_general(t_general *info, char **env) {
    info->number_of_tokens = 0;
    info->tokens_list = NULL;
    info->env = env;
    return 0;
}

// Controlador de señal para SIGINT (Ctrl+C)
void sigint_handler(int signo) {
    if (signo == SIGINT) {
        write(STDOUT_FILENO, "\n", 1);
    }
}

// Configura los manejadores de señales
void setup_signals(void) {
    signal(SIGINT, sigint_handler); // Manejo de Ctrl+C
}

// Inicializa el historial de comandos
void init_history(const char *history_file) {
    using_history(); // Habilita el uso del historial
    if (access(history_file, F_OK) == 0) {
        read_history(history_file);
    }
}

// Guarda el historial al final del programa
void cleanup(const char *history_file) {
    write_history(history_file);
}

// Maneja comandos especiales como 'print_on' y 'print_off'
void handle_special_commands(const char *input, int *print_mode) {
    if (strcmp(input, "print_on") == 0) {
        *print_mode = 1;
        printf("Print mode enabled\n");
    } else if (strcmp(input, "print_off") == 0) {
        *print_mode = 0;
        printf("Print mode disabled\n");
    }
}

// Maneja la entrada del usuario y añade comandos al historial
void handle_input(t_general *info, char *input, int *print_mode) {
    add_history(input);
    handle_special_commands(input, print_mode);
//int i = 1;
    // Tokenizar la entrada
while (1) {
    
    //printf("%dpaso por aqui\n",i++);
    char *new_input = extract_current_section(input, info); // Llapa a la función una vez

    // Compara el nuevo resultado con el anterior
    if (strcmp(input, new_input) == 0) {
        free(new_input); // Libera la memoria del nuevo input
        break; // Sale del bucle si no hay cambios
    }

    //free(input); // Libera el antiguo input
    input = new_input; // Actualiza input con el nuevo resultado
}

    //printf("Input: %s\n", input);
    t_token *token_list = tokenize_input(info, input);

    // Verificar si se generaron tokens
    if (token_list == NULL) {
        free(input);
        return;
    }

    // Imprimir tokens si el modo de impresión está habilitado
    if (*print_mode) {
        t_token *current = token_list;
        while (current) {
            printf("Token: %s, Type: %s\n", current->str, get_token_type_name(current->type));
            current = current->next;
        }
        printf("Total tokens: %d\n", info->number_of_tokens);
    }

    //if (check_syntax_errors(info)) {
    // Si hay errores de sintaxis, puedes manejar el error aquí.
    //fprintf(stderr, "Error de sintaxis encontrado. Terminando ejecución.\n");

    // Limpiar la lista de tokens y liberar la entrada
    //free_tokens_list_tokenize(info);
    //free(input);
    return;  // Terminar la función si hay error
//}
    
    // Limpiar la lista de tokens y liberar la entrada
    //free_tokens_list_tokenize(info);
    //free(input);
}

void print_tokens_list_alb(t_general *info) {
    t_token *current = info->tokens_list;
    int i = 1;

    if (!current) {
        printf("La lista de tokens está vacía.\n");
        return;
    }

    printf("Número de tokens: %d\n", info->number_of_tokens);

    while (current != NULL) {
        printf("Token %d:\n", i);
        printf("  - Valor: %s\n", current->str ? current->str : "(null)");
        printf("  - Tipo: %d\n", current->type);
        //printf("  - Es el primer token: %d\n", current->is_first);
        //printf("  - Expansión: %d\n", current->expansion);
        printf("  - Anterior: %s\n", current->prev ? current->prev->str : "(null)");
        printf("  - Siguiente: %s\n\n", current->next ? current->next->str : "(null)");

        current = current->next;
        i++;
    }
}

int main(int argc, char **argv, char **env) {
    char *input;
    t_general info;
    const char *history_file = ".minishell_history";
    int print_mode = 1;
    t_quote_state state;

    (void)argc;
    (void)argv;

    init_general(&info, env);
    setup_signals();
    init_history(history_file);
    set_paths_and_env(&info, env);
    while (1) {
        input = readline("mini🐚> ");
        if (!input) {
            printf("\nExit\n");
            break;
        }

        // Verificar comillas no cerradas antes de manejar la entrada
        if (open_quote(input, &state) != 0) {
            //printf("Error: Comillas no cerradas\n");
            //free(input);
            continue; // Esperar la siguiente entrada
        }

        handle_input(&info, input, &print_mode);
        //print_tokens_list_alb(&info);
        info.sections = create_sections_list(&info);
			//print_sections_info(info.sections);
			executor(&info);
    
        //free_sections_list(info.sections); 
        //free_tokens_list_tokenize(&info);
        free(input); // Liberar la memoria de la entrada
        free_sections_list(info.sections);            
        free_tokens_list(info.tokens_list);
    }

    // Descomentar si deseas guardar el historial
    // cleanup(history_file);
    write_history(history_file);
    return 0;
}

/*
int main(int argc, char **argv, char **env)
{
    char *input;
    t_general info;
    const char *history_file;
	(void)argv;

    history_file = ".minishell_history";
	info.number_of_tokens = 0;
	info.tokens_list = NULL;
	if (argc != 1)
		return (0);
	signal(SIGINT, sigint_handler);   // Para ctrl+C
    signal(SIGQUIT, sigquit_handler); // Para ctrl+"\"
	signal(SIGTERM, sigterm_handler); // Para el builtin exit

    using_history();
    read_history(history_file);

	set_paths_and_env(&info, env);
    while (1)
    {
		input = readline("mini> ");
		if (!input)
		{
			printf("\nExit\n");
			free_info(&info);
			break; // Salida en EOF
		}
		if (*input)
		{
            add_history(input); // Añade al historial si no es una entrada vacía

			//tokenizer(&info, input);
            tokenize_input(&info, input);
			t_token	*new_tokens_list = reverse_copy_list(info.tokens_list);
            free_tokens_list(info.tokens_list);
			info.tokens_list = new_tokens_list;
			//print_tokens_list(info.tokens_list);

			info.sections = create_sections_list(&info);
			//print_sections_info(info.sections);
			executor(&info);

			free_sections_list(info.sections);            
            free_tokens_list(info.tokens_list);
		}
        free(input);
    }
    write_history(history_file);
    return 0;
}
*/
