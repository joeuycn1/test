/* pthread test 8: Producer-Consumer */
/* with named pipe, pthread, semaphore */
/* set "end" to finish reading pipe  */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>
#include<semaphore.h>
#include<sys/ipc.h>

#define MYFIFO   "myfifo" // for named pipe
#define BUFFER_SIZE 50    // No of Unit for each buff
#define UNIT_SIZE   30     // Bytes for each Unit
#define RUN_TIME    10    // time to run
#define DTL   5.0 	  // Delay Time Level

const char *log_path="lf1"; // path of the log file
const char *mode="w";  // for log file open mode
int fd;  // for pipe
time_t end_time;   // time for thread
sem_t mutex, full, avail;  // for semaphore
const char *end="   --- the end ---"; // to end the pipe read thread

void *producer1(void *arg)
{
   int real_write;  // the bytes really write
   int delay_time=0;
   unsigned char log[UNIT_SIZE]; // for log info TBA
   int i=1;
   char ch[2];

   while(time(NULL)<end_time)
   {
	delay_time=(int)(rand()*DTL/(RAND_MAX)/4.0)+1;
	sleep(delay_time);
	sem_wait(&avail);
	sem_wait(&mutex);
//	printf("\nProducer1: delay=%d\n",delay_time);
// Producer write data
	strncpy(log,"Log1 infomation! No.",UNIT_SIZE);
	sprintf(ch,"%d",i++);
	strncat(log,ch,2);
	if((real_write=write(fd,log,UNIT_SIZE))==-1)
	{
	   if(errno==EAGAIN)
		printf("The FIFO has not been read yet. Pls try later.\n");
	}
	else
	   printf("\nProducer1 write %d to the FIFO, size=%d.\n",i-1,real_write);
	memset(log,0,UNIT_SIZE);   
	sem_post(&full);
	sem_post(&mutex);
   }
printf("\n !!! When Pro1 end, produce itmes: %d !!!\n",i-1);
   pthread_exit(NULL);
}

void *producer2(void *arg)
{
   int real_write;  // the bytes really write
   int delay_time=0;
   unsigned char log[UNIT_SIZE];  // for log linfo TBA
   int i=1;
   char ch[2];

   while(time(NULL)<end_time)
   {
	delay_time=(int)(rand()*DTL/(RAND_MAX)/2.0)+1;
	sleep(delay_time);
	sem_wait(&avail);
	sem_wait(&mutex);
//	printf("\nProducer2: delay=%d\n",delay_time);
// Producer write data
	strncpy(log,"LOG2 INFO! No.",UNIT_SIZE);
	sprintf(ch,"%d",i++);
	strncat(log,ch,2);
	if((real_write=write(fd,log,UNIT_SIZE))==-1)
	{
	   if(errno==EAGAIN)
		printf("The FIFO has not been read yet. Pls try later.\n");
	}
	else
	   printf("\nProducer2 write %d to the FIFO, size=%d.\n",i-1,real_write);
	memset(log,0,UNIT_SIZE);
	sem_post(&full);
	sem_post(&mutex);
   }

sleep(2);
printf("\n !!! When Pro2 end, produce itmes: %d !!!\n",i-1);
// set info to finish consumer.
strncpy(log,end,UNIT_SIZE);
if((real_write=write(fd,log,UNIT_SIZE))==-1)
{  if(errno==EAGAIN)  printf("FIFO not ready!\n"); }
else printf("\n...write the end info...\n");

   pthread_exit(NULL);
}

void *consumer(void *arg)
{
   unsigned char read_buffer[UNIT_SIZE];
   int delay_time;
   int read_size;
   FILE *fp=NULL;
   if((fp=fopen(log_path, mode))==NULL)
   { printf("Log File Open Err!\n"); exit(1);}

   while(time(NULL)<end_time)
   {
	delay_time=(int)(rand()*DTL/(RAND_MAX))+1;
	sleep(delay_time);

	sem_wait(&full);
	sem_wait(&mutex);

	memset(read_buffer,0,UNIT_SIZE);
	printf("\nDuring Producing, Consumer: delay=%d\n",delay_time);

	if((read_size=read(fd,read_buffer,UNIT_SIZE))==-1)
	{
	   if(errno==EAGAIN)
		printf("No data in buffer.\n");
	}
	printf("Read %s from FIFO!\n",read_buffer);
	fprintf(fp,"%s\n",read_buffer);
	fflush(fp);
//	printf("This info has been written to file successfully.\n");
	
	sem_post(&avail);
	sem_post(&mutex);
   }

// after the producers are finished, read all the info from pipe
fprintf(fp,"\n   *** TIME UP *** \n\n");
fflush(fp);
sleep(3);

   while((read_size=read(fd,read_buffer,UNIT_SIZE))==UNIT_SIZE)
   {
	printf("\n>>> After Time up, read (%d) bytes from FIFO.\n",read_size);
	printf("    Info Read from FIFO >>> %s\n",read_buffer);
	fprintf(fp,"%s\n",read_buffer);
	fflush(fp);
//	printf("This info has been written to file successfully.\n");
// for break
	if(strcmp(read_buffer,end)==0) break;
	memset(read_buffer,0,UNIT_SIZE); read_size=0;
   }

   fclose(fp);
   pthread_exit(NULL);
}


int main()
{
   pthread_t thrd_pro1_id, thrd_pro2_id, thrd_cus_id;
   pthread_t mon_th_id;
   int ret;

   srand(time(NULL));
   end_time=time(NULL)+RUN_TIME;

// create named pipe
   if((mkfifo(MYFIFO,O_CREAT|O_EXCL)<0)&&(errno!=EEXIST))
   {
	printf("Create FIFO fail!\n");
	return errno;
   }
// open pipe
   fd=open(MYFIFO,O_RDWR,0666);
   if(fd==-1)
	{ printf("Open FIFO fail!\n"); return fd; }

// init semaphore mutex ==1
   ret=sem_init(&mutex,0,1);
// init semaphore avail
   ret+=sem_init(&avail,0,BUFFER_SIZE);
// init semaphore full
   ret+=sem_init(&full,0,0);

   if(ret!=0)
   { printf("Semaphore init fail!\n"); return ret; }

// create threads for 2 producers and 1 consumer
   ret=pthread_create(&thrd_pro1_id,NULL,producer1,NULL);
   if(ret!=0)
   { printf("Create Producer thread fail!\n"); return ret; }

   ret=pthread_create(&thrd_pro2_id,NULL,producer2,NULL);
   if(ret!=0)
   { printf("Create Producer thread fail!\n"); return ret; }

   ret=pthread_create(&thrd_cus_id,NULL,consumer,NULL);
   if(ret!=0)
   { printf("Create Consumer thread fail!\n"); return ret; }

   pthread_join(thrd_pro1_id,NULL);
printf("\n > Producer1 finish! <\n");
   pthread_join(thrd_pro2_id,NULL);
printf("\n > Producer2 finish! <\n");
   pthread_join(thrd_cus_id,NULL);
printf("\n > Consumer finish! <\n");
//   wait();
   close(fd);

printf("\n\n  --- THE END ---\n\n ");
 
unlink(MYFIFO);  // Del the pipe file when all threads end

   return 0;
}

