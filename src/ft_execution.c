/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_execution.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: onouaman <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/07/24 23:42:10 by onouaman          #+#    #+#             */
/*   Updated: 2019/09/23 00:58:29 by mfetoui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "shell.h"
#include "read_line.h"

///******* print *****/
void	ft_print_pipe(t_pipes *st_pipes)
{
	t_tokens *st_tokens;

	st_tokens = NULL;
	while (st_pipes)
	{
		dprintf(2,"\n\t\t------- Start Pipe-------\n\t\t");
		st_tokens = st_pipes->st_tokens;
		while (st_tokens != NULL)
		{
			dprintf(2, "<%s>  bl_jobctr=%d \n",st_tokens->value,st_pipes->bl_jobctr);
			st_tokens = st_tokens->next;
		}
		dprintf(2,"\n\t\t--------------\n");		
		st_pipes = st_pipes->next;
	}
}
void	ft_print_logopr(t_logopr *st_logopr)
{
	t_tokens *st_tokens;

	st_tokens = NULL;
	while (st_logopr)
	{
		dprintf(2,"\n\t------- Start LOG_OPR -------\n\t");
		st_tokens = st_logopr->st_tokens;
		while (st_tokens != NULL)
		{
			dprintf(2, "<%s>  bl_jobctr=%d \n",st_tokens->value,st_logopr->bl_jobctr);
			st_tokens = st_tokens->next;
		}
		dprintf(2,"\n\t--------------\n");
		ft_print_pipe(st_logopr->st_pipes);
		st_logopr = st_logopr->next;
	}
}
void	ft_print_jobctr(t_jobctr *st_jobctr)
{
	t_tokens *st_tokens;

	st_tokens = NULL;
	while (st_jobctr)
	{
		dprintf(2,"\n------- Start job_ctr-------\n");
		st_tokens = st_jobctr->st_tokens;
		while (st_tokens != NULL)
		{
			dprintf(2, " <%s>  bl_jobctr=%d \n",st_tokens->value,st_jobctr->status);
			st_tokens = st_tokens->next;
		}
		dprintf(2,"\n--------------\n");
		ft_print_logopr(st_jobctr->st_logopr);
		st_jobctr = st_jobctr->next;
	}
}

void	ft_print_tokens(t_cmds *st_cmds)
{
	t_jobctr *st_jobctr;

	if (!st_cmds)
		return ;
	st_jobctr = st_cmds->st_jobctr;
	ft_print_jobctr(st_jobctr);
}
///******* End print *****/
	/*if (ft_error_redir(st_tokens))*/



/*
** Check if exist Cmd : check if Ok and permission
*/

int				ft_check_cmd(t_pipes *st_pipes, char **environ)
{
	int			rtn;
	char		*str_arg;
	char		*cmd;
	struct stat	st_stat;

	if (!st_pipes)
		return (0);
	rtn = 0;
	cmd = NULL;
	if (st_pipes->st_tokens)
		cmd = st_pipes->st_tokens->value;
	if (!ft_check_char(cmd, '/'))
		str_arg = ft_find_path(cmd, environ);
	else
	{
		str_arg = ft_strdup(cmd);
		if (access(str_arg, F_OK) != 0 && ++rtn)
			ft_print_error(FIL_NS, NULL, str_arg, 2);
		else if (lstat(str_arg, &st_stat) == 0 && S_ISDIR(st_stat.st_mode) && ++rtn)
			ft_print_error("Is a directory", "21sh :", str_arg, 0);
	}
	if (!rtn && str_arg && access(str_arg, X_OK) != 0 && ++rtn)
		ft_print_error(FIL_PD, NULL, str_arg, 2);
	if (rtn == 0 && str_arg == NULL && ++rtn)
		ft_print_error(CMD_NF, "21sh: ", cmd, 0);
	return (rtn);
}

/*
** Execute Cmd
*/

int			ft_cmd_exec(t_pipes *st_pipes, char **env)
{
	char	*str_arg;
	int		i;

	str_arg = NULL;
	i = 0;
	if (st_pipes->args == NULL || st_pipes->args[0] == NULL)
		return (-2);
	if (!ft_check_char(st_pipes->args[0], '/'))
		str_arg = ft_find_path((st_pipes->args)[0], env);
	else
	{
		str_arg = ft_strdup(st_pipes->args[0]);
		if (access(str_arg, F_OK) != 0 && ++i)
			ft_print_error(FIL_NS, NULL, str_arg, 2);
	}
	if (!i && str_arg && access(str_arg, X_OK) != 0 && ++i)
		ft_print_error(FIL_PD, NULL, str_arg, 2);
	if (i == 0 && str_arg != NULL)
	{
		execve(str_arg, st_pipes->args, env);
		ft_strdel(&str_arg);
		exit(0);
	}
	return ((i == 0) ? -1 : -2);

}

/*
** Check if cmd is builtens
*/

