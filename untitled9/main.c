#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <string.h>
#include <signal.h>


struct job {
	int argumentsNUm;
	char** name;
	int pid;
};


int CommandLoop();
char** getAndSplitUserInput(char** userInput);
int NumWords(char* str);
enum BuiltInCommand CheckCommand(char **args, int numWords);
int ExecuteCommand(char ***args, char **userInput, int numWords);
void Print(char** str, int n);
void FreeFunction(char*** str, int n);
int ExecuteBuiltInCommands(char ***args, enum BuiltInCommand command, struct job *jobs_list, int *jobs_list_size,
						   char **userInput, int numWords);
void ExecuteBackground(char ***args, int numWords, struct job *jobs_list, int *jobs_list_size, char **userInput);
void CopyStrinsArry(char*** dstStr, char** strToCpy , int numWords);
enum CommandType CheckIfBackground(char*** args , int numWords);
void BackgroundJobStatus(struct job *jobs_list, int *jobs_list_size);



enum BuiltInCommand{REG , CD , JOBS, EXIT};
enum CommandType{FOREGOUND, BACKGROUND};



int main() {
	CommandLoop();
	return 0;
}


int CommandLoop() {
	char *userInput = NULL;
	char **userSplitInput = NULL;
	int flag = 1;
	int jobs_list_size = 0;
	int i = 0;
	struct job jobs_list[50];
	enum BuiltInCommand commandKind;
	enum CommandType commandType;
	printf("press exit to finish\n");
while(1){
		printf("prompt>");
		userSplitInput = getAndSplitUserInput(&userInput);
		int numWords = NumWords(userInput);
		commandType = CheckIfBackground(&userSplitInput, numWords);
		commandKind = CheckCommand(userSplitInput, numWords);
		if (commandType == BACKGROUND) {
			numWords--;
			ExecuteBackground(&userSplitInput, numWords, jobs_list, &jobs_list_size, &userInput);
			usleep(50000);
		} else {
			if (commandKind == REG) {
				ExecuteCommand(&userSplitInput, &userInput, numWords);
			} else {
				if (commandKind == EXIT) {
					FreeFunction(&userSplitInput, numWords+1);
					free(userInput);
					for (i = 0; i < jobs_list_size; i++) {
						FreeFunction(&(jobs_list[i].name), jobs_list[i].argumentsNUm);
					}
					exit(1);
				}
				ExecuteBuiltInCommands(&userSplitInput, commandKind, jobs_list, &jobs_list_size, &userInput, numWords);
			}
		}
	}
}




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



int ExecuteCommand(char ***args, char **userInput, int numWords)
{
	pid_t child_pid;
	int status;
	child_pid = fork();
	if (child_pid == 0) {
		printf("%ld\n" , (long)getpid());
		// Child process
		if (execvp((*args)[0], *args) == -1) {
			fprintf (stderr, "unknown command: %s\n", (*args)[0]);
			exit(1);
		}
		exit(EXIT_FAILURE);
	} else if (child_pid < 0) {
		perror("Fork failed");
		exit(1);
	} else {
		// Parent process
		waitpid(child_pid, &status, WUNTRACED);
		FreeFunction(&(*args), numWords);
		free(*userInput);
	}
	return 1;
}


void BackgroundJobStatus(struct job *jobs_list, int *jobs_list_size) {
	int i;
	int status;
	int j;
	int k = 0;
	for (i = 0; i < *(jobs_list_size); i++) {
		pid_t return_pid = waitpid((jobs_list)[i].pid , &status , WNOHANG);
		if(return_pid == -1) {
			//error;
		} else if (return_pid == 0) {
			//still running
		} else if (return_pid == (jobs_list)[i].pid) {
			for (k = 0; k < (jobs_list)[i].argumentsNUm; k++) {
				free(((jobs_list)[i]).name[k]);
			}
			free(((jobs_list)[i]).name);
			//rearange the arry
			for(j = i; j < (*jobs_list_size) - 1; j++) {
				(jobs_list)[j] = (jobs_list)[j + 1];
			}
			(*jobs_list_size)--;
		}
	}
}



void ExecuteBackground(char ***args, int numWords, struct job *jobs_list, int *jobs_list_size, char **userInput)
{ int pid;
	pid = fork();
	if (pid == 0)
	{
		fclose(stdin); // close child's stdin
		fopen("/dev/null", "r"); // open a new stdin that is always empty
		//fprintf(stderr, "Child Job pid = %d\n", getpid());
		//add pid to jobs list
		execvp((*args)[0], *args);
		// this should never be reached, unless there is an error
		fprintf (stderr, "unknown command: %s\n", (*args)[0]);
		exit(1);
	} else {
		jobs_list[*jobs_list_size].pid = pid;
		jobs_list[*jobs_list_size].argumentsNUm = numWords;
		CopyStrinsArry(&jobs_list[*jobs_list_size].name , (*args) , numWords);
		(*jobs_list_size)++;
	}
	FreeFunction(&(*args) , numWords);
	free(*userInput);
}



int ExecuteBuiltInCommands(char ***args, enum BuiltInCommand command, struct job *jobs_list, int *jobs_list_size,
						   char **userInput, int numWords) {
	//check if this success.
	int i;
	char cdArgs[50];
	if (command == CD) {

		if ((*args)[1] == NULL) {
			FreeFunction(&(*args) , numWords);
			free(*userInput);
			return chdir(getenv("HOME"));
		} else {
			for (i = 0; i < strlen((*args)[1])+1 ; i++) {
				cdArgs[i] = (*args)[1][i];
			}
		}
		FreeFunction(&(*args) , numWords);
		free(*userInput);
		if(chdir(cdArgs) < 0) {
			fprintf (stderr, "unknown command: %s\n", (*args)[0]);
		}
	} else if (command == JOBS) {
		BackgroundJobStatus(jobs_list, jobs_list_size);
		for (i = 0; i < (*jobs_list_size); i++) {
				printf("%d\t\t" , jobs_list[i].pid);
			Print(jobs_list[i].name , jobs_list[i].argumentsNUm);
		}
		FreeFunction(&(*args) , numWords);
		free(*userInput);
	}
}



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


void FreeFunction(char*** str, int n) {
	int i = 0;
	for (i = 0; i < n; i++) {
		free((*str)[i]);
	}
	free(*str);
}

void Print(char** str, int n) {
	int i = 0;
	for (i = 0; i < n; i++) {
		printf("%s " , str[i]);
	}
	printf("%c" , '\n');


}



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



