/*  Erik Safford
 *  Child Reads a Delay/String, Parent Writes until Ctrl+c Signal
 *  Spring 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#define DEBUG 0

char str[512];          //Line read by parent process from stdin (parent process)
char readMessage[512];  //Edited line w/out the number that is printed on interval delay (child process)
int delay;              //Delay interval to print input, 5 by default (child process)
int alarmFlag = 0;      //After line is parsed and put into readMessage, alarmFlag is set to
                        //print the line every alarm seconds (child process)
int fd[2];
int rdr,wtr;  //Global file descriptors
int pid;


void parentInputHandler() {
	//This handler catches when a SIGINT (ctrl+c) is entered, and breaks the pause in the parent process
}

void childInputHandler() {
	//This handler catches when a SIGFPE is sent from the parent to the child, reads new input string/delay
	
	char initial[512];
	if(DEBUG == 1) { printf("before read initial: %s\n",initial); }
        read(fd[0],initial,512);  //Read 512 characters from fd[0] (pipe stdin)
        if(DEBUG == 1) { printf("after read initial: %s\n",initial); }

        char buffer[512];
        int i = 0;
	//Now to parse the read string for delay number/string
	
	if(initial[0] == '-') { //If a negative delay is entered
		printf("Negative interval entered, turning to positive interval\n");
		for(int m = 1; m < strlen(initial); m++) { //Skip -
                        if(isdigit(initial[m]) != 0) {  //Read in all numbers before non-number is encountered
                                buffer[i] = initial[m];
                                i++;
                        }
                        else {
                                break;
                        }
                }
	}
	else {  //Else if a normal positive delay is entered/no delay
		for(int m = 0; m < strlen(initial); m++) {
			if(isdigit(initial[m]) != 0) {  //Read in all the numbers from string before non-number
				buffer[i] = initial[m];
				i++;
			}
			else {
				break;
			}
		}
	}
	if(DEBUG == 1) { printf("buffer :%s\n",buffer); }

        if(i == 0) {  //If no delay was entered/read in
        	delay = 5;  //Set default delay of 5 seconds
		strcpy(readMessage,initial);  //Copy the exact read string from fd[0] to readMessage
		                              //readMessage is the string that is continuously printed by alarm
        }
        if(i > 0) {  //If a delay was entered
        	delay = atoi(&buffer[0]);  //Set the delay to the number read into buffer[]
        	if(delay == 0) {  //If user enters 0 second delay
			printf("0 time interval entered, interval defaulted to 5...\n");
			delay = 5; //Set back to default 5 seconds
		}
		memset(buffer, 0, sizeof(buffer));  //Clear buffer[] so we can do more processes with it
        	int j = 0;
		if(initial[0] == '-') { //If a negative delay was entered intially
			for(int k = i+2; k < strlen(initial);k++) {  //Copy string not including the -_ delay
                        	buffer[j] = initial[k];
                        	j++;
                	}
		}
		else {  //Else if a positive delay was entered initially
        		for(int k = i+1; k < strlen(initial);k++) {  //Copy string not including the _ delay
        			buffer[j] = initial[k];
               			j++;
        		}
		}
        	buffer[j] = '\0';  //Null terminate the final parsed string (without numbers)
        	strcpy(readMessage,buffer);  //Copy parsed string to readMessage, which is printed every alarm
	}
	//If the read message is 'exit', we want to exit the child process
	if(readMessage[0] == 'e') {
		if(readMessage[1] == 'x') {
			if(readMessage[2] == 'i') {
				if(readMessage[3] == 't') {
					printf("exiting child...\n");
					exit(0);
				}
			}
		}
	}

	memset(initial, 0, sizeof(initial));  //Reset string buffers so they can be used next time
	memset(buffer, 0, sizeof(buffer)); 		

	alarmFlag = 1;  //Set the alarm to start printing readMessage continuously until ctrl+c/exit
}

void printLine() {
	//When alarmFlag = 1, alarm/pause combo in child process calls this handler to print readMessage
	printf("%s",readMessage);
}

int main() {
	if(pipe(fd) == -1) { //Pipe with global file descriptors
		fprintf(stderr,"Pipe Error = %d, %s\n",errno,strerror(errno));
        	exit(1);
	}

	rdr = fd[0];  //Pipe stdin
	wtr = fd[1];  //Pipe stdout

	pid = fork();  //Fork into parent/child processes
	               //pid is the process id of the child process
	if(pid  == -1) { 
		fprintf(stderr,"Fork Error = %d, %s\n",errno,strerror(errno));
                exit(1);
	}

	if(pid > 0) {  //Parent process, reads from stdin, writes to the pipe
		while(1) {
			signal(SIGINT,parentInputHandler);  //Want to pause until ctrl+c
			signal(SIGFPE,SIG_IGN);  //Parent process never deals with SIGFPE, so ignore

			pause();  //Waits here until ctrl+c is pressed
			
			if(kill(pid,SIGSTOP) == -1) {  //Stop the child process readMessage printing
				fprintf(stderr,"Kill Error = %d, %s\n",errno,strerror(errno));
		                exit(1);
			}

			printf("Enter a string in the form: (delay) (string) or (string)");
        		fgets(str,512,stdin); //Read in a new delay/string from stdin
			if(DEBUG == 1) { printf("str: %s len: %ld\n",str, strlen(str)); }
			write(fd[1],str,strlen(str));  //Write the read string to the pipe fd[1]

			//Send the child process SIGFPE signal to specify it should read in new string from pipe fd[0]
			if(kill(pid,SIGFPE) == -1) { 
				fprintf(stderr,"Kill Error = %d, %s\n",errno,strerror(errno));
		                exit(1);
			}

			if(kill(pid,SIGCONT) == -1) {  //Continue the child process readMessage printing
				fprintf(stderr,"Kill Error = %d, %s\n",errno,strerror(errno));
		                exit(1);
			}
			
			//If the read string is 'exit', parent wants to send to child, wait for child to exit, then parent exits
			if(str[0] == 'e') {
                		if(str[1] == 'x') {
                        		if(str[2] == 'i') {
                                		if(str[3] == 't') {
                                        		printf("\nwaiting for child to finish...\n");
							wait(NULL);
							printf("child exited, parent now exiting...\n");
                                        		exit(0);
                                		}
                        		}
                		}
        		}
		}
	}
	else {  //Child process, reads from the pipe, prints readMessage continuously
		while(1) {
			signal(SIGINT,SIG_IGN);  //Want only the parent process to deal with ctrl+c, so ignore
               		signal(SIGFPE,childInputHandler); //Parent sends child SIGFPE when child should parse new readMessage from pipe fd[0]
			signal(SIGALRM,printLine); //When alarm/pause finish, SIGALRM calls printLine handler to print the readMessage
			
			if(alarmFlag == 1) { //If a string has been parsed for delay/string, alarmFlag will be 1
				if(DEBUG == 1) { printf("got to child while loop w: %s\n",readMessage); printf("delay = : %d\n",delay); }
				alarm(delay);  //Set an alarm timer for the specified/default delay seconds
				pause();       //Wait until the alarm seconds pass, SIGALRM goes off when alarm reaches 0
			}
		}
	}
}
