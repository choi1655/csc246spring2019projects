/** mchoi John M Choi */
/** ewmcalli Eric W McAllister */
/**
  * @file defrag.c
  * @version 04202019
  *
  * CSC246 Homework 6 Problem 2
  */


/*4 blocks = 4 - 0
2048 bytes = 512 * 4
-----
100 ~= 20*/


#define _POSIX_C_SOURCE 200809L // for POSIX strdup()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#define N_DBLOCKS 10
#define N_IBLOCKS 4
#define INT_SIZE 4
#define BYTE_SIZE 8
int new_index = 0;

/**
 * Superblock structure (given)
 */
typedef struct {
    int blocksize; /* size of blocks in bytes */
    int inode_offset; /* offset of inode region in blocks */
    int data_offset; /* data region offset in blocks */
    int swap_offset; /* swap region offset in blocks */
    int free_inode; /* head of free inode list */
    int free_block; /* head of free block list*/
} superblock;

typedef struct {
    int next_inode; /* list for free inodes */
    int protect; /* protection field */
    int nlink; /* Number of links to this file */
    int size; /* Number of bytes in file */
    int uid; /* Owner's user ID */
    int gid; /* Owner's group ID */
    int ctime; /* Time field */
    int mtime; /* Time field */
    int atime; /* Time field */
    //struct stat s;
    int dblocks[N_DBLOCKS]; /* Pointers to data blocks */
    int iblocks[N_IBLOCKS]; /* Pointers to indirect blocks */
    int i2block; /* Pointer to doubly indrect block */
    int i3block; /* Pointer to triply indirect block */ 
} inode;

/** Linked list node data structure */
struct NodeTag {
  inode value;
  struct NodeTag *next;
};

typedef struct NodeTag Node;

typedef struct {
  Node *head;
  int size;
} LinkedList;

/**
  * Inserts inode into LinkedList.
  * Sorts inode by its size for now.
  */
void insert(LinkedList *list, inode val) {
  Node **link = &(list->head);
  Node *n = (Node *)malloc(sizeof(Node));
  n->value = val;
  n->next = *link;
  list->head = n;
  list->size++;
}

void insertSorted(LinkedList *list, inode val) {
  Node **link = &(list->head);
  
  while((*link) && val.next_inode <= (*link)->value.next_inode) { //TODO sort by inode's size for now
    link = &(*link)->next;
  }
  
  Node *n = (Node *)malloc(sizeof(Node));
  n->value = val;
  
  n->next = (*link);
  *link = n;
  list->size++;
}

/**
  * Prints failure message and terminates program.
  *
  * @param msg - failure message
  */
void fail(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

/**
  * Prints usage information and terminates program.
  */
void usage() {
	fprintf(stderr, "usage: ./defrag [FILE_NAME]\n");
	exit(EXIT_FAILURE);
}

/**
  * Reading a word from arbitrary address.
  * Code taken from the homework writeup.
  */
int readIntAt(unsigned char *p) {
  return *(p + 3) * 256 * 256 * 256 + *(p + 2) * 256 * 256 + *(p + 1) * 256 + *p;
}

/**
  * Writing int to a buffer.
  * Taken from Moodle.
  */
void writeIntAt(unsigned char *p, int v) {
  *(p + 3) = (char) (v >> BYTE_SIZE * 3);
  *(p + 2) = (char) (v >> BYTE_SIZE * 2);
  *(p + 1) = (char) (v >> BYTE_SIZE);
  *p = (char) v;
}

int dblockSize(inode *n) {
  int num = 0;
  for (int i = 0; i < 10; i++) {
    if (n->dblocks[i] != -1) {
      num++;
    }
  }
  return num;
}

/**
  * Returns the number of inodes.
  */
int calculateInodeSize(superblock *sb) {
  int size = 0;
  int offset = sb->data_offset - sb->inode_offset;
  size = sb->blocksize * offset;
  size /= 100;
  return size;
}

bool isValid(inode *n) {
  if(n->nlink == 0) { 
    return false;
  } 
  return true;
}

bool compareInode(inode* a, inode* b) {
  if(a->next_inode == b->next_inode && a->protect == b->protect && a->nlink == b->nlink && 
  a->size == b->size && a->uid == b->uid && a->ctime == b->ctime) {
    return true; 
  }
  return false;
}

void overwrite_block(int old_index, int numberOfElem, superblock *psb, unsigned char *new_buffer, unsigned char *old_buffer) {
  for(int i = 0; i < numberOfElem; i++) {
    // int a = readIntAt(old_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * old_index) + (i * sizeof(int)));
    writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * new_index) + (i * sizeof(int)), readIntAt(old_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * old_index) + (i * sizeof(int))));
    // int b = readIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * new_index) + (i * sizeof(int)));
  }
}

