/*
 * string_parser.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gguan, Monil
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"

#define _GUN_SOURCE

int count_token (char* buf, const char* delim)
{
     if(buf == NULL){
		return EXIT_FAILURE;
	 }

	 int num_tokens = 0;
	 char *token;
	 char *saveptr;
	 char *copy = strdup(buf);
	 token =strtok_r(copy, delim, &saveptr);
	 while(token != NULL){
		num_tokens++;
		token = strtok_r(NULL, delim, &saveptr);
	 }

	 free(copy);
	 return num_tokens;


}

command_line str_filler (char* buf, const char* delim)
{
    command_line result;
	int tokens = count_token(buf, delim);
	result.num_token = tokens;
	result.command_list = (char**)malloc((tokens + 1) * sizeof(char*));

	char* token;
	char* saveptr;
	strtok_r(buf, "\n", &saveptr);
	token = strtok_r(buf, delim, &saveptr);
	int i = 0;
	while(token != NULL){
		result.command_list[i] = (char*)malloc(strlen(token) + 1);
		strcpy(result.command_list[i], token);
		i++;
		token = strtok_r(NULL, delim, &saveptr);
	}

	result.command_list[tokens] = NULL;
	
	return result;
}


void free_command_line(command_line* command)
{
    for(int i = 0; i < command->num_token; i++){
		free(command->command_list[i]);
	}

	free(command->command_list);

}
