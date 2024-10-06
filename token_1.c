/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_1.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: albealva <albealva@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/08 18:10:59 by albealva          #+#    #+#             */
/*   Updated: 2024/10/05 18:15:06 by albealva         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char	*get_token_type_name(int type)
{
	static const char *token_type_names[9]; 

	token_type_names[0] = "EMPTY";
	token_type_names[1] = "CMD";
	token_type_names[2] = "ARG";
	token_type_names[3] = "TRUNC";
	token_type_names[4] = "APPEND";
	token_type_names[5] = "INPUT";
	token_type_names[6] = "FIL";
	token_type_names[7] = "PIPE";
	token_type_names[8] = "END";
	if (type >= 0 && type <= 8)
		return token_type_names[type];
	else
		return "UNKNOWN";
	
}

// Libera la memoria ocupada por la lista de tokens
void	free_tokens_list_tokenize(t_general *info)
{
	t_token *current;
	t_token *next;

	current = info->tokens_list;
	while (current != NULL)
	{
		next = current->next;
		free(current->str);
		free(current);
		current = next;
	}
	info->tokens_list = NULL;
	info->number_of_tokens = 0;
}

void	quoting_choice(t_quote_state *state, char c)
{
	if (c == '"' && state->sq == 0)
	{
		if (state->dq == 0)
			state->dq = 1;
		else
			state->dq = 0;
	} 
	else if (c == '\'' && state->dq == 0)
	{
		if (state->sq == 0)
			state->sq = 1;
		else
			state->sq = 0;
	}
}


int	open_quote(char *line, t_quote_state *state)
{
	int i;

	i = 0;
	state->dq = 0;
	state->sq = 0;
	while (line[i] != '\0')
	{
		quoting_choice(state, line[i]);
		i++;
	}
	if (state->dq != 0 || state->sq != 0)
	{
		printf("unclosed quotes error \n");
		return 1;
	}
	return 0;
}

void	error_malloc()
{
	perror("Malloc failed");
	exit(EXIT_FAILURE);
}

void	error_strdup()
{
	perror("Strdup failed");
	exit(EXIT_FAILURE);
}

void update_quotes(char current_char, int *in_single_quotes, int *in_double_quotes)
{
    if (current_char == '\'')
        *in_single_quotes = !(*in_single_quotes);
    else if (current_char == '\"')
        *in_double_quotes = !(*in_double_quotes);
}

void init_extract_section(int *in_single_quotes, int *in_double_quotes)
{
	*in_single_quotes = 0;
	*in_double_quotes = 0;
}

char *allocate_and_copy_section(char **start, char *end)
{
    size_t section_length = end - *start;
    char *section = malloc(section_length + 1);
    if (!section) {
        error_malloc();
    }
    strncpy(section, *start, section_length);
    section[section_length] = '\0';
    *start = end;
    return section;
}


char	*extract_section(char **start, const char *delimiters)
{
	char *end;
	char *section;
	int in_single_quotes;
	int in_double_quotes;
	
	init_extract_section(&in_single_quotes, &in_double_quotes);
	if (**start == '|') {
		section = malloc(2);
		if (!section)
			error_malloc();
		section[0] = '|';
		section[1] = '\0';
		(*start)++;
		return section;
	}
	end = *start;
	while (*end && (!strchr(delimiters, *end) || in_single_quotes || in_double_quotes))
	{
		update_quotes(*end, &in_single_quotes, &in_double_quotes);
		end++;
	}
	section = allocate_and_copy_section(start, end);
	return section;
}

void	add_token_to_list(t_general *info, const char *str, int type)
{
	t_token	*new_token;
	t_token *last;
	
	new_token = malloc(sizeof(t_token));
	if (!new_token)
		error_malloc();
	new_token->str = strdup(str);
	if (!new_token->str) 
		error_strdup();
	new_token->type = type;
	new_token->prev = NULL;
	new_token->next = NULL;
	if (!info->tokens_list)
		info->tokens_list = new_token;
	else
	{
		last = info->tokens_list;
		while (last->next)
			last = last->next;
		last->next = new_token;
		new_token->prev = last;
	}
	info->number_of_tokens++;
}

// Función para añadir un carácter a un string
char *add_char_to_token(char *token, char c)
{
	int len;
	char *new_token;
	
	if (token)
		len = strlen(token);
	else
		len = 0;
	new_token = malloc(len + 2);
	if (!new_token)
		error_malloc();
	if (token)
	{
		strcpy(new_token, token);
		free(token);
	}
	new_token[len] = c;
	new_token[len + 1] = '\0';
	return (new_token);
}

int ft_isspace(char c)
{
	return (c == ' ' || c == '\t' || c== '\r');
}

char *get_env_var(t_general *info, const char *var_name)
{
	int		i;
	char	*equal_sign;
	size_t	name_length;
	
	i = 0; 
	while (info->env[i] != NULL)
	{ 
		equal_sign = strchr(info->env[i], '=');
		if (equal_sign != NULL)
		{
			name_length = equal_sign - info->env[i];
			if (strncmp(info->env[i], var_name, name_length) == 0 && name_length == strlen(var_name))
				return (equal_sign + 1);
		}
		i++;
	}
	return ("");
}

int is_special_separator(char c)
{
	return (c == ' ' || c == '\t' || c == '\'' || c == '\"' || c == '<' || c == '>' || c == '|' || c == '$');
}

char* itoa(int value, char* str, int base)
{
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
char* expand_dollar_question(t_general *info)
{
	char* result;
	
	result = (char*)malloc(12);
	if (result == NULL) 
		error_malloc();
	itoa(info->exit_status, result, 10);

	return (result);
}

int count_dollars(const char *section)
{
	int count;
	int i;

	i = 0;
	count = 0;
	while (section[i] != '\0')
	{
    if (section[i] == '$')
        count++;
    i++;
	}
	return (count);
}

void print_start_pos(int *start_pos)
{
	int i;
	
	i = 0;
	printf("Contenido de start_pos: ");
	while (start_pos[i])
	{
        printf("%d ", start_pos[i]);
		i++;
	}
	printf("\n");
}

int reset_positions(int *start_pos, int size_malloc)
{
	int k;
	
	k = 0;
	if(start_pos == NULL)
		return (-1);
	while (k < size_malloc)
	{
    	start_pos[k] = -1;
    	k++;
	}
	return (0);
}

char	*initialize_var_name()
{
    static char var_name[256];
    memset(var_name, 0, sizeof(var_name));
    return (var_name);
}
void	extract_var_name(const char *input, int *i, char *var_name, int *var_index)
{
	while (input[*i] && !is_special_separator(input[*i]) && (isalnum(input[*i]) || input[*i] == '_'))
	{
        var_name[(*var_index)++] = input[*i];
        (*i)++;
    }
    var_name[*var_index] = '\0';
}

char *handle_dollar_expansion(const char *input, int *i, int *temp_index, char *temp, t_general *info)
{
    static char var_name[256];
    int var_index;
	char* exit_status_str;

	var_index = 0;
	memset(var_name, 0, sizeof(var_name));	
    if (input[*i + 1] == '?')
	{
        exit_status_str = expand_dollar_question(info);
        if (exit_status_str != NULL)
		{
            strcpy(temp + *temp_index, exit_status_str);
            *temp_index += strlen(exit_status_str);
            free(exit_status_str);
        }
        (*i) += 1;
        return (NULL);
    }
	else
	{
        (*i)++;
		extract_var_name(input, i, var_name, &var_index);
        (*i)--;
        return (get_env_var(info, var_name));
    }
}

void	copy_until_start_pos(const char *input, int start_pos, char *temp, int *temp_index)
{
    int j;
	
	j = 0;
    while (j < start_pos - 1) {
        temp[(*temp_index)++] = input[j++];
    }
    if (start_pos == 0) {
        temp[(*temp_index)++] = input[0];
    }
}

void expand_variables_in_input(const char *input, int start_pos, char *temp, int *temp_index, t_general *info)
{
    int len;
    int expanded;
    int i;

	len = strlen(input);
    expanded = 0;
    i = start_pos - 1;
    while (i < len)
	{
        if (input[i] == '$' && !expanded)
		{
            char *value = handle_dollar_expansion(input, &i, temp_index, temp, info);
            if (value)
			{
                strcpy(temp + *temp_index, value);
                *temp_index += strlen(value);
            }
            expanded = 1;
        }
		else
            temp[(*temp_index)++] = input[i];
        i++;
    }
}

char *expand_variable(const char *input, int start_pos, t_general *info)
{
    char *result;
    char temp[1024]; 
    int temp_index;  

    result = NULL;
    temp_index = 0;
	memset(temp, 0, sizeof(temp));  
    copy_until_start_pos(input, start_pos, temp, &temp_index);
    expand_variables_in_input(input, start_pos, temp, &temp_index, info);
    result = strdup(temp);
    return (result);
}

/*
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
*/

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