void indirect_block(int numberOfElem, superblock *psb, unsigned char *new_buffer, unsigned char *old_buffer) {
  int inThisBlock = new_index;
  new_index++;
  for(int i = 0; i < numberOfElem; i++) {
    int r = readIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * inThisBlock) + (i * sizeof(int)));
    if(r != -1) {
      int dbNum = r;
      writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * inThisBlock) + (i * sizeof(int)), new_index);
      //PLACE DATA FROM DBNUM into new_index
      overwrite_block(dbNum, numberOfElem, psb, new_buffer, old_buffer);
      new_index++;
    }
  }
}

void indrect_block_two(int numberOfElem, superblock *psb, unsigned char *new_buffer, unsigned char *old_buffer) {
  int i2block = new_index;
  for(int i = 0; i < numberOfElem; i++) {
    new_index++;
    int i1block = readIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * i2block) + (i * sizeof(int)));
    if(i1block == -1) {
      break;
    }
    //Give i1block a new number
    writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * i2block) + (i * sizeof(int)), new_index);
    //End i1block re-figure now to rewrite contents in i blocked one
    overwrite_block(i1block, numberOfElem, psb, new_buffer, old_buffer);
    //DONE WITH i1block NOW!!
    int location = new_index; //block 1161
    for(int j = 0; j < numberOfElem; j++) {
      int direct = readIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * location) + (j * sizeof(int)));
      if(direct == -1) {
        break;
      }
      new_index++;
      writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * location) + (j * sizeof(int)), new_index);
      overwrite_block(direct, numberOfElem, psb, new_buffer, old_buffer);
    }
  }
}

void indrect_block_three(int numberOfElem, superblock *psb, unsigned char *new_buffer, unsigned char *old_buffer) {
  int i3block = new_index;
  //Each element in i3block
  for(int i = 0; i < numberOfElem; i++) {
    new_index++;
    int i2block = readIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * i3block) + (i * sizeof(int)));
    if(i2block == -1) {
      break;
    }
    writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * i3block) + (i * sizeof(int)), new_index);
    overwrite_block(i2block, numberOfElem, psb, new_buffer, old_buffer);
    int location = new_index;
    //Each element in i2block
    for(int j = 0; j < numberOfElem; j++) {
      new_index++;
      int i1block = readIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * location) + (j * sizeof(int)));
      if(i1block == -1) {
        break;
      }
      writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * location) + (j * sizeof(int)), new_index);
      overwrite_block(i1block, numberOfElem, psb, new_buffer, old_buffer);
      int deepLocation = new_index;
      //Each element in that direct block
      for(int k = 0; k < numberOfElem; k++) {
        new_index++;
        int direct = readIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * deepLocation) + (k * sizeof(int)));
        if(direct == -1) {
          new_index++;
          return;
        }
        writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * deepLocation) + (k * sizeof(int)), new_index);
        overwrite_block(direct, numberOfElem, psb, new_buffer, old_buffer);

      }
    }
  }
}

/**
  * Entry point of this program.
  *
  * @param argc number of arguments
  * @param argv list of arguments
  */
