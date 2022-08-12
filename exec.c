#include "exec.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#define _GNU_SOURCE


// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"


static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}


// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"


static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}


// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here


static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int index = block_contains(eargv[i], '=');
		if (index != -1) {
			char *key =
			        (char *) calloc(sizeof(eargv[i]) / sizeof(char),
			                        sizeof(char));
			char *value =
			        (char *) calloc(sizeof(eargv[i]) / sizeof(char),
			                        sizeof(char));
			get_environ_key(eargv[i], key);
			get_environ_value(eargv[i], value, index);
			setenv(key, value, 1);
		}
	}
}


// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file


static int
open_redir_fd(char *file, int flags, int old_file_descriptor)
{
	int new_file_descriptor;
	int dupfd;

	if (strcmp(file, "&1") == 0) {
		new_file_descriptor = 1;
	}

	else {
		if ((new_file_descriptor = open(file, flags, 0644)) == -1) {
			perror("open");
			_exit(-1);
		}
	}


	if ((dupfd = dup2(new_file_descriptor, old_file_descriptor)) == -1) {
		perror("dup");
		_exit(-1);
	}

	if (close(new_file_descriptor) == -1) {
		perror("close");
		_exit(-1);
	}

	return dupfd;
}


// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option

void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:

		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);

		_exit(-1);
		break;

	case BACK: {
		b = (struct backcmd *) cmd;

		int i = fork();

		if (i > 0) {
			waitpid(i, NULL, WNOHANG);
			_exit(0);
		}

		if (i == 0) {
			exec_cmd(b->c);
		}

		if (i < 0) {
			perror("fork");
			_exit(-1);
		}

		break;
	}

	case REDIR: {
		r = (struct execcmd *) cmd;

		if (strlen(r->out_file) > 0) {
			open_redir_fd(r->out_file, O_CREAT | O_RDWR | O_TRUNC, 1);
		}

		if (strlen(r->in_file) > 0) {
			open_redir_fd(r->in_file, O_RDONLY, 0);
		}

		if (strlen(r->err_file) > 0) {
			open_redir_fd(r->err_file, O_CREAT | O_RDWR | O_TRUNC, 2);
		}

		execvp(r->argv[0], r->argv);
		_exit(-1);
		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;

		struct cmd *cmd_izq = p->leftcmd;
		struct cmd *cmd_der = p->rightcmd;

		pipe_cmd(cmd_izq, cmd_der);
		free_command(parsed_pipe);
		_exit(0);

		break;
	}
	}
}


void
pipe_cmd(struct cmd *cmd_izq, struct cmd *cmd_der)
{
	int file_descriptor[2];

	pipe(file_descriptor);

	int i = fork();

	if (i == 0) {
		close(file_descriptor[0]);
		dup2(file_descriptor[1], 1);
		close(file_descriptor[1]);
		exec_cmd(cmd_izq);
		_exit(0);
	}

	int j = fork();

	if (j == 0) {
		close(file_descriptor[1]);
		dup2(file_descriptor[0], 0);
		close(file_descriptor[0]);
		exec_cmd(cmd_der);
		_exit(0);
	}

	close(file_descriptor[0]);
	close(file_descriptor[1]);

	waitpid(i, NULL, 0);
	waitpid(j, NULL, 0);
}
