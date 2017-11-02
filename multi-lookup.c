#include "multi-lookup.h"
#include "util.h"
#define ARRAYSIZE 15


int main(int argc, char* argv[])
{

  //local declarations
  char* tmp;
  char buffer[100];
  char* sharedArray[ARRAYSIZE];
  int counter = -1;
  int i = 0;
  int j = 0;
  int requester = strtol(argv[1], &tmp, 10);
  int resolver = strtol(argv[2], &tmp, 10);
  if(requester > 5)
    requester = 5;
  if(resolver > 10)
    resolver = 10;


  //pthread declarations
  pthread_mutex_t lock;
  pthread_t requestPtr[requester];
  pthread_t resolverPtr[resolver];


  //declare structs and struct pointers
  struct requesterStruct requesterData;
  struct requesterStruct* structPtr1;
  struct resolverStruct resolverData;
  struct resolverStruct* structPtr2;
  structPtr1 = &requesterData;
  structPtr2 = &resolverData;


  //initialize shared struct data
  requesterData.sharedArrayCounter = &counter;
  resolverData.sharedArrayCounter = &counter;
  requesterData.sharedArrayLock = &lock;
  resolverData.sharedArrayLock = &lock;
  requesterData.sharedBufferPtr = sharedArray;
  resolverData.sharedBufferPtr = sharedArray;


  //initialize requester data
  requesterData.fileCounter = 0;
  requesterData.numberOfFiles = argc - 5;
  requesterData.inputFileCounter = 0;


  //initialize flags and threadCounter
  bool flag = true;
  requesterData.threadCounter = 0;
  requesterData.requesterCounter = requester;
  requesterData.flag = &flag;
  resolverData.flag = &flag;


  //initialize mutexes
  pthread_mutex_init(requesterData.sharedArrayLock, NULL);
  pthread_mutex_init(&requesterData.inputFilesLock, NULL);
  pthread_mutex_init(&requesterData.servicedFileLock, NULL);


  //starting timer
  struct timeval start, end;
  gettimeofday(&start, NULL);


  //check if user input is correct
  if(argc < 6)
  {
    printf("Please provide the correct arguments in the following format.\n");
    printf("requesterThreads resolverThreads results.txt serviced.txt inputFiles\n");
    return 0;
  }


  //initialize input files array
  for(int x = 0; x < argc - 5; x++)
    requesterData.inputFiles[x] = argv[5 + x];


  //open output files
  requesterData.servicedFile = fopen(argv[4], "a");
  if(requesterData.servicedFile == NULL)
  {
    printf("Bad output file\n");
    return -1;
  }
  resolverData.outputFile = fopen(argv[3], "a");
  if(resolverData.outputFile == NULL)
  {
    printf("Bad output file\n");
    return -1;
  }


  //create threads
  for(i = 0; i < requester; i++)
  {
    pthread_create(&requestPtr[i], NULL, writeBuffer, structPtr1);
  }

  for(j = 0; j < resolver; j++)
  {
    pthread_create(&resolverPtr[j], NULL, readBuffer, structPtr2);
  }

  for(i = 0; i < requester; i++)
  {
    pthread_join(requestPtr[i], NULL);
  }

  for(j = 0; j < resolver; j++)
  {
    pthread_join(resolverPtr[j], NULL);
  }


  //close output files
  fclose(requesterData.servicedFile);
  fclose(resolverData.outputFile);


  //stop timer
  gettimeofday(&end, NULL);
  printf("Runtime: %ld\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));


  return 0;
}

void* writeBuffer(struct requesterStruct* requesterData)
{

  int numFiles = requesterData->numberOfFiles;
  int numServiced = 0;
  char name[1025];
  uint64_t tid;
  pthread_threadid_np(NULL, &tid);
  FILE* currentFile;

  while(requesterData->fileCounter < numFiles)
  {

    pthread_mutex_lock(&requesterData->inputFilesLock);
    if((requesterData->fileCounter + 1) > numFiles)
      break;
    currentFile = fopen(requesterData->inputFiles[requesterData->fileCounter], "r+");
    if(currentFile == NULL)
      fprintf(stderr, "Can't open file\n");
    requesterData->fileCounter += 1;
    pthread_mutex_unlock(&requesterData->inputFilesLock);

    while(!feof(currentFile))
    {
      pthread_mutex_lock(requesterData->sharedArrayLock);
      if(*requesterData->sharedArrayCounter < 15)
      {
        fscanf(currentFile, "%s", name);
        *requesterData->sharedArrayCounter += 1;
        requesterData->sharedBufferPtr[*requesterData->sharedArrayCounter] = (char *)malloc(1025);
        strcpy(requesterData->sharedBufferPtr[*requesterData->sharedArrayCounter], name);
        pthread_mutex_unlock(requesterData->sharedArrayLock);
      }
      else
      {
        pthread_mutex_unlock(requesterData->sharedArrayLock);
        usleep(100000);
      }
    }
    fclose(currentFile);
    numServiced++;
  }


  pthread_mutex_lock(&requesterData->servicedFileLock);
  fprintf(requesterData->servicedFile, "Thread %lld serviced %d file(s)\n", tid, numServiced);
  if((requesterData->threadCounter + 1) != requesterData->requesterCounter)
  {
    requesterData->threadCounter += 1;
  }
  else
  {
    *requesterData->flag = false;
  }
  pthread_mutex_unlock(&requesterData->servicedFileLock);
  pthread_exit(0);
}


void* readBuffer(struct resolverStruct* resolverData)
{

  char name[1025];
  char ip[100];

  while(*resolverData->flag && resolverData->sharedArrayCounter >= 0)
  {
    pthread_mutex_lock(resolverData->sharedArrayLock);
    if(*resolverData->sharedArrayCounter >= 0)
    {
      printf("name: %s\n", resolverData->sharedBufferPtr[*resolverData->sharedArrayCounter]);
      if(dnslookup(resolverData->sharedBufferPtr[*resolverData->sharedArrayCounter], ip, 100) == 0)
        fprintf(resolverData->outputFile, "%s,%s\n", resolverData->sharedBufferPtr[*resolverData->sharedArrayCounter], ip);
      else
        fprintf(resolverData->outputFile, "%s,\n", resolverData->sharedBufferPtr[*resolverData->sharedArrayCounter]);
      printf("ip address: %s\n", ip);
      free(resolverData->sharedBufferPtr[*resolverData->sharedArrayCounter]);
      *resolverData->sharedArrayCounter -= 1;
      pthread_mutex_unlock(resolverData->sharedArrayLock);
    }
    else
    {
      pthread_mutex_unlock(resolverData->sharedArrayLock);
      usleep(100000);
    }
  }
  pthread_exit(0);
}
