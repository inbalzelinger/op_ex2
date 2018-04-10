#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <string.h>


#define MAX_JOBS 70

typedef struct job {
	int argumentsNUm;
	char** name;
	int pid;
}job;


int CommandLoop();
char** getAndSplitUserInput(char** userInput);
int NumWords(char* str);
enum BuiltInCommand CheckCommand(char **args, int numWords);
int ExecuteCommand(char ***usrSplitInput, char **userInput, int numWords);
void PrintStringArry(char **str, int numWords);
void FreeFunction(char*** str, int numWords);
int ExecuteBuiltInCommands(char ***userSplitInput, enum BuiltInCommand command, job *jobsList, int *jobsListSize,
						   char **userInput, int numWords);
void ExecuteBackground(char ***usrSplitInput, int numWords, job *jobsList, int *jobsListSize, char **userInput);
void CopyStrinsArry(char*** dstStr, char** strToCpy , int numWords);
enum CommandType CheckIfBackground(char*** args , int numWords);
void BackgroundJobStatus(job *jobsList, int *jobsListSize);
void executePwd();



enum BuiltInCommand{REG , CD , JOBS, EXIT};
enum CommandType{FOREGOUND, BACKGROUND};



int main() {
	CommandLoop();
	exit(1);
}


/**
* function name: CommandLoop
* the shell loop, return 0 when the user input is exit.
* @return -  int.
**/

int CommandLoop() {
	char *userInput = NULL;
	char **userSplitInput = NULL;
	int jobsListSize = 0;
	int i = 0;
	job jobsList[MAX_JOBS];
	enum BuiltInCommand commandKind;
	enum CommandType commandType;
	printf("press exit to finish\n");
while(1){
		printf("prompt>");
		userSplitInput = getAndSplitUserInput(&userInput);
		int numWords = NumWords(userInput);
	if (numWords < 1) {
		FreeFunction(&userSplitInput, numWords+1);
		free(userInput);
	}
	if (numWords >= 1) {
		commandType = CheckIfBackground(&userSplitInput, numWords);
		commandKind = CheckCommand(userSplitInput, numWords);
		if (commandType == BACKGROUND) {
			numWords--;
			ExecuteBackground(&userSplitInput, numWords, jobsList, &jobsListSize, &userInput);
			usleep(50000);
		} else {
			if (commandKind == REG) {
				ExecuteCommand(&userSplitInput, &userInput, numWords);
			} else {
				if (commandKind == EXIT) {
					FreeFunction(&userSplitInput, numWords+1);
					free(userInput);
					for (i = 0; i < jobsListSize; i++) {
						FreeFunction(&(jobsList[i].name), jobsList[i].argumentsNUm);
					}
					return 0;
				}
				ExecuteBuiltInCommands(&userSplitInput, commandKind, jobsList, &jobsListSize, &userInput, numWords);
			}
		}
	}
}
}


/**
* function name: getAndSplitUserInput
* the function reads the user input as a sentence and splits it to words
* @param userInput - an arry to the user sentence
* @return -  char** - the user split input.
**/


char** getAndSplitUserInput(char** userInput) {
	char* arg;
	char** userSplitInput = NULL;
	int wordsNumber = 0;
	int j = 0;
	size_t userInputLen = 0;
	ssize_t read;
	read = getline(userInput, &userInputLen, stdin);
	if (read > 0) {
		//ff
	}
	char* copy = malloc(strlen(*userInput) + 1);
	if (copy == NULL) {
		perror("malloc failed");
		exit(1);
	}
	strcpy(copy, *userInput);
	wordsNumber = NumWords(copy);
	userSplitInput = malloc(sizeof(char**) * (wordsNumber + 1));
	if (userSplitInput == NULL) {
		perror("malloc failed");
		exit(1);
	}
	arg = strtok(copy, "\n, " "");
	// get the other srguments
	while (arg != NULL) {
		userSplitInput[j] = malloc(sizeof(char*) * (strlen(arg)) + 1);
		if (userSplitInput[j] == NULL) {
			perror("malloc failed");
			exit(1);
		}
		strcpy(userSplitInput[j], arg);
		j++;
		arg = strtok(NULL, "\n, " "");
	}
	userSplitInput[j] = NULL;
	free(copy);
	return userSplitInput;
}


/**
* function name: ExecuteCommand
* the function execute a regular command.
* @param userInput - the user full sentence
* @param usrSplitInput - the user split input
* @param numWords the number of words in the user input
* @return -  int 1 - if everything ok.
**/

