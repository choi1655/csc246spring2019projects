/* mchoi John M Choi */

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_TOKENS 6
#define MAX_STRLEN 1024

/* Every simple command has one of these associated with it */
struct simple {
	char* token[MAX_TOKENS]; /* tokens of the command */
	int count; /* the number of tokens */ /* count - 2 gives the last argument */
}cmd_inst;

/* Single node in the alias data structure */
typedef struct {
	int count; /* the number of original tokens */
	struct simple aliasPointer; /* original command */
	char *alias; /* Alias command */
} AliasNode;

int size = 0; /* Current size of aliases */
AliasNode *aliases; /* Alias data structure */

struct simple *parseSequence();

/**
  * Used to clear memory before program termination.
  */
void clearAliases() {
  if (size == 0) {
    return;
  }
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < aliases[i].aliasPointer.count; j++) {
			free(aliases[i].aliasPointer.token[j]);
		}
		free(aliases[i].alias);
	}
	//free(aliases);
}

bool aliasExists(const char *command) {
	for (int i = 0; i < size; i++) {
		if (strcmp(command, aliases[i].alias) == 0) {
			return true;
		}
	}
	return false;
}

/**
  * Adds alias to the data structure.
  *
  * @param command pointer
  */
void addAlias(struct simple *command) {
  if (size >= 8) {
    fprintf(stdout, "Already have 8 aliases\n");
    return;
  }
	if (aliasExists(command->token[1])) {
		fprintf(stdout, "Alias %s exists\n", command->token[1]);
		return;
	}
  AliasNode *newNode = (AliasNode *)malloc(sizeof(AliasNode));
  struct simple newSimple = cmd_inst;
  memset(&newSimple, 0, sizeof(struct simple));
  newSimple.count = command->count;
  for (int i = 0; i < newSimple.count; i++) {
    if (command->token[i] != NULL) {
      newSimple.token[i] = (char *)malloc(MAX_STRLEN);
      strcpy(newSimple.token[i], command->token[i]);
    } else {
      newSimple.token[i] = NULL;
    }
  }
  newNode->aliasPointer = newSimple;
  newNode->alias = (char *)malloc(MAX_STRLEN);
  strcpy(newNode->alias, command->token[1]);
  //aliases[size] = *newNode;
  memcpy(&aliases[size], newNode, sizeof(AliasNode));
  size++;
  free(newNode);
}

/**
  * Used for "alias ll" command to search and print ll information.
  *
  * @param command pointer
  */
void searchAlias(struct simple *command) {
  for (int i = 0; i < size; i++) {
    if (strcmp(command->token[1], aliases[i].alias) == 0) {
      command = &aliases[i].aliasPointer;
      char *token = (char *)malloc(MAX_STRLEN);
      int i = 1;
      while (command->token[i] != NULL) {
        strcpy(token, command->token[i]);
        fprintf(stdout, "%-10s", token);
        i++;
      }
      fprintf(stdout, "%-10s\n", "(null)");
      free(token);
      return;
    }
  }

  // Alias doesnt exist
  fprintf(stdout, "ERROR: no such alias\n");
}

/**
  * Used for "ll" commands.
  * Checks to see if alias command is valid.
  *
  * @param command pointer
  */
void validatePossibleAlias(struct simple *command) {
  for (int i = 0; i < size; i++) {
    if (strcmp(command->token[0], aliases[i].alias) == 0) {
      bool argumentExists = false;
      char *optionalArgument[MAX_TOKENS];
      int optionalArgCounter = command->count - 2;
      if (command->count > 2) { // there is optional argument
        argumentExists = true;
        for (int i = 0; i < optionalArgCounter; i++) {
          optionalArgument[i] = (char *)malloc(MAX_STRLEN);
          strcpy(optionalArgument[i], command->token[i + 1]);
        }
      }
      struct simple *newSimple = &cmd_inst;
      memset(newSimple, 0, sizeof(struct simple));
      newSimple->count = aliases[i].aliasPointer.count - 2;
      int j = 2;
      int k = 0;
      while (aliases[i].aliasPointer.token[j] != NULL) {
        newSimple->token[k] = (char *)malloc(MAX_STRLEN);
        strcpy(newSimple->token[k++], aliases[i].aliasPointer.token[j++]);
      }
      memcpy(command, newSimple, sizeof(struct simple));
      if (argumentExists) {
        for (int i = command->count - 1, j = 0; j < optionalArgCounter; i++, j++) {
          command->token[i] = (char *)malloc(MAX_STRLEN);
          strcpy(command->token[i], optionalArgument[j]);
        }
        command->count += optionalArgCounter;
      }
      for (int i = 0; i < optionalArgCounter; i++) {
        free(optionalArgument[i]);
      }
      return;
    }
  }
}

