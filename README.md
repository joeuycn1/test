log1.c descript:
main creates 2 threads of producers write to a named pipe.
and then 1 thread of consumer read from the pipe.
use semaphore.h to lock and unlock.
Compiler PASS without any Error or Waring.
   ---
log1.c Problem:
1. Consumer can read all the info from pipe, but fail to exit.
   The problem must be in line 134 to 143.
>> Sep.30, 2014 <

Solution for problem 1 of log1.c:
Set the end signal for consumer to quit the circle and finish reading the pipe.
>> Sep.30, 2014 <
###########################################

 
