
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>


#define MAX_LINE		80 
#define MAX_COMMANDS	5 

char history[MAX_COMMANDS][MAX_LINE];
char display_history [MAX_COMMANDS][MAX_LINE];

int command_count = 0;



void addtohistory(char inputBuffer[]) {
	int i = 0;
	
	
	strcpy(history[command_count % MAX_COMMANDS], inputBuffer);
	
	
	while (inputBuffer[i] != '\n' && inputBuffer[i] != '\0') {
		display_history[command_count % MAX_COMMANDS][i] = inputBuffer[i];
		i++;
	}
	display_history[command_count % MAX_COMMANDS][i] = '\0';
	
	++command_count;

	return;
}


int setup(char inputBuffer[], char *args[],int *background)
{
    int length,		
	i,			
	start,			
	ct,				
	command_number;
    
    ct = 0;
	
   
	do {
		printf("osh>");
		fflush(stdout);
		length = read(STDIN_FILENO,inputBuffer,MAX_LINE); 
	}
	while (inputBuffer[0] == '\n');
	
    start = -1;
    if (length == 0)
        exit(0);           
    if ( (length < 0) && (errno != EINTR) ) {
		perror("error reading the command");
		exit(-1);          
    }
	
	
	if (inputBuffer[0] == '!') {
		if (command_count == 0) {
			printf("No history\n");
			return 1;
		}
		else if (inputBuffer[1] == '!') {
			
			strcpy(inputBuffer,history[(command_count - 1) % MAX_COMMANDS]);
			length = strlen(inputBuffer) + 1;
		}
		else if (isdigit(inputBuffer[1])) { 
			command_number = atoi(&inputBuffer[1]);
			strcpy(inputBuffer,history[command_number]);
			length = strlen(inputBuffer) + 1;
		}
	}
	
	
	
	
	addtohistory(inputBuffer);
	
	
	
    for (i=0;i<length;i++) { 
		
		
        switch (inputBuffer[i]){
			case ' ':
			case '\t' :               
				if(start != -1){
              		args[ct] = &inputBuffer[start];   
		    		ct++;
				}
				inputBuffer[i] = '\0'; 
				start = -1;
				break;
				
			case '\n':                
				if (start != -1){
					args[ct] = &inputBuffer[start];     
		    		ct++;
				}
				inputBuffer[i] = '\0';
				args[ct] = NULL; 
				break;
				
	    	default :           
				if (start == -1)
		    		start = i;
              	if (inputBuffer[i] == '&') {
	    			*background  = 1;
                   	inputBuffer[i-1] = '\0';
				}
		} 
	}   
	
	
	
	if (*background)
		args[--ct] = NULL;
	
	args[ct] = NULL; 
	
	return 1;
	
} /* end of setup routine */


int main(void)
{
	char inputBuffer[MAX_LINE]; 	
	int background;             	
	char *args[MAX_LINE/2 + 1];	
	pid_t child;            		
	int status;           		
	int shouldrun = 1;
	
	int i, upper;
		
    while (shouldrun){            		
		background = 0;
		
		shouldrun = setup(inputBuffer,args,&background);      
		
		if (strncmp(inputBuffer, "exit", 4) == 0)
			return 0;
		else if (strncmp(inputBuffer,"history", 7) == 0) {
			if (command_count < MAX_COMMANDS)
				upper = command_count;
			else 
				upper = MAX_COMMANDS;
			
			for (i = 0; i < upper; i++) {
				printf("%d \t %s\n", i, display_history[i]);
			}
			
			continue;
		}

				
		if (shouldrun) {
			child = fork();          
			switch (child) {
				case -1: 
					perror("could not fork the process");
					break; 
				
				case 0: 
					status = execvp(args[0],args);
					if (status != 0){
						perror("error in execvp");
						exit(-2); 
					}
					break;
				
				default : 
					if (background == 0) 
						while (child != wait(NULL)) 
							;
			}
		}
    }
	
	return 0;
}
