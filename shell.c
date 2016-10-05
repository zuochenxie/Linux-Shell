//
//  Created by Zuochen Xie on 2/02/16.
//  Copyright (c) 2016 Zuochen Xie. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

char **readcommand(int *pipes, int* amp);
int execute(char **command, int pipes, int amp);
char** getCmdByNum(char ** command, int num);

int main(int argc, char *argv[])
{
	char **command;
	int pipes = 0;
	int amp = 0;
	while(1)
	{	
		if(isatty(STDIN_FILENO) == 1){		
				printf("sish:>");
			}	
		command = readcommand(&pipes, &amp);	
		if(command[0] == NULL) {
			//printf("sish:>");
			continue;
		}
		if (strcmp(command[0], "exit")==0){
			break;
		}
		else {
    			execute(command, pipes, amp);	
		}		
	}
  	exit(EXIT_SUCCESS);
}

char **readcommand(int* pipes, int* amp)
{

	char *line = NULL;
	size_t size = 0;
	ssize_t read;
	char *token;
	char **tokenArr;
	int index = 0;
	*pipes = 0;
	*amp = 0;

	char *temp2 = malloc(513*sizeof(char));

	tokenArr = malloc(513*sizeof(char*));
	read = getline(&line, &size, stdin);
	if (read == -1)
	{	
		exit(EXIT_FAILURE);
	}
	token = strtok(line, " \n\t\r\v\f");
	while(token != NULL) {
		int i = 0;
		int j = 0;
		int flag = 0;	
		char *temp5 = malloc(513*sizeof(char));	
		while(j < strlen(token)){
			if(token[j] != '\"'){
				temp5[j] = token[j];
			}
			if(token[j] == '\"'){
				if(j == 0){
					char *temp4 = malloc(513*sizeof(char));
					int k = 0;
					j++;
					if (token[j] == '\0'){
							token = strtok(NULL, " \n");
							if (token == NULL){
								printf("ERROR: Didnt find second quotation mark1\n");
								tokenArr[0] = NULL;
								return tokenArr;	
							}					
							j = 0;
							temp4[k] = ' ';
							k++;
						}				
					if (token[j] != '\"'){				
						temp4[k] = token[j];
						j++;
						k++;
					}
					while(token[j] != '\"'){
						if (token[j] == '\0'){
							token = strtok(NULL, " \n");
							if (token == NULL){
								printf("ERROR: Didnt find second quotation mark2\n");
								tokenArr[0] = NULL;
								return tokenArr;	
							}		
							j = 0;
							temp4[k] = ' ';
							k++;
						}
						else{
							temp4[k] = token[j];
							j++;
							k++;
						}
					}
					tokenArr[index] = temp4;
					index++;
					if(token[j+1] == '\0' ){
						flag = 2;
						break;
					}
					continue;
				}
				else if (j > 0){
					char *temp6 = malloc(513*sizeof(char));
					int k = 0;
					j++;
					if (token[j] == '\0'){
							token = strtok(NULL, " \n");
							if (token == NULL){
								printf("ERROR: Didnt find second quotation mark3\n");
								tokenArr[0] = NULL;
								return tokenArr;	
							}					
							j = 0;
							temp6[k] = ' ';
							k++;
						}
					if (token[j] != '\"'){					
						temp6[k] = token[j];
						j++;
						k++;
					}
					while(token[j] != '\"'){
						if (token[j] == '\0'){
							token = strtok(NULL, " \n");
							if (token == NULL){
								printf("ERROR: Didnt find second quotation mark4\n");
								tokenArr[0] = NULL;
								return tokenArr;	
							}					
							j = 0;
							temp6[k] = ' ';
							k++;
						}
						else{
							temp6[k] = token[j];
							j++;
							k++;
						}
					}
					strcat(temp5, temp6);
					tokenArr[index] = temp5;
					index++;
					if(token[j+1] == '\0' ){
						flag = 2;
						break;
					}
					continue;
				}
			}
		j++;
		}


		while(flag != 2 && i < strlen(token)){
			if(token[i] == '|'|| token[i] == '<' || token[i] == '>' ){

				char *temp1 = malloc(513*sizeof(char));
				strncpy(temp1, token, i);
				if(temp1 != NULL && i != 0){
					tokenArr[index] = temp1;
					index++;
				}
				char *temp3 = malloc(2*sizeof(char));
				temp3[0] = token[i];
				temp3[1] = '\0';
				if(token[i] == '|'){
					(*pipes)++;
				}
				tokenArr[index] = temp3;
				index++;
				strncpy(temp2, token+i+1, strlen(token)-i);
				strcpy(token, temp2);
				i = 0;
				flag = 1;
				continue;
			}
			i++;
		}

		if(flag == 1 && temp2 != NULL && (temp2[0] != '\0') && i != strlen(token)-1){
			tokenArr[index] = temp2;
			index++;
		}
		if (flag == 0) {
		        	tokenArr[index] = token;
      					if (strcmp(token, "|")==0){
						(*pipes)++;
      					}
      				index++;
			}
      		token = strtok(NULL, " \n\t\r\v\f");
	}
	tokenArr[index] = NULL;
	if (index != 0 && strcmp(tokenArr[index-1], "&")==0){
      		*amp = 1;
			tokenArr[index-1] = NULL;
	}
	return tokenArr;
}