int				ft_cmd_fork(int fork_it, t_pipes *st_pipes, char ***env)
{
	pid_t pid;
	int status;
	t_job *process;

	pid = 0;
	status = 0;
	process = NULL;
	/// Fill args from tokens
	ft_tokens_args(st_pipes);
	/// Remove Quote
	ft_remove_quot(st_pipes->args);
	/// Check if Builtens
	if (st_pipes && ft_check_built((st_pipes->args)[0]))
		return (ft_init_built(st_pipes, env)); ///  add return to ft_init_built
	/// Fork - Child
	if (fork_it && (pid = fork()) == -1)
		ft_err_exit("Error in Fork new process \n");
	if (pid == 0)
	{
		ft_signal_default();
		pid = getpgrp();
		setpgid(pid, pid);
		if (ft_check_redi(st_pipes) && ft_parse_redir(st_pipes) == PARSE_KO)
			exit(EXIT_FAILURE);
		if (!ft_strcmp(st_pipes->args[0], "echo"))
			ft_buil_echo(st_pipes->args);
		else if (ft_cmd_exec(st_pipes, *env) == -1)
		{
			ft_print_error(CMD_NF, "21sh: ", (st_pipes->args)[0], 0);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		process = job->content;
		//printf("add00001 = %p\n",process);
		process->pgid = (process->pgid == -1) ? pid : process->pgid;
		setpgid(process->pgid, pid);
		process->status = RUN;
		if (!st_pipes->bl_jobctr)
		{
			if (tcsetpgrp(0, pid) == -1)
			ft_putendl("ERROR in seting the controling terminal to the child process");
			g_sign = 1;
			waitpid(pid, &status, WUNTRACED);
			g_sign = 0;
			if (WIFSTOPPED(status))
			{
				process->status = STOPED;
				tcgetattr(0, &process->term_child);
				printf("\n[%d] Stopped  cmd\n", process->index);
			}
		}
		else if (process->pgid == pid)
		{
			process->job = 1;
			printf("[%d] %d\n", process->index, process->pgid);
		}
		if (tcsetpgrp(0, getpgrp()) == -1)
			ft_putendl("ERROR in reset the controling terminal to the parent process");
		//job->content = process;
	//	ft_enable();
	}
	return ((status) ? 0 : 1);
}

/*
 ** Config Cmds by : - Lexer - Check Syntax - apply her_doc - Execution - Clear lists
 */

int				ft_cmds_setup(char *str_arg, char ***environ)
{
	t_cmds		*st_cmds;

	if (str_arg == NULL)
		return (-1);
	st_cmds = ft_new_cmds();

	/// Fill args
	st_cmds->args = ft_str_split_q(str_arg, " \t\n");

	/// Apply Lexer
	if ((st_cmds->st_tokens = ft_lexer(st_cmds->args)) == NULL)
		return (-1);

	/// Check Error Syntax
	/*if (ft_error_syntax(st_tokens) == 1)
	  return (-1);*/

	/// Fill Lists of lists
	ft_parse_cmd(st_cmds);

	/// Apply here_doc
	ft_apply_her_doc(st_cmds->st_jobctr);

	/// Executions
	ft_cmds_exec(st_cmds, environ);

	/// Clear allocated space
	//ft_clear_cmds(st_cmds);
	return (1);
}

/*
 ** Execute Logical Operateur
 */

static void		logical_ops(t_logopr *st_logopr, char ***env)
{
	int		cmp;
	int		state;

	state = -1;
	while (st_logopr != NULL)
	{
		state = ft_pipe(st_logopr->st_pipes, env);
		if ((st_logopr->status == 248 && state == 0) ||
				(st_logopr->status == 76 && state == 1))
			st_logopr = st_logopr->next;
		else
		{
			cmp = st_logopr->status;
			st_logopr = st_logopr->next;
			while (st_logopr != NULL)
			{
				if (st_logopr->status != cmp)
				{
					st_logopr = st_logopr->next;
					break;
				}
				st_logopr = st_logopr->next;
			}
		}
	}
}

/*
 ** Execute cmds
 */

void			ft_cmds_exec(t_cmds *st_cmds, char ***environ)
{
	t_jobctr	*st_jobctr;
	UNUSED(environ);
	t_job *process;
	t_job *tmp;

	if (!st_cmds)
		return ;
	st_jobctr = st_cmds->st_jobctr;
	while (st_jobctr)
	{
		//st_jobctr->str_cmd = ft_tokens_str();
		process = ft_memalloc(sizeof(t_job));
		tmp = (job) ? job->content : NULL;
		process->pgid = -1;
		process->status = -1;
		process->index = 1;
		process->job = 0;
		(!job) ? (job = ft_lstnew(NULL, sizeof(t_job))) : (tmp->pgid != -1) ? (ft_lstadd(&job, ft_lstnew(NULL, sizeof(t_job)))) : 0;
		job->content = process;
		logical_ops(st_jobctr->st_logopr, environ);
		st_jobctr = st_jobctr->next;
	}
}