void extract_tokens(const char *section, t_general *info) {
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
				free(current_token);
				current_token = NULL;
				j=reset_positions(start_pos, size_malloc);
				j = 0;
				quote_state = NONE;
			}
			add_token_to_list(info, ">>", APPEND);
			free(current_token);
			current_token = NULL;
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
				free(current_token);
				current_token = NULL;
				j=reset_positions(start_pos, size_malloc);
				j = 0;
				quote_state = NONE;
			}
			add_token_to_list(info, ">", TRUNC);
			free(current_token);
			current_token = NULL;
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
				free(current_token);
				current_token = NULL;
				j=reset_positions(start_pos, size_malloc);
				j = 0;
				quote_state = NONE;
			}
			add_token_to_list(info, "<", INPUT);
			free(current_token);
			current_token = NULL;
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
				free(current_token);
				current_token = NULL;
				j=reset_positions(start_pos, size_malloc);
				j = 0;
				quote_state = NONE;
			}
			while (section[i] == '|') {
				add_token_to_list(info, "|", PIPE);
				free(current_token);
				current_token = NULL;
				i++;
				is_first_token = 1;
				expect_file = 0;
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
				free(current_token);
				current_token = NULL;
				j=reset_positions(start_pos, size_malloc);
				j = 0;
				quote_state=NONE;
				is_first_token = 0;
				expect_file = 0;
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

	// Liberar la memoria de current_token
	free(current_token);
	current_token = NULL;

	// Resetear posiciones
	j = reset_positions(start_pos, size_malloc);
	j = 0;
	quote_state = NONE;
	is_first_token = 0;
	expect_file = 0;
}

// Aquí puedes liberar start_pos si fue utilizado
if (count_dollars(section))
	free(start_pos);
if (current_token != NULL)
{
	free(current_token);
	current_token = NULL;
}
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



void process_section(char *section, t_general *info) {
	if (section) {
		extract_tokens(section, info);
		free(section);
	}
}

t_token *tokenize_input(t_general *info, char *input) {
	t_quote_state state;
	open_quote(input, &state); // Verifica las comillas antes de tokenizar

	char *start = input;
	const char *section_delimiters = "|\n\r";

	info->tokens_list = NULL;

	while (*start) {
		char *section = extract_section(&start, section_delimiters);
		if (section) {
			process_section(section, info);
		}
	}

	return info->tokens_list;
}