/**
  * Lists all the alias commands.
  */
void listAliases() {
  for (int i = 0; i < size; i++) {
    AliasNode current = aliases[i];
    int i = 1;
    char *token = (char *)malloc(MAX_STRLEN);
    while (current.aliasPointer.token[i] != NULL) {
      strcpy(token, current.aliasPointer.token[i]);
      fprintf(stdout, "%-10s", token);
      i++;
    }
    fprintf(stdout, "%-10s\n", "(null)");
    free(token);
  }
}

/**
  * Performs the alias command.
  * Does nothing if command is not alias.
  *
  * @param command the struct with the parsed command
  * @return true if command is alias
  */
bool aliasCommand(struct simple *command) {
  bool opDone = false;
  if (strcasecmp("alias", command->token[0]) == 0) {
    opDone = true;
    if (command->count == 2) { // token: ["alias", NULL]
      listAliases();
    } else if (command->count == 3) { // token: ["alias", "ll", NULL]
      searchAlias(command);
    } else { // token: ["alias", "ll", "ls", "-al", NULL]
      addAlias(command);
    }
  }
  return opDone;
}

void freeCommandTokens(struct simple *command) {
  for (int i = 0; i < command->count; i++) {
    if (command->token[i] != NULL) {
      command->token[i] = 0x0;
    }
  }
}

/**
  * Driver function for this program.
  *
  * @param argc number of arguments
  * @param argv arguments list
  * @return 0 if successful
  */
int main(int argc, char *argv[]) {
	struct simple *command;
  aliases = (AliasNode *)malloc(8 * sizeof(AliasNode));

  int seenExit = 0; /* flag to determine when to exit program */
  char *line = (char *)malloc(MAX_STRLEN);
  //char *line2 = (char *)malloc(MAX_STRLEN);
  while (1) {
    printf("> ");
    fflush(stdout);

    //fgets(line, MAX_STRLEN, stdin);
    fscanf(stdin, "%[^\n]", line);
    fscanf(stdin, "%*c");
    //strcpy(line2, line);
    command = parseSequence(line);
    if (command->count == 1) {
      fflush(stdin);
      //freeCommandTokens(command);
      continue;
    }
    if (strcasecmp("exit", command->token[0]) == 0) {
      fflush(stdin);
      //freeCommandTokens(command);
      break;
    }
    if (aliasCommand(command)) {
      //free(line);
      fflush(stdin);
      //freeCommandTokens(command);
      continue;
    }
    validatePossibleAlias(command);

    /* NEEDS TO HAVE COMMAND GOOD TO GO AFTER THIS LINE */

    /**
    * After reading user input, the steps are:
    * (1) fork a child process using fork()
    * (2) the child process will invoke execvp()
    * (3) parent will invoke wait() unless command included &
    */

    pid_t forkVal = fork();
      if (forkVal < 0) {
        fprintf(stderr, "error while forking\n");
        exit(EXIT_FAILURE);
      }
      if (forkVal == 0) {
        // child process
        if (execvp(command->token[0], command->token) == -1) {
          free(line);
          clearAliases();
          free(aliases);
          freeCommandTokens(command);
          exit(EXIT_FAILURE);
        }
        free(line);
        clearAliases();
        free(aliases);
        freeCommandTokens(command);
        exit(EXIT_SUCCESS);
      } else {
        // parent process
        waitpid(forkVal, &seenExit, 0);
        int exitStatusVal = WEXITSTATUS(seenExit);
        int exit = WIFEXITED(exitStatusVal);
        if(!exit){
          //printf( "Child did not terminate successfully");
          fprintf(stdout, "ERROR: %s return %d\n", command->token[0], exitStatusVal);
        }
      }
      fflush(stdin);
      freeCommandTokens(command);
  } // end of while loop
  free(line);
	clearAliases();
	free(aliases);
	return 0; /* exit success */
}

/*
"parseSequence" function is used to parse the char line got from the
standard input into the simple structure to pass arguments into system
calls later.
*/
struct simple *parseSequence(char * line) {
	int i, t;
	struct simple *c = &cmd_inst;
	memset(c, 0, sizeof(struct simple));

	t = 0;

	i = 0;
	while (isspace(line[i]))
		i++;
	c->token[t] = &line[i];

	while (line[i] != '\0' && t < MAX_TOKENS - 1) {
		t++;

		while (!isspace(line[i]) && line[i] != '\0')
			i++;

		while (isspace(line[i])) {
			line[i] = '\0';
			i++;
		}

		c->token[t] = &line[i];
	}
	c->count = t + 1;
	c->token[t] = NULL;

	return c;
}
