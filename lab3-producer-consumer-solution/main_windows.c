#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <windows.h>

struct element
{
  int value;//value of element
  HANDLE lock;//lock to ensure element is not being produced/consumed twice at same time
};

//global representation of data
struct element* buffer; //buffer of elements
int max_buffer_size;//size of the buffer
int count; //number of elements in buffer
int fd; //file descriptor for log file
HANDLE fileLock; //mutex lock for file writing
int in; //next slot for producer
int out; //next slot for consumer
HANDLE proLock; //lock 
int val=0;//value that is produced

int sleepTime=100; //Wait time if buffer is full/empty when produced/consumed

//Producer Thread function
DWORD WINAPI producerFunction(void* args)
{
  int i;

  //beginning to all print statements from producer
  char name[100];
  sprintf(name,"[Producer %d]",GetCurrentThreadId());

  //infinite loop for producing values
  while (1)
  {
    WaitForSingleObject(proLock,INFINITE);
    while (count==max_buffer_size)
    {
      char message[100];
      int size =sprintf(message,"%s Buffer is full waiting for a bit\n",name);
      WaitForSingleObject(fileLock,INFINITE);
      
      int rc = write(fd,message,size);
      if (rc<0)
      {
    	  fprintf(stderr,"[ERROR] Could not write to file\n");
      }
      ReleaseMutex(fileLock);
      Sleep(sleepTime);
    }

    int tmp=val; //temporary storage of the value

    //produce a value into the next slot 
    WaitForSingleObject(buffer[in].lock,INFINITE);
    buffer[in].value=val;
    val++;
    count++;
    in++;
    in%=max_buffer_size;


    //print the value that was produced
    char message[100];
    int size = sprintf(message,"%s Produced value %d\n",name,tmp);
    WaitForSingleObject(&fileLock,INFINITE);
    int rc = write(fd,message,size);
    if (rc<0)
    {
      fprintf(stderr,"[ERROR] Could not write to file\n");
    }
    ReleaseMutex(&fileLock);
    ReleaseMutex(buffer[(in-1+max_buffer_size)%max_buffer_size].lock);
    ReleaseMutex(proLock);
    
  }
  return 0;
}


//Consumer Thread function
DWORD WINAPI consumerFunction(void* args)
{
  int i;

  //beginning to all print statements from consumer
  char name[100];
  sprintf(name,"[Consumer %d]",GetCurrentThreadId());

  //infinite loop for consuming values
  while (1)
  {
    WaitForSingleObject(proLock,INFINITE);
    while (count==0)
    {
      char message[100];
      int size =sprintf(message,"%s Buffer is empty waiting for a bit\n",name);
      WaitForSingleObject(fileLock,INFINITE);
      
      int rc = write(fd,message,size);
      if (rc<0)
      {
    	  fprintf(stderr,"[ERROR] Could not write to file\n");
      }
      ReleaseMutex(fileLock);
      Sleep(sleepTime);
    }

    //consume the value in the next slot
    WaitForSingleObject(&buffer[out].lock,INFINITE);
    int recv =buffer[out].value;

    //update global variables and then unlock 
    count--;
    out++;
    out%=max_buffer_size;


    //print the value consumed
    char message[100];
    int size = sprintf(message,"%s Consumed value %d\n",name,recv);
    WaitForSingleObject(fileLock,INFINITE);
    int rc = write(fd,message,size);
    if (rc<0)
    {
      fprintf(stderr,"[ERROR] Could not write to file\n");
    }
    ReleaseMutex(fileLock);
    ReleaseMutex(buffer[(out-1+max_buffer_size)%max_buffer_size].lock);
    ReleaseMutex(proLock);
    
  }
  return 0;
}


//Main startup function
//argv[1]=number of producers
//argv[2]=number of consumers
//argv[3]=buffer size
int main(int argc, char* argv[])
{
  //loop variable 
  int i;

  //number of consumers and number of producers respectively
  int numConsumers,numProducers;
  int buffer_size;

  //parsing of command line arguments
  if (argc==1)
  {
    //if no extra arguments imply minimums
    numConsumers=3;
    numProducers=4;
    buffer_size=20;
  }
  else if (argc==4)
  {
    numConsumers=atoi(argv[2]);
    numProducers=atoi(argv[1]);
    buffer_size=atoi(argv[3]);
  }
  else
  {
    //incorrect inputs results in error and instructions on correct execution
    fprintf(stderr,"[ERROR] Incorrect command line arguments\n");
    printf("    ./exe\n");
    printf("    ./exe <number of producers> <number of consumers> <buffer size>\n");
    return EXIT_FAILURE;
  }
  
  //create buffer and file
  buffer = (struct element*)calloc(buffer_size,sizeof(struct element));
  for (i=0;i<buffer_size;i++)
	  buffer[i].lock=CreateMutex(NULL,FALSE,NULL);
  fd = open("log.txt",O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

  //initialize global constants
  max_buffer_size=buffer_size;
  count=0;
  in=0;
  out=0;
  fileLock = CreateMutex(NULL,FALSE,NULL);
  proLock = CreateMutex(NULL,FALSE,NULL);
  proLock = CreateMutex(NULL,FALSE,NULL);
  

  //producer and consumer thread arrays
  HANDLE* pro=(HANDLE*)(calloc(numProducers,sizeof(HANDLE)));
  HANDLE* con=(HANDLE*)(calloc(numConsumers,sizeof(HANDLE)));

  //create the producer threads
  for (i=0;i<numProducers;i++)
  {
    pro[i] = CreateThread(NULL, 0, producerFunction, NULL, 0, NULL);
    if (!pro[i])
    {
      fprintf(stderr,"[ERROR] could not make producer thread %d",i+1);
    }
  }

  //Create the consumer threads
  for (i=0;i<numConsumers;i++)
  {
    con[i] = CreateThread(NULL, 0, consumerFunction, NULL, 0, NULL);
    if (!con[i])
    {
      fprintf(stderr,"[ERROR] could not make consumer thread %d",i+1);
    }
  }
  printf("log of information printed to log.txt\n");

  //block until all threads are terminated
  for (i=0;i<numProducers;i++)
	  WaitForSingleObject(pro[i],INFINITE);
  for (i=0;i<numConsumers;i++)
	  WaitForSingleObject(con[i],INFINITE);
  return EXIT_SUCCESS;
}

