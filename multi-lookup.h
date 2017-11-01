#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#define ARRAYSIZE 15

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
#include<stdbool.h>

struct requesterStruct {

  pthread_mutex_t* sharedArrayLock;
  pthread_mutex_t inputFilesLock;
  pthread_mutex_t servicedFileLock;
  int* sharedArrayCounter;
  int fileCounter;
  char** sharedBufferPtr;
  char* inputFiles[10];
  int numberOfFiles;
  int inputFileCounter;
  FILE* servicedFile;
  int threadCounter;
  int requesterCounter;
  bool* flag;

};

struct resolverStruct {
  pthread_mutex_t* sharedArrayLock;
  pthread_mutex_t outputFileLock;
  int* sharedArrayCounter;
  char** sharedBufferPtr;
  FILE* outputFile;
  bool* flag;

};




void* writeBuffer(struct requesterStruct* requesterData);
void* readBuffer(struct resolverStruct* resolverData);
void* functionC(char* prt);

#endif
