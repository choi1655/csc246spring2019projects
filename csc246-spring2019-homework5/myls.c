/** mchoi John M Choi */
/**
  * @file myls.c
  * @author John Choi mchoi
  *
  * CSC246 Homework 5 Part 3
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define ER_MSG "Invalid directory"

/**
  * Prints failure message and terminates.
  *
  * @param msg - error message
  */
void fail(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

/**
  * Prints usage information and terminates.
  */
void usage() {
  fprintf(stderr, "usage: ./myls [-l] [PATH TO DIRECTORY]\n");
  exit(EXIT_FAILURE);
}

/**
  * Removes the new line generated by the ctime() function.
  */
char *removeNewline(void *stringArg) {
  char *string = (char *)stringArg;
  char *returnVal = (char *)malloc(1024);
  int i;
  for (i = 0; i < strlen(string) - 1; i++) {
    returnVal[i] = string[i];
  }
  returnVal[strlen(string) - 1] = '\0';
  return returnVal;
}

/**
  * Returns permission in string form.
  *
  * @param st_mode permission in mode_t
  * @return permission in string
  */
char *makePermission(mode_t st_mode) {
  char *permission = (char *)malloc(1024);
  strcpy(permission, "");

  if ((st_mode & S_IFMT) == S_IFDIR) {
    strcat(permission, "d");
  } else {
    strcat(permission, "-");
  }

  strcat(permission, st_mode & S_IRUSR ? "r" : "-"); // read right for usr
  strcat(permission, st_mode & S_IWUSR ? "w" : "-"); // write right for usr
  strcat(permission, st_mode & S_IXUSR ? "x" : "-"); // execute right for usr

  strcat(permission, st_mode & S_IRGRP ? "r" : "-"); // read right for group
  strcat(permission, st_mode & S_IWGRP ? "w" : "-"); // write right for group
  strcat(permission, st_mode & S_IXGRP ? "x" : "-"); // execute right for group

  strcat(permission, st_mode & S_IROTH ? "r" : "-"); // read right for others
  strcat(permission, st_mode & S_IWOTH ? "w" : "-"); // write right for others
  strcat(permission, st_mode & S_IXOTH ? "x" : "-"); // execute right for others

  return permission;
}

/**
  * Handles simple ls command.
  * Prints all contents including files and directories.
  *
  * @param path to scan
  */
void noarg(const char *path) {
  // Structs
  DIR *dp;
  struct dirent *files;

  dp = opendir(path);
  if (!dp) {
    fail(ER_MSG);
  }

  while ((files = readdir(dp)) != NULL) {
    fprintf(stdout, "%s\n", files->d_name);
  }

  closedir(dp);
}

/**
  * Handles ls -l command.
  * Prints permissions, hard links, owners, etc.
  *
  * @param path to scan
  */
void arg(const char *path) {
  // Structs
  DIR *dp;
  struct stat buf;
  struct dirent *files;
  struct passwd *owner;
  struct group *group;

  int statResult;
  dp = opendir(path);
  if (!dp) {
    fail(ER_MSG);
  }

  // make a detailed path
  char *detailedPath = (char *)malloc(1024);

  while ((files = readdir(dp)) != NULL) {
    // first, make a detailed path
    strcpy(detailedPath, path);
    strcat(detailedPath, "/");
    strcat(detailedPath, files->d_name);

    // run stat with detailed path and populate struct stat
    statResult = stat(detailedPath, &buf);
    if (statResult == -1) {
      perror("stat");
    }

    // decode permission
    char *permission = makePermission(buf.st_mode);
    fprintf(stdout, "%s ", permission); // print permission
    free(permission);
    fprintf(stdout, "%3ld ", buf.st_nlink); // print hard links

    owner = getpwuid(buf.st_uid);
    fprintf(stdout, "%s ", owner->pw_name); // print owner

    group = getgrgid(buf.st_gid);
    fprintf(stdout, "%s ", group->gr_name); // print group

    fprintf(stdout, "%10zu ", buf.st_size); // print size

    char *date = removeNewline(ctime(&buf.st_mtime));
    fprintf(stdout, "%s ", date); // print date
    free(date);
    fprintf(stdout, "%s\n", files->d_name); // print filename
  }

  free(detailedPath);
  closedir(dp);
}

/**
  * Entry point to this program.
  * Determines if there is optional flag or path passed in and calls
  * its subroutines.
  *
  * @param argc num of arguments
  * @param argv list of arguments
  */
int main(int argc, const char **argv) {
  if (argc > 3) {
    usage();
  }
  bool optionalPath, optionalFlag;
  if (argc == 1) {
    optionalPath = false;
    optionalFlag = false;
  } else if (argc == 2) {
    if (strcmp(argv[1], "-l") == 0) {
      optionalFlag = true;
      optionalPath = false;
    } else {
      optionalFlag = false;
      optionalPath = true;
    }
  } else { // if argc is 3
    if (strcmp(argv[1], "-l") == 0) {
      optionalFlag = true;
      optionalPath = true;
    } else {
      usage();
    }
  }

  if (optionalPath) {
    if (optionalFlag) {
      arg(argv[2]);
    } else {
      noarg(argv[1]);
    }
  } else {
    char *currentPath = (char *)malloc(1024);
    getcwd(currentPath, 1024);
    if (optionalFlag) {
      arg(currentPath);
    } else {
      noarg(currentPath);
    }
    free(currentPath);
  }

  return EXIT_SUCCESS;
}
