/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: albealva <albealva@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 13:34:43 by mgimon-c          #+#    #+#             */
/*   Updated: 2024/10/20 15:36:12 by albealva         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "libft/libft.h"
# include <stdio.h>
# include <fcntl.h>
# include <stdlib.h>
# include <limits.h>
# include <unistd.h>
# include <signal.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <stdbool.h> // Para tipo bool

# define EMPTY 0
# define CMD 1
# define ARG 2
# define TRUNC 3
# define APPEND 4
# define INPUT 5
# define FIL 6
# define PIPE 7
# define END 8

# define MAX_VAR_LENGTH 256
# define MAX_VAR_NAME 256

# define TEMP_BUFFER_SIZE 1024


typedef struct s_quote_state {
    int dq; // Indica si estamos dentro de comillas dobles
    int sq; // Indica si estamos dentro de comillas simples
} t_quote_state;

typedef struct	s_token
{
	char			*str;
	int				type;
	struct s_token	*prev;
	struct s_token	*next;
}	t_token;

typedef struct	s_file
{
	struct s_file	*next;
	char			*string;
	int				open_mode;
}	t_file;

typedef struct	s_heredoc
{
	struct s_heredoc	*next;
	char				*delimiter;
	int					fds[2];
}	t_heredoc;

typedef struct	s_section
{
	int					gottofree;
	struct s_general	*info;
	struct s_section	*next;
	t_file				*files;
	t_heredoc			*heredocs;
	int					fd_read;
	int					fd_write;
	char				**env;
	char				**paths;
	char				*path;
	char				**cmdv;
	pid_t				pid;
}	t_section;

typedef	struct	s_general
{
	int					exit_status;
	int					number_of_tokens;
	struct s_token		*tokens_list;
	char				**env;
	char				**exports;
	char				**paths;
	struct s_section	*sections;
}	t_general;

// tokens.c
t_token *tokenize_input(t_general *info, char *input);
t_token	*reverse_copy_list(t_token *tokens_list);


// sections.c
t_section	*create_sections_list(t_general *info);

// executor.c
void	executor(t_general *info);

// prints.c
void	put_str_fd(int fd, char *str);
void    print_tokens_list(t_token *tokens_list);
void    print_matrix(char **matrix);
void    print_string_to_stderror(char *str);
void    print_sections_info(t_section *section);
void    print_matrix_stderror(char **matrix);

// frees.c
void	matrix_free(char **str);
void	free_sections_list(t_section *first);
void    free_tokens_list(t_token *first);
void	free_files(t_file *files);
void	free_info(t_general *info);

// utils_1.c
int		thereis_pipe(t_token *first);
void    set_paths_and_env(t_general *info, char **env);
int		count_tokens_per_section(t_token *first);
int		count_sections(t_token *first);
int		ft_strncmp_pipex(const char *str, const char *str2, size_t c);

// utils_2.c
int		count_files_per_section(t_token *first);
int		count_cmdvs_per_section(t_token *first);
t_token *get_first_in_section(t_token *first, int s);
void    add_file_to_files(t_token *section_first, t_file *files, int *i, int n);
void    open_files_section(t_section *section);

// utils_3.c
int		exec_if_builtin_1(t_section *current);
void	exec_if_builtin_2(t_section *current);
char    *ft_strjoin_pipex(char const *s1, char const *s2);
void	set_cmd_in_paths(t_section *section);
void	set_path(t_section *section);

// utils_4.c
char    *add_var_equal(char *cmdv1);
char    **remove_env_line(t_section *current, int line);
int		is_directory(const char *path);
char	*ft_getenv(const char *name, char **env);
void	set_exports(t_general *info, char **env);

// utils_5.c
void	add_str_to_matrix(char ***matrix, char *str);
void    allocate_heredocs(t_section *section, t_token *first);
void	write_in_heredocs(t_section *current);
char    *clean_str_exit(char *str);
void    close_section_hdocs_parent(t_section *current);
void	close_all_process_files(void);

// builtins_1.c
int		execute_echo(t_section *current);
int		execute_pwd(t_section *current);
int		execute_env(t_section *current);

// builtins_2.c
void	execute_unset(t_section *current);
void	execute_export(t_section *current);
void    execute_cd(t_section *current);
void	execute_exit(t_section *current);



const char *get_token_type_name(int type);
int check_syntax_errors(t_general *info);
void free_tokens_list_tokenize(t_general *info);
int open_quote(char *line, t_quote_state *state);
char *extract_current_section(const char *section, t_general *info);
char	mark_char(char c);
char	unmark_char(char c);
int	is_marked(char c);

#endif