int ExecuteCommand(char ***usrSplitInput, char **userInput, int numWords)
{
	pid_t child_pid;
	int status;
	child_pid = fork();
	if (child_pid == 0) {
		printf("%ld\n" , (long)getpid());
		// Child process
		if (execvp((*usrSplitInput)[0], *usrSplitInput) == -1) {
			fprintf (stderr, "unknown command: %s\n", (*usrSplitInput)[0]);
			exit(1);
		}
		exit(EXIT_FAILURE);
	} else if (child_pid < 0) {
		perror("Fork failed");
		exit(1);
	} else {
		// Parent process
		waitpid(child_pid, &status, WUNTRACED);
		FreeFunction(&(*usrSplitInput), numWords);
		free(*userInput);
	}
	return 1;
}


/**
* function name: BackgroundJobStatus
* the function checks the status of the background proccess and update the jobsList.
* @param jobsList - the arry of the background proccess.
* @param jobsListSize - the sise of the jobslist.
 *
**/

void BackgroundJobStatus(job *jobsList, int *jobsListSize) {
	int i;
	int status;
	int j;
	int k = 0;
	for (i = 0; i < *(jobsListSize); i++) {
		pid_t return_pid = waitpid((jobsList)[i].pid , &status , WNOHANG);
		if(return_pid == -1) {
			//error;
		} else if (return_pid == 0) {
			//still running
		} else if (return_pid == (jobsList)[i].pid) {
			for (k = 0; k < (jobsList)[i].argumentsNUm; k++) {
				free(((jobsList)[i]).name[k]);
			}
			free(((jobsList)[i]).name);
			//rearange the arry
			for(j = i; j < (*jobsListSize) - 1; j++) {
				(jobsList)[j] = (jobsList)[j + 1];
			}
			(*jobsListSize)--;
		}
	}
}

/**
* function name: ExecuteBackground
* the function execute a background command.
* @param usrSplitInput - the user split input
* @param numWords the number of words in the user input
* @param jobsList - the arry of the background commands.
* @param jobsListSize - the sise of the jobslist.
* @param userInput - the user full sentence
**/


void ExecuteBackground(char ***usrSplitInput, int numWords, job *jobsList, int *jobsListSize,
					   char **userInput) {
	int pid;
	pid = fork();
	if (pid == 0)
	{
		fclose(stdin); // close child's stdin
		fopen("/dev/null", "r"); // open a new stdin that is always empty
		//fprintf(stderr, "Child Job pid = %d\n", getpid());
		//add pid to jobs list
		execvp((*usrSplitInput)[0], *usrSplitInput);
		// this should never be reached, unless there is an error
		fprintf (stderr, "unknown command: %s\n", (*usrSplitInput)[0]);
		exit(1);
	} else {
		jobsList[*jobsListSize].pid = pid;
		jobsList[*jobsListSize].argumentsNUm = numWords;
		CopyStrinsArry(&jobsList[*jobsListSize].name , (*usrSplitInput) , numWords);
		(*jobsListSize)++;
	}
	FreeFunction(&(*usrSplitInput) , numWords);
	free(*userInput);
}


/**
* function name: ExecuteBuiltInCommands
* the function execute a built in command
* @param usrSplitInput - the user split input
* @param command - the command
* @param jobsList - the arry of the background commands.
* @param jobsListSize - the sise of the jobslist.
* @param userInput - the user full sentence
* @param numWords the number of words in the user input
**/

int ExecuteBuiltInCommands(char ***userSplitInput, enum BuiltInCommand command, job *jobsList, int *jobsListSize,
						   char **userInput, int numWords) {
	int i;
	char cdArgs[50];
	if (command == CD) {
		// cd with no arguments or cd ~
		if ((*userSplitInput)[1] == NULL || strcmp((*userSplitInput)[1] , "~" ) == 0 ) {
			FreeFunction(&(*userSplitInput), numWords);
			free(*userInput);
			printf("%ld\n" , (long)getpid());
			return chdir(getenv("HOME"));
			// cd -
		} else if (strcmp((*userSplitInput)[1] , "-" ) == 0) {
			strcpy(cdArgs , "..");
			FreeFunction(&(*userSplitInput) , numWords);
			free(*userInput);
			if(chdir(cdArgs) < 0) {
				fprintf (stderr, "unknown command:\n");
			} else {
				printf("%ld\n" , (long)getpid());
			}
			executePwd();
			return 1;
			//cd with arguments.
		} else {
			for (i = 0; i < strlen((*userSplitInput)[1])+1 ; i++) {
				cdArgs[i] = (*userSplitInput)[1][i];
			}
		}
		FreeFunction(&(*userSplitInput) , numWords);
		free(*userInput);
		if(chdir(cdArgs) < 0) {
			fprintf (stderr, "unknown command: \n");
		} else {
			printf("%ld\n" , (long)getpid());
		}
	} else if (command == JOBS) {
		BackgroundJobStatus(jobsList, jobsListSize);
		for (i = 0; i < (*jobsListSize); i++) {
				printf("%d\t\t" , jobsList[i].pid);
			PrintStringArry(jobsList[i].name, jobsList[i].argumentsNUm);
		}
		FreeFunction(&(*userSplitInput) , numWords);
		free(*userInput);
	}
}


