log1.c

2 threads of producers write to a named pipe.
1 thread of consumer read from the pipe.
use semaphore.h to lock and unlock.

Compiler PASS without any Error or Waring.

Problem:
1. Consumer can read all the info from pipe, but fail to exit.
   The problem must be in line 134 to 143.

--- Sep.30, 2014---
