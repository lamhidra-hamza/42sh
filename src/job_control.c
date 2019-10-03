/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   job_control.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hlamhidr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/09/24 19:32:46 by hlamhidr          #+#    #+#             */
/*   Updated: 2019/09/24 19:32:48 by hlamhidr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "shell.h"

void	ft_continue(void)
{
	t_job *process;

	process = job->content;
	if (process->pgid == -1)
		process = job->next->content;
	if (process->status == STOPED)
		kill(process->pgid, SIGCONT);
}

void	ft_catch_sigchild(int sig)
{
	int pid;
	int status;
	t_job *process;
	t_list *head;
	t_list *pr;

	head = job;
	pr = NULL;
	sig = 0;
	if ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		while (job)
		{
			process = job->content;
			if (pid == process->pgid)
			{
				if (WIFSTOPPED(status))
				{	
					process->status = STOPED;
					printf("[%d] Stopped \n", 1);
				}
				else if (WIFEXITED(status) || WTERMSIG(status))
				{
					if (WTERMSIG(status))
						printf("[1]+  Terminated: 15\n");
					else if (process->job)
						printf("[%d] Done \n", 1);
					if (pr == NULL)
					{
						pr = job;
						job = job->next;
						free(pr->content);
						free(pr);
					}
					else
					{
						pr->next = job->next;
						free(job->content);
						free(job);
					}
				}
				break ;
			}
			pr = job;
			job = job->next;
		}
	}
}

void	ft_foreground(void)
{
	t_job *process;
	int status;

	process = job->content;
	if (process->pgid == -1)
		process = job->next->content;
	if (process->status == STOPED || process->status == RUN)
	{
		tcsetattr(0, TCSANOW, &process->term_child);
		kill(process->pgid, SIGCONT);
		if (tcsetpgrp(0, process->pgid) == -1)
			ft_putendl_fd("ERROR in seting the controling terminal to the child process", 2);
		process->status = RUN;
		waitpid(process->pgid, &status, WUNTRACED);
		if (WIFSTOPPED(status))
			process->status = STOPED;
		if (tcsetpgrp(0, getpgrp()) == -1)
			ft_putendl_fd("ERROR in reset the controling terminal to the parent process", 2);
	}
}