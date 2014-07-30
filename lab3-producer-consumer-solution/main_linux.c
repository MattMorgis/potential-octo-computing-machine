#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

struct element
{
  int value;//value of element
  pthread_mutex_t lock;//lock to ensure element is not being produced/consumed twice at same time
};

//global representation of data
struct element* buffer; //buffer of elements
int max_buffer_size;//size of the buffer
int count; //number of elements in buffer
int fd; //file descriptor for log file
pthread_mutex_t fileLock; //mutex lock for file writing
int in; //next slot for producer
int out; //next slot for consumer
pthread_mutex_t proLock; //lock for producers
int val=0;//value that is produced

//Producer Thread function
void * producerFunction(void* args)
{
  int i;

  //beginning to all print statements from producer
  char name[100];
  sprintf(name,"[Producer %u]",(unsigned int)pthread_self());

  //infinite loop for producing values
  while (1)
  {
    pthread_mutex_lock(&proLock);
    while (count==max_buffer_size)
    {
      char message[100];
      int size =sprintf(message,"%s Buffer is full waiting 1 second\n",name);
      printf("%s Buffer is full waiting 1 second\n",name);
      pthread_mutex_lock(&fileLock);
      
      int rc = write(fd,message,size);
      if (rc<0)
      {
	fprintf(stderr,"[ERROR] Could not write to file\n");
      }
      pthread_mutex_unlock(&fileLock);
      sleep(1);
    }

    int tmp=val; //temporary storage of the value

    //produce a value into the next slot 
    pthread_mutex_lock(&buffer[in].lock);
    buffer[in].value=val;
    val++;
    count++;
    in++;
    in%=max_buffer_size;


    //print that value was produced
    char message[100];
    int size = sprintf(message,"%s Produced value %d\n",name,tmp);
    printf("%s Produced value %d\n",name,tmp);
    pthread_mutex_lock(&fileLock);
    int rc = write(fd,message,size);
    if (rc<0)
    {
      fprintf(stderr,"[ERROR] Could not write to file\n");
    }
    pthread_mutex_unlock(&fileLock);
    pthread_mutex_unlock(&buffer[(in-1+max_buffer_size)%max_buffer_size].lock);
    pthread_mutex_unlock(&proLock); 
    
  }
  return NULL;
}


//Consumer Thread function
void * consumerFunction(void* args)
{
  int i;

  //beginning to all print statements from producer
  char name[100];
  sprintf(name,"[Consumer %u]",(unsigned int)pthread_self());

  //infinite loop for producing values
  while (1)
  {
    pthread_mutex_lock(&proLock);
    while (count==0)
    {
      char message[100];
      int size =sprintf(message,"%s Buffer is empty waiting 1 second\n",name);
      printf("%s Buffer is empty waiting 1 second\n", name);
      pthread_mutex_lock(&fileLock);
      
      int rc = write(fd,message,size);
      if (rc<0)
      {
	fprintf(stderr,"[ERROR] Could not write to file\n");
      }
      pthread_mutex_unlock(&fileLock);
      sleep(1);
    }

    //produce a value into the next slot
    pthread_mutex_lock(&buffer[out].lock);
    int recv =buffer[out].value;

    //update global variables and then unlock 
    count--;
    out++;
    out%=max_buffer_size;


    //print that value was produced
    char message[100];
    int size = sprintf(message,"%s Consumed value %d\n",name,recv);
    printf("%s Consumed value %d\n",name,recv);
    pthread_mutex_lock(&fileLock);
    int rc = write(fd,message,size);
    if (rc<0)
    {
      fprintf(stderr,"[ERROR] Could not write to file\n");
    }
    pthread_mutex_unlock(&fileLock);
    pthread_mutex_unlock(&buffer[(out-1+max_buffer_size)%max_buffer_size].lock);
    pthread_mutex_unlock(&proLock);
    
  }
  return NULL;
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
  fd = open("log.txt",O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  max_buffer_size=buffer_size;
  count=0;
  in=0;
  out=0;
  
  //thread arrays for producer and consumer
  pthread_t* pro=(pthread_t*)(calloc(numProducers,sizeof(pthread_t)));
  pthread_t* con=(pthread_t*)(calloc(numConsumers,sizeof(pthread_t)));

  //create producer threads
  for (i=0;i<numProducers;i++)
  {
    int rc = pthread_create(&pro[i],NULL,producerFunction,NULL);
    if (rc!=0)
    {
      fprintf(stderr,"[ERROR] could not make producer thread %d",i+1);
    }
  }
  
  //create consumer threads
  for (i=0;i<numConsumers;i++)
  {
    int rc = pthread_create(&con[i],NULL,consumerFunction,NULL);
    if (rc!=0)
    {
      fprintf(stderr,"[ERROR] could not make consumer thread %d",i+1);
    }
  }

  printf("log of information printed to log.txt\n");

  sleep(15);


  printf("Exit the program\n");
  exit(0);


}