int main(int argc, const char **argv) {
	if (argc != 2) {
		usage();
	}
	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		fail("Can't open file");
	}

	// open output file
	FILE *output = fopen("disk_defrag", "wb");

	struct stat fileMeta;
	char *detailedPath = (char *) strdup("./");
	strcat(detailedPath, argv[1]);
	if (stat(detailedPath, &fileMeta) != 0) {
	  free(detailedPath);
	  fclose(fp);
	  fail("Error with stat()");
	}
	
	off_t fileSize = fileMeta.st_size; // size of file in bytes
	// make buffer exactly that size
	unsigned char *buffer = (unsigned char *)malloc(fileSize);
	unsigned char *new_buffer = (unsigned char *)malloc(fileSize);
	fread(buffer, fileSize, 1, fp); // read the entire file to buffer
	fclose(fp); // close the file

  memcpy(new_buffer, buffer, fileSize); // copy the boot block and super block
	// populate superblock
	superblock *psb = (superblock *)&(new_buffer[512]);
  psb->blocksize = readIntAt(buffer + 512);
  psb->inode_offset = readIntAt(buffer + 512 + INT_SIZE);
  psb->data_offset = readIntAt(buffer + 512 + (INT_SIZE * 2));
  psb->swap_offset = readIntAt(buffer + 512 + (INT_SIZE * 3));
  psb->free_inode = readIntAt(buffer + 512 + (INT_SIZE * 4));
  psb->free_inode = readIntAt(buffer + 512 + (INT_SIZE * 5));

	int offset = 1024 + (psb->inode_offset * psb->blocksize);
	inode *node = (inode *)&(new_buffer[offset]); // array of inodes
  int final_run = calculateInodeSize(psb);
  int numberOfElem = psb->blocksize / 4;
  int old_index;
  LinkedList list;
  for(int i = 0; i < final_run; i++) {
    bool checkIblock = true;
    bool checkIblockTwo = true;
    //if the inode is valid - relocate and change values
    if(isValid(&node[i])) {
      for(int j = 0; j < 10; j++) {
        if(node[i].dblocks[j] == -1) {
          checkIblock = false;
          break;
        }
        //data region processings
        old_index = node[i].dblocks[j];
        node[i].dblocks[j] = new_index;
        overwrite_block(old_index, numberOfElem, psb, new_buffer, buffer);
        new_index++;
      }

      if(checkIblock) {
        for(int k = 0; k < 4; k++) {
          if(node[i].iblocks[k] == -1) {
            checkIblockTwo = false;
            break;
          }
          old_index = node[i].iblocks[k];
          node[i].iblocks[k] = new_index;
          overwrite_block(old_index, numberOfElem, psb, new_buffer, buffer);
          indirect_block(numberOfElem, psb, new_buffer, buffer);
        }
      }

      if(node[i].i2block == -1) {
        checkIblockTwo = false;
      }
      if(checkIblockTwo) {
        old_index = node[i].i2block;
        node[i].i2block = new_index;
        overwrite_block(old_index, numberOfElem, psb, new_buffer, buffer);
        indrect_block_two(numberOfElem, psb, new_buffer, buffer);
      }


      if(checkIblockTwo && node[i].i3block != -1) {
        old_index = node[i].i3block;
        node[i].i3block = new_index;
        overwrite_block(old_index, numberOfElem, psb, new_buffer, buffer);
        indrect_block_three(numberOfElem, psb, new_buffer, buffer);
      }

    } else {
      insertSorted(&list, node[i]);
    }
  }
  //Set where the next free block exist
  psb->free_block = new_index;
  //Set the free inode blockb
  inode *test = &(list.head)->value;
  for(int i = 0; i < final_run; i++) {
    if(compareInode(test, &node[i]) == true) {
      psb->free_inode = i;
      break;
    }
  }
  int free_region_nodes = psb->swap_offset - psb->data_offset;
  
  for(int i = new_index; i < free_region_nodes; i++) {
    for(int j = 0; j < numberOfElem; j++) {
      writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * i) + (j * sizeof(int)), 0);
    }
  }
  for(int i = new_index; i < free_region_nodes; i++) {
    writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * i) + (0 * sizeof(int)), (i + 1));
  }
  //Last block needs to have -1 as the first element
  writeIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * (free_region_nodes - 1)) + (0 * sizeof(int)), -1);
  
  // for(int i = 0; i < numberOfElem; i++) {
  //   printf("New Buffer[%d]: Number of blocks: %d\t\n", i, readIntAt(new_buffer + 1024 + (psb->data_offset * psb->blocksize) + (psb->blocksize * 20375) + (i * sizeof(int))));
  // }

	fwrite(new_buffer, fileSize, 1, output);
	fclose(output);
	return EXIT_SUCCESS;
}
