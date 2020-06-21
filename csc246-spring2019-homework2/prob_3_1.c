/* mchoi John M Choi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

volatile int sum = 0;
volatile int max = 0;
volatile int min = 0;

/* Struct with list of numbers and size from the input file */
typedef struct {
  int size;
  int *list;
} NumberList;

/**
  * Prints out usage information and terminates program.
  */
void usage() {
  fprintf(stdout, "usage: ./prob_3_1 filename\n");
  exit(EXIT_FAILURE);
}

/**
  * Thread driver that calculates sum of all numbers and prints it.
  *
  * @param arg void pointer argument
  * @return void pointer
  */
void *calculateSum(void *arg) {
  NumberList *nl = (NumberList *) arg;
  sum = nl->list[0];
  for (int i = 1; i < nl->size; i++) {
    sum += nl->list[i];
  }
  return NULL;
}

/**
  * Thread driver that calculates the maximum number in the list and reports it.
  *
  * @param arg void pointer argument
  * @return void pointer
  */
void *calculateMax(void *arg) {
  NumberList *nl = (NumberList *) arg;
  max = nl->list[0];
  for (int i = 1; i < nl->size; i++) {
    if (max < nl->list[i]) {
      max = nl->list[i];
    }
  }
  return NULL;
}

/**
  * Thread driver that calculates the minimum number in the list and reports it.
  *
  * @param arg void pointer argument
  * @return void pointer
  */
void *calculateMin(void *arg) {
  NumberList *nl = (NumberList *) arg;
  min = nl->list[0];
  for (int i = 0; i < nl->size; i++) {
    if (min > nl->list[i]) {
      min = nl->list[i];
    }
  }
  return NULL;
}

/**
  * Returns a pointer to the struct with list of numbers and size.
  *
  * @param fp file pointer of input file
  * @return nl pointer to the struct with list of numbers
  */
NumberList *populateList(FILE *fp) {
  int counter = 0;
  int capacity = 5;
  int *list = (int *)calloc(capacity, sizeof(int));
  int number;
  int matches = fscanf(fp, "%d", &number);
  if (matches != 1) {
    free(list);
    fprintf(stderr, "Empty file\n");
    fclose(fp);
    exit(EXIT_FAILURE);
  }
  while (matches != EOF) {
    if (counter >= capacity) {
      capacity *= 2;
      int *newList = (int *)calloc(capacity, sizeof(int));
      for (int i = 0; i < counter; i++) {
        newList[i] = list[i];
      }
      free(list);
      list = (int *)calloc(capacity, sizeof(int));
      for (int i = 0; i < counter; i++) {
        list[i] = newList[i];
      }
      free(newList);
    }
    list[counter++] = number;
    matches = fscanf(fp, "%d", &number);
    if (matches != 1 && matches != EOF) {
      free(list);
      fclose(fp);
      fprintf(stderr, "Invalid input detected\n");
      exit(EXIT_FAILURE);
    }
  }
  NumberList *nl = (NumberList *)malloc(sizeof(NumberList));
  nl->size = counter;
  nl->list = list;
  fclose(fp);
  return nl;
}

/**
  * Driver function for this program. Takes arguments.
  *
  * @param argc number of arguments
  * @param argv list of arguments
  * @return EXIT_SUCCESSFUL if program terminates correctly
  */
int main(int argc, char **argv) {
  if (argc != 2) {
    usage();
  }
  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "Cannot open file\n");
    return EXIT_FAILURE;
  }

  NumberList *nl = populateList(fp);
  pthread_t threads[3];
  pthread_create(&threads[0], NULL, calculateSum, nl);
  pthread_create(&threads[1], NULL, calculateMax, nl);
  pthread_create(&threads[2], NULL, calculateMin, nl);
  for (int i = 0; i < 3; i++) {
    pthread_join(threads[i], NULL);
  }

  free(nl->list);
  free(nl);

  fprintf(stdout, "The total sum of all numbers is %5d\n", sum);
  fprintf(stdout, "The max value of all numbers is %5d\n", max);
  fprintf(stdout, "The min value of all numbers is %5d\n", min);
	return EXIT_SUCCESS;
}
