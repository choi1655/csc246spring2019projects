#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

pthread_t tid[3];

void * loop_print(void * arg)
{
  int *args = (int *)arg;
  unsigned long i = 0;
  int j = args[0]; //1
  int k = args[1]; //2

  int cnt[15];

  while(j > 0)
  {
    printf("%d ", k);
    for(i = 0; i < 100000000; i++) {}
    k++;
    j--;
  }
  
}

int main()
{
  int err;

  int *args = (int *)malloc(sizeof(int)*2);
  
  /*args[0] = 1;
  args[1] = 10;
  err = pthread_create(&(tid[1]), NULL, loop_print, args);
  if (err != 0)
  {
    printf("\ncan't create thread :[%s]", strerror(err));
    free(args);
  }*/
  
  args[0] = 2;
  args[1] = 1;
  err = pthread_create(&(tid[0]), NULL, &loop_print, args);
  if (err != 0)
  {
    printf("\ncan't create thread :[%s]", strerror(err));
    free(args);
  }
  
  free(args);
  args = (int *)malloc(sizeof(int) * 2);
  
  args[0] = 1;
  args[1] = 10;
  err = pthread_create(&(tid[1]), NULL, &loop_print, args);
  if (err != 0)
  {
    printf("\ncan't create thread :[%s]", strerror(err));
    free(args);
  }
  
  /*for (int i = 0; i < 2; i++) {
    pthread_join(tid[i], NULL);
  }*/

  sleep(5);
  free(args);
  return 0;
}
