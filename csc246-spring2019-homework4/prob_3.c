/** mchoi John M Choi */
/**
  * @file proc_3.c
  * @author John Choi mchoi
  *
  * CSC246 Assignment 4 part 3.
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/** global 2D array used to store the page memory address from the input file */
char *memory[128][32];

/**
  * Prints the usage information and exits the program.
  */
void usage() {
  fprintf(stderr, "usage: ./proc_3 [PATH_TO_FILE] [DECIMAL_NUM] [HEXADECIMAL_NUM]\n");
  exit(EXIT_FAILURE);
}

/**
  * Prints failure message and quits.
  *
  * @param msg - message to print
  */
void fail(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

/**
  * Opens and reads file and stores it to the data structure.
  *
  * @param filename - name of the file to open
  */
void readFile(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fail("Cannot open file");
  }
  //char *byte = (char *)malloc(2);
  for (int i = 0; i < 128; i++) {
    fscanf(fp, "%*s %*s"); //read page and line number in the beginning of line
    for (int j = 0; j < 32; j++) {
      memory[i][j] = (char *)malloc(2);
      fscanf(fp, "%s", memory[i][j]);
    }
  }
  fclose(fp);
}

/**
  * Frees memory for the memory table.
  */
void clearMemory() {
  for (int i = 0; i < 128; i++) {
    for (int j = 0; j < 32; j++) {
      free(memory[i][j]);
    }
  }
}

/**
  * Simply prints i number of spaces.
  */
void printSpaces(int i) {
  for (int j = 0; j < i; j++) {
    fprintf(stdout, " ");
  }
}

/**
  * Opens and closes a file for validity.
  */
bool validFile(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    return false;
  }
  int vCounter = 0;
  char *line = (char *)malloc(2048);
  while (fscanf(fp, "%[^\n]\n]", line) != EOF) {
    vCounter++;
  }

  free(line);
  fclose(fp);
  return vCounter == 128;
}

/**
  * Entry point for this program.
  */
int main(int argc, const char **argv) {
  if (argc != 4) {
    usage();
  }
  int pdbr = atoi(argv[2]);
  if (pdbr < 0 || pdbr > 127) {
    fail("invalid PDBR number");
  }
  if (strlen(argv[3]) < 3 || (argv[3][0] != '0' || argv[3][1] != 'x')) {
    fail("invalid hexadecimal number");
  }
  char *trimmedNum = (char *)malloc(strlen(argv[3]) - 1);
  for (int i = 0; i < strlen(argv[3]) - 2; i++) {
    trimmedNum[i] = argv[3][i + 2];
  }
  trimmedNum[strlen(argv[3] - 1)] = '\0';
  int vAddress = (int) strtol(trimmedNum, NULL, 16); // number base 16
  free(trimmedNum);
  if (!validFile(argv[1])) {
    fail("invalid file format");
  }
  readFile(argv[1]);

  // do work
  int vAddressTemp = vAddress;
  int firstFive, secondFive, thirdFive;
  firstFive = vAddressTemp >> 10;
  vAddressTemp &= 0x03ff;
  secondFive = vAddressTemp >> 5;
  vAddressTemp &= 0x001f;
  thirdFive = vAddressTemp;

  fprintf(stdout, "Virtual Address: %s\n", argv[3]);

  /** 1) go to page [pdbr]
      2) find firstFive's element in that page
      3) set pdbr to that element in int form
      4) see if valid bit is 1
      5) repeat
  */
  // pde
  int num = (int) strtol(memory[pdbr][firstFive], NULL, 16);
  int valid = num >> 7;
  int pfn = num & 0x7f;
  printSpaces(2);
  if (valid == 0) { // check for a fault
    fprintf(stdout, "--> Fault (page table entry not valid)\n");
    clearMemory();
    return EXIT_FAILURE;
  }
  fprintf(stdout, "--> pde index:0x%x [decimal %d] pde contents:0x%s (valid %d, pfn 0x%x [decimal %d])\n", firstFive, firstFive, memory[pdbr][firstFive], valid, pfn, pfn);
  pdbr = pfn;

  // pte
  num = (int) strtol(memory[pdbr][secondFive], NULL, 16);
  valid = num >> 7;
  pfn = num & 0x7f;
  printSpaces(4);

  fprintf(stdout, "--> pte index:0x%x [decimal %d] pte contents:0x%s (valid %d, pfn 0x%x [decimal %d])\n", secondFive, secondFive, memory[pdbr][secondFive], valid, pfn, pfn);
  if (valid == 0) {
    printSpaces(6);
    fprintf(stdout, "--> Fault (page table entry not valid)\n");
    clearMemory();
    return EXIT_FAILURE;
  }
  pdbr = pfn;

  //physical address
  int vpn = pdbr;
  int offset = vAddress & 0x1f;
  //int value = (int) strtol(memory[pdbr][thirdFive], NULL, 16);
  printSpaces(6);
  int physicalAddress = vpn << 5;
  physicalAddress |= offset;
  fprintf(stdout, "--> Translates to Physical Address 0x%x --> Value: %s\n", physicalAddress, memory[pdbr][thirdFive]);

  // clear memory before exiting
  clearMemory();
  return EXIT_SUCCESS;
}
