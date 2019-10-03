/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pipe.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: onouaman <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/07/24 23:40:16 by onouaman          #+#    #+#             */
/*   Updated: 2019/07/24 23:40:17 by onouaman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "shell.h"
#include "read_line.h"
/*
**	ft_close_pipes : close all pipes created  :
*/

void		ft_close_pipes(t_pipes *st_pipes)
{
	while (st_pipes != NULL)
	{
		if (st_pipes->fds[0] != 0)
			close(st_pipes->fds[0]);
		if (st_pipes->fds[1] != 0)
			close(st_pipes->fds[1]);
		st_pipes = st_pipes->next;
	}
}

/*
**	ft_create_pipes : create all pipes needed :
*/

void		ft_create_pipes(t_pipes *st_pipes)
{
	t_pipes *st_head;
	int		fds[2];

	st_head = st_pipes;
	while (st_pipes->next != NULL)
	{
		if (pipe(fds) == -1)
			ft_err_exit("Error in Pipe Function \n");
		if (st_pipes == st_head)
			st_pipes->fds[0] = fds[0];
		st_pipes->fds[1] = fds[1];
		if (st_pipes->next != NULL)
			st_pipes->next->fds[0] = fds[0];
		st_pipes = st_pipes->next;
		if (st_pipes->next == NULL)
			st_pipes->fds[1] = fds[1];
	}
}

/*
**	ft_apply_pipe_h : helper function (norme) :
*/

void		ft_apply_pipe_h(t_pipes *st_pipes, t_pipes *st_head,
	int i, char ***environ)
{
	if (dup2(st_pipes->fds[i], i) == -1)
		ft_putendl_fd("Error in dub STD_", 2);
	ft_close_pipes(st_head);
	ft_cmd_fork(0, st_pipes, environ);
	exit(0);
}

/*
**	ft_apply_pipe : PIPE :
*/

int			ft_apply_pipe(t_pipes *st_pipes, char ***environ)
{
	t_pipes	*st_head;
	int		parent;
	int		status;

	status = 0;
	st_head = st_pipes;
	if ((parent = fork()) == -1)
		ft_err_exit("Error in Fork \n");
	(parent == 0) ? ft_create_pipes(st_pipes) : NULL;
	while (parent == 0 && st_pipes != NULL)
	{
		if (!ft_check_cmd(st_pipes, *environ) && fork() == 0)
		{
			if (st_pipes != st_head && dup2(st_pipes->fds[0], 0) == -1)
				ft_putendl_fd("Error in dub STD_IN", 2);
			ft_apply_pipe_h(st_pipes, st_head, 1, environ);
		}
		st_pipes = st_pipes->next;
		if (st_pipes && st_pipes->next == NULL)
			ft_apply_pipe_h(st_pipes, st_head, 0, environ);
	}
	t_job *process;

	status = 0;
	process = NULL;
	if (parent > 0)
	{
		process = job->content;
		//printf("add1 = %p\n",process);
		process->pgid = (process->pgid == -1) ? parent : process->pgid;
		setpgid(process->pgid, parent);
		process->status = RUN;
		if (!st_pipes->bl_jobctr)
		{
			if (tcsetpgrp(0, parent) == -1)
				ft_putendl("ERROR in seting the controling terminal to the child process");
			g_sign = 1;
			waitpid(parent, &status, WUNTRACED);
			g_sign = 0;
			if (WIFSTOPPED(status))
			{
				process->status = STOPED;
				tcgetattr(0, &process->term_child);
				printf("\n[%d] Stopped  cmd\n", process->index);
			}
		}
		else if (process->pgid == parent)
		{
			process->job = 1;
			printf("[%d] %d\n", process->index, process->pgid);
		}
		if (tcsetpgrp(0, getpgrp()) == -1)
			ft_putendl("ERROR in reset the controling terminal to the parent process");
	}
	return ((status) ? 0 : 1);
}

/*
** exec pipe
*/

int			ft_pipe(t_pipes *st_pipe, char ***env)
{
	int status;

	status = 0;
	if (!st_pipe)
		return (-1); /// Check this status
	/// if exist pipe
	if (st_pipe && st_pipe->next)
		status = ft_apply_pipe(st_pipe, env);
	else
		status = ft_cmd_fork(1, st_pipe, env);
	return (status);
}