int execute(char **command, int pipes, int amp)
{
	int i, j;
	int inputflag, outputflag;
	char *inputfile, *outputfile;
	char** singleCmd;
	int numCmd = pipes + 1;
	int pipeSize = pipes * 2;
	int pipeArr[pipeSize];
	pid_t pidArr[numCmd];

	inputfile = malloc(255*sizeof(char));
	outputfile = malloc(255*sizeof(char));
	inputflag = 0;
	outputflag = 0;

	// Initialize all of the pipes.
	for(i = 0; i < pipeSize; i+=2) {
		pipe(pipeArr + i);
	}
	for(i = 0; i < numCmd; i++) {
		pidArr[i] = fork();
		if (pidArr[i] < 0){
	    	printf("ERROR: Can't Fork\n");
	    	exit(EXIT_FAILURE);
		}
		else if (pidArr[i] == 0){
	    	// Child process.
			singleCmd = getCmdByNum(command, i);

			// FILE INPUT/OUTPUT
			j = 0;
			while(singleCmd[j] != NULL) {
				if((i == 0) && (strcmp(singleCmd[j], "<") == 0)) {
					singleCmd[j] = NULL;
					strcpy(inputfile, singleCmd[j+1]);
					inputflag = 1;
					j++;
				}
				if((i == numCmd-1) && (strcmp(singleCmd[j], ">") == 0)) {
					singleCmd[j] = NULL;
					strcpy(outputfile, singleCmd[j+1]);
					outputflag = 1;
					j++;
				}
				j++;
			}

			// Create pipes.
			if(pipes > 0) {
				// Create INPUT pipe if not first command.
				if(i > 0) {
					//close(STDIN_FILENO);	// Close STDIN.
					dup2(pipeArr[-2 + 2*i], STDIN_FILENO);	// Replace STDIN.
					close(pipeArr[-2 + 2*i]);
				}
				// Create OUTPUT pipe if not last command.
				if(i < numCmd-1) {
					//close(STDOUT_FILENO);	// Close STDOUT.
					dup2(pipeArr[1 + 2*i], STDOUT_FILENO);	// Replace STDOUT.
					close(pipeArr[1 + 2*i]);
				}
			}
			// Create FILE INPUT pipe if first command.
			if(i == 0 && inputflag) {
				int fp = open(inputfile, O_RDONLY);
				dup2(fp, STDIN_FILENO);
				close(fp);
				inputflag = 0;
			}
			// Create FILE OUTPUT pipe if last command.
			if(i == numCmd-1 && outputflag) {
				int fp = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
				dup2(fp, STDOUT_FILENO);
				close(fp);
				outputflag = 0;
			}
	    	execvp(singleCmd[0], singleCmd);

			printf("ERROR: Command not found\n");
			exit(EXIT_FAILURE);
		}
		// Parent process. Close pipes.
		if(pipes > 0) {
			if(i > 0)
				close(pipeArr[-2 + 2*i]);
			if(i < numCmd-1)
				close(pipeArr[1 + 2*i]);
		}
	}
	// Wait until all child processes have terminated.
	int options = (amp) ? WNOHANG : 0;
	for(i = 0; i < numCmd; i++) {
		waitpid(pidArr[i], 0, options);
	}
	return 0;
}

char** getCmdByNum(char ** command, int num) {
	int i, j;
	int numCmd;
	char** result = malloc(513 * sizeof(char*));
	// Find the start index of the command.
	for(i = 0, numCmd = 0; numCmd < num; numCmd++) {
		while(command[i] != NULL && strcmp(command[i], "|") != 0) {
			i++;
		}
		// If NULL is reached before command is found, return NULL.
		if(command[i] == NULL) return NULL;
		// Add 1 to i to move past the "|""
		if(strcmp(command[i], "|") == 0) {
			i++;
		}
	}
	// i is now the start index of the command.
	j = 0;
	while(command[i] != NULL && strcmp(command[i], "|") != 0) {
		result[j] = command[i];
		i++;
		j++;
	}
	result[j] = NULL;
	return result;
}
