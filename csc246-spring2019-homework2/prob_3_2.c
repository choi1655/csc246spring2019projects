/* mchoi John M Choi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

volatile int sum = 0;
volatile int max = 0;
volatile int min = 0;

pthread_mutex_t sumLock;
pthread_mutex_t maxLock;
pthread_mutex_t minLock;

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
  * Deals with first 33% of the list.
  * Calculates sum, max, and min for the first 33%.
  *
  * @param arg void pointer
  */
void *firstChunk(void *arg) {
  NumberList *nl = (NumberList *) arg;
  int chunkSize = nl->size / 3;
  for (int i = 0; i < chunkSize; i++) {
    pthread_mutex_lock(&sumLock);
    sum += nl->list[i];
    pthread_mutex_unlock(&sumLock);
    pthread_mutex_lock(&maxLock);
    if (max < nl->list[i]) {
      max = nl->list[i];
    }
    pthread_mutex_unlock(&maxLock);
    pthread_mutex_lock(&minLock);
    if (min > nl->list[i]) {
      min = nl->list[i];
    }
    pthread_mutex_unlock(&minLock);
  }

  if (nl->size % chunkSize == 1) {
    int leftOver = nl->list[nl->size - 1];
    pthread_mutex_lock(&sumLock);
    sum += leftOver;
    pthread_mutex_unlock(&sumLock);
    pthread_mutex_lock(&maxLock);
    if (max < leftOver) {
      max = leftOver;
    }
    pthread_mutex_unlock(&maxLock);
    pthread_mutex_lock(&minLock);
    if (min > leftOver) {
      min = leftOver;
    }
    pthread_mutex_unlock(&minLock);
  } else if (nl->size % chunkSize == 2) {
    int leftOver = nl->list[nl->size - 2];
    pthread_mutex_lock(&sumLock);
    sum += leftOver;
    pthread_mutex_unlock(&sumLock);
    pthread_mutex_lock(&maxLock);
    if (max < leftOver) {
      max = leftOver;
    }
    pthread_mutex_unlock(&maxLock);
    pthread_mutex_lock(&minLock);
    if (min > leftOver) {
      min = leftOver;
    }
    pthread_mutex_unlock(&minLock);
  } else {
    return NULL;
  }
  return NULL;
}

/**
  * Deals with second 33% of the list.
  * Calculates sum, max, and min for the second 33%.
  *
  * @param arg void pointer
  */
void *secondChunk(void *arg) {
  NumberList *nl = (NumberList *) arg;
  int chunkSize = nl->size / 3;
  for (int i = chunkSize; i < chunkSize + chunkSize; i++) {
    pthread_mutex_lock(&sumLock);
    sum += nl->list[i];
    pthread_mutex_unlock(&sumLock);
    pthread_mutex_lock(&maxLock);
    if (max < nl->list[i]) {
      max = nl->list[i];
    }
    pthread_mutex_unlock(&maxLock);
    pthread_mutex_lock(&minLock);
    if (min > nl->list[i]) {
      min = nl->list[i];
    }
    pthread_mutex_unlock(&minLock);
  }

  if (nl->size % chunkSize == 2) {
    int leftOver = nl->list[nl->size - 1];
    pthread_mutex_lock(&sumLock);
    sum += leftOver;
    pthread_mutex_unlock(&sumLock);
    pthread_mutex_lock(&maxLock);
    if (max < leftOver) {
      max = leftOver;
    }
    pthread_mutex_unlock(&maxLock);
    pthread_mutex_lock(&minLock);
    if (min > leftOver) {
      min = leftOver;
    }
    pthread_mutex_unlock(&minLock);
  }
  return NULL;
}

/**
  * Deals with the last 33% of the list.
  * Calculates sum, max, min for the last 33% of the list.
  *
  * @param arg void pointer
  */
void *lastChunk(void *arg) {
  NumberList *nl = (NumberList *) arg;
  int chunkSize = nl->size / 3;
  for (int i = chunkSize + chunkSize, j = 0; j < chunkSize; i++, j++) {
    pthread_mutex_lock(&sumLock);
    sum += nl->list[i];
    pthread_mutex_unlock(&sumLock);
    pthread_mutex_lock(&maxLock);
    if (max < nl->list[i]) {
      max = nl->list[i];
    }
    pthread_mutex_unlock(&maxLock);
    pthread_mutex_lock(&minLock);
    if (min > nl->list[i]) {
      min = nl->list[i];
    }
    pthread_mutex_unlock(&minLock);
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
  max = nl->list[0];
  min = nl->list[0];

  if (pthread_mutex_init(&sumLock, NULL) != 0) {
    fprintf(stderr, "Mutex init failed\n");
    free(nl->list);
    free(nl);
    return EXIT_FAILURE;
  }
  if (pthread_mutex_init(&maxLock, NULL) != 0) {
    fprintf(stderr, "Mutex init failed\n");
    free(nl->list);
    free(nl);
    return EXIT_FAILURE;
  }
  if (pthread_mutex_init(&minLock, NULL) != 0) {
    fprintf(stderr, "Mutex init failed\n");
    free(nl->list);
    free(nl);
    return EXIT_FAILURE;
  }
  pthread_t threads[3];
  pthread_create(&threads[0], NULL, firstChunk, nl);
  pthread_create(&threads[1], NULL, secondChunk, nl);
  pthread_create(&threads[2], NULL, lastChunk, nl);
  for (int i = 0; i < 3; i++) {
    pthread_join(threads[i], NULL);
  }
  free(nl->list);
  free(nl);
  pthread_mutex_destroy(&sumLock);
  pthread_mutex_destroy(&maxLock);
  pthread_mutex_destroy(&minLock);
  fprintf(stdout, "The total sum of all numbers is %5d\n", sum);
  fprintf(stdout, "The max value of all numbers is %5d\n", max);
  fprintf(stdout, "The min value of all numbers is %5d\n", min);
	return EXIT_SUCCESS;
}