/**
* function name: CheckCommand
* the function gets strings arry and returns the kind of the command.
 * the function return REG if the command isnt built in command
 * else it returns the name of the relevant built in command.
* @param args - the string arry.
* @param numWords the number of words in args.
**/

enum BuiltInCommand CheckCommand(char **args, int numWords) {
	if (numWords <= 0) {
		return REG;
	}
	if (strcmp(args[0] , "cd") == 0) {
		return CD;
	} else if (strcmp(args[0] , "jobs") == 0) {
		return JOBS;
	} else if (strcmp(args[0] , "exit") == 0) {
		return EXIT;
	} else {
		return REG;
	}
}

/**
* function name: CheckIfBackground
* the function gets strings arry. in the last word in the arry is '&'
 * the function return BACKGROUND
 * else, the function returns FOREGOUND
* @param args - the string arry.
* @param numWords the number of words in args.
**/

enum CommandType CheckIfBackground(char*** args , int numWords) {
	if (numWords>0) {
		if (strcmp((*args)[numWords - 1] , "&") == 0) {
			free((*args)[numWords-1]);
			(*args)[numWords-1] = NULL;
			return BACKGROUND;
		} else {
			return FOREGOUND;
		}
	}
}

/**
* function name: CopyStrinsArry
* copy char** to char**.
* @param dstStr - the destination.
* @param strToCpy - string to copy.
* @param numWords the number of words to coppy
**/

void CopyStrinsArry(char*** dstStr, char** strToCpy , int numWords) {

	*dstStr = malloc(sizeof(char**) * numWords);
	int i;
	for (i = 0; i < numWords; i++) {
		(*dstStr)[i] = malloc(sizeof(char*) * ((strlen(strToCpy[i]))+1) );
		if((*dstStr)[i] == NULL){
			printf("Null\n");
			break;
		}
		strcpy((*dstStr)[i] , strToCpy[i]);
	}
}


/**
* function name: FreeFunction
* the function frees strings arry.
* @param str - the strings arry to free
* @param numWords - the words number in the string arry.
**/

void FreeFunction(char*** str, int numWords) {
	int i = 0;
	for (i = 0; i < numWords; i++) {
		free((*str)[i]);
	}
	free(*str);
}

/**
* function name: PrintStringArry
* the function prints string arry.
* @param str - the strings arry to print
* @param numWords - the words number in the string arry.
**/

void PrintStringArry(char **str, int numWords) {
	int i = 0;
	for (i = 0; i < numWords; i++) {
		printf("%s " , str[i]);
	}
	printf("%c" , '\n');


}

/**
* function name: NumWords
* count the words in a given string
* @param str - string to count the words in.
* @return - int wordsNumber.
**/


int NumWords(char* str) {
	char* arg;
	int wordsNumber = 0;
	char* copy = malloc(strlen(str) + 1);
	if (copy == NULL) {
		perror("malloc failed");
		exit(1);
	}
	strcpy(copy, str);
	arg = strtok(copy, "\n, " "");
	// get the other srguments
	while (arg != NULL) {
		wordsNumber++;
		arg = strtok(NULL, "\n, " "");
	}
	free(copy);
	return wordsNumber;
}


void executePwd() {
	pid_t pidForPwd;
	int status;
	char* args[] = {"pwd" , NULL};
	pidForPwd = fork();
	if (pidForPwd == 0) {
		// Child process
		if (execvp("pwd", args) == -1) {
			fprintf (stderr, "unknown command:\n");
			exit(1);
		}
		exit(EXIT_FAILURE);
	} else if (pidForPwd < 0) {
		perror("Fork failed");
		exit(1);
	} else {
		// Parent process
		waitpid(pidForPwd, &status, WUNTRACED);
	}
}



