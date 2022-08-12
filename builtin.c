#include "builtin.h"
#include <stdlib.h>

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0) {
		return 1;
	}
	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (strcasestr(cmd, "cd") != NULL) {
		if (strcmp(cmd, "cd") == 0) {
			if (chdir(getenv("HOME")) == -1) {
				status = 1;
			}
		}

		else {
			if (chdir(cmd + 3) == -1) {
				status = 1;
			}
		}

		strcpy(promt, getcwd(NULL, 0));

		return 1;
	}
	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") == 0) {
		printf("%s \n", getcwd(NULL, 0));
		status = 0;
		return 1;
	}
	return 0;
}
