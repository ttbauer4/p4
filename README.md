---
title: CS 537 Project 4
layout: default
---

# CS537 Fall 2024, Project 4

## Updates
* TBD

## Administrivia 
- **Due Date** by November 5, 2024 at 11:59 PM
- **Questions**: We will be using Piazza for all questions.

- This project is to be done on the [lab machines](https://csl.cs.wisc.edu/docs/csl/2012-08-16-instructional-facilities/), so you can learn more about programming in C on a typical UNIX-based platform (Linux).
  
- **Handing it in**:
  -  Copy the whole project, including solution and tests folder, to ~cs537-1/handin/login/p4 where login is your CS login.  
  -  Be sure to `make clean` before handing in your solution. 
  -  Only one person from the group needs to submit the probject.
- **Slip Days**: 
  - In case you need extra time on projects, you each will have 2 slip days for the final three projects. After the due date we will make a copy of the handin directory for on time grading. 
  - To use a slip days or turn in your assignment late you will submit your files with an additional file that contains only a single digit number, which is the number of days late your assignment is(e.g 1, 2, 3). Each consecutive day we will make a copy of any directories which contain one of these slipdays.txt files. This file must be present when you submit you final submission, or we won't know to grade your code. 
  - We will track your slip days and late submissions from project to project and begin to deduct percentages after you have used up your slip days.
  - After using up your slip days you can get up to 90% if turned in 1 day late, 80% for 2 days late, and 70% for 3 days late, but for any single assignment we won't accept submissions after the third days without an exception. This means if you use both of your individual slip days on a single assignment you can only submit that assignment one additional day late for a total of 3 days late with a 10% deduction.
  - Any exception will need to be requested from the instructors.

  - Example slipdays.txt
```
1
```
- **Collaboration**: 
  
  - The assignment may be done by **yourself or with one partner**. Copying code from anyone else is considered cheating. [Read this](http://pages.cs.wisc.edu/~remzi/Classes/537/Spring2018/dontcheat.html) for more info on what is OK and what is not. Please help us all have a good semester by not doing this.
  - When submitting each project, you will submit a `partners.txt` file containing the cslogins of both people in the group. One cslogin per line. Do not add commas or any other additional characters.
  - Only one person from the group needs to submit the probject.
  - Partners will receive the same grades for the project.
  - Slip days will be deducted from both members of the group if used. If group members have unequal numbers of slip days, the member with the lower number of days will not be penalized.




## Dynamic Stride Scheduler with Dynamic Ticket Modification
In this project, you will implement a dynamic stride scheduler in xv6, incorporating dynamic ticket modification based on process behavior. The stride scheduler ensures that processes receive CPU time proportional to their assigned tickets, providing deterministic scheduling behavior. By dynamically adjusting tickets, the scheduler can adapt to changing process workloads, priorities, or resource usage.

Learning Objectives:

* Understand and implement a stride scheduling algorithm.
* Gain experience modifying and extending the xv6 operating system.
* Understand how system calls, scheduler and process state are modified.

---

# Project Details

## Overview of Basic Stride Scheduling

The stride scheduler maintains a few additional pieces of information for each process:

- `tickets` -- a value assigned upon process creation.  It can be modified by a system call.  It should default to 8.
- `stride` -- a value that is inversely proportional to a process' tickets. `stride = STRIDE1 / tickets` where `STRIDE1` is a constant (for this project: 1<<10).
- `pass` -- initially set to `0`.  This gets updated every time the process runs.

When the stride scheduler runs, it selects the runnable process with the lowest `pass` value to run for the next tick.
After the process runs, the scheduler increments its `pass` value by its `stride`: `pass += stride`.  These steps ensures that over time,
each process receives CPU time proportional to its tickets.

## Dynamic Process Participation

The Basic Stride Algorithm does not account for changes to the total number of processes waiting to be scheduled.

Consider the case when we have 2 long running processes which have already been running and they all currently have a `stride` value of 1 and `pass` value of 100.
A new process, let us say `A` now joins our system with `stride` value of 1. 

What happens in the case of Basic Stride Scheduling?

Because the `pass` value of `A` is so small compared to the other processes, it will now be scheduled for the next 100 ticks before any other process is allowed to run.
This is not the behavior we want. Given each process has equal tickets, we want the CPU to be shared equally among all of the processes including the newly arrived process.
In this particular case we would want all processes to take turns.

#### How do we do that?

Let us maintain aggregate information about the set of processes waiting to be scheduled and use that information when a process enters or leaves.

- `global_tickets` -- the sum of all **runnable** process's tickets.
- `global_stride` -- inversely proportional to the `global_tickets`, specifically `STRIDE1 / global_tickets`
- `global_pass` -- incremented by the **current** `global_stride` at every tick.

Now, when a process is created, its `pass` value will begin at `global_pass`.  In the case of process `A` above, the `global_stride` and number of ticks that have occurred
will make `A`'s starting `pass` value be the same as the other 2 processes.

The global variables will need to be recalculated whenever a process enters or leaves to create the intended behaviour.

The final piece of information the scheduler will need to keep track for each process is the `remain` value which will store the remaining portion
of its stride when a dynamic change occurs. The `remain` field represents the number of passes that are left before a process' next selection.
When a process leaves the scheduler queue, `remain` is computed as the difference between the process' `pass` and the `global_pass`. 
Then when a process rejoins the system, its `pass` value is recomputed by adding its `remain` value to the `global_pass`.

This mechanism handles situations involving either positive or negative error between the specified and actual number of allocations. 

- If remain < stride, then the process is effectively given credit when it rejoins for having previously waited for part of its stride without receiving a timeslice tick to run. 
- If remain > stride, then the process is effectively penalized when it rejoins for having previously received a timeslice tick without waiting for its entire stride.

Let us consider an example, process `A` currently has a pass of 1000, where the `global_pass` is 600. Now process `A` decides to sleep on keyboard inturrupt. After a few ticks, when the interrupt occurs, the `global_pass` has updated to a 1400. We only want to increment `A`'s pass for the time it was asleep, so we cannot just add 1400 to 1000. Instead we measure `remain` at the time process left the scheduler queue, in this case 1000-600=400, and when process `A` rejoins we will calculate the new pass as `remain+global_pass` that is 400+1400= 1800.

## Dynamic Ticket Modification

We also want to support dynamically changing a process’ ticket allocation. 

When a process’ allocation is dynamically changed from `tickets`to `tickets'`, its stride and pass values must be recomputed. The new `stride'` is computed as usual, inversely
proportional to tickets.

To compute the new `pass'`, the remaining portion of the client’s current stride, denoted by `remain`, is adjusted to reflect the new `stride'`. This is accomplished 

by scaling `remain` by `stride'/stride`


## Implementation
We will be using a modular scheduler for our implementation. Take a look at the Makefile and the `SCHED_MACRO` variable, your implementation should support both RR and Stride scheduler based on the flag passed.

We will only be implementing the dynamic stride scheduler for a single CPU.
Also for this project, consider the timeslice equal to a tick in xv6. We will measure all our time in tick granuality.

**Task 1**

Add appropriate fields in proc struct and get the number of ticks a process has been running for. Populate and maintain these values. 

(Think about where is remain modified? How do you maintain the global values?)

**Task 2**

Pick the process with the lowest pass value among all processes waiting to be scheduled.

(What is the process state here?).

For tie breaking, use the `total runtime`. That is the process which has spent lower number of ticks running will be scheduled first. 
If both the comparisions are equal fall back on pid. The process with smaller pid goes first. 

The process' pass is updated everytime it is scheduled.

**Task 3**

Create a new system call to allow a process to set its own number of tickets. 

```
int settickets(int n);
```

The max tickets allowed to be set for a process is 1<<5.

The minimum tickets allowed is 1. 

If a process sets a value lower than 1, set the number of tickets to default = 8.
**This is also the number of tickets a new process in your system should have.**

**Task 4**

Implement a system call 
```
int getpinfo(struct pstat*);
```
To retrieve scheduling information for all processes.

```
struct pstat {
  int inuse[NPROC];      // Whether this slot of the process table is in use (1 or 0)
  int tickets[NPROC];    // Number of tickets for each process
  int pid[NPROC];        // PID of each process
  int pass[NPROC];       // Pass value of each process
  int remain[NPROC];     // Remain value of each process
  int stride[NPROC];     // Stride value for each process
  int rtime[NPROC];      // Total running time of each process
};
```

Note that you need to add this struct definition to a header file named `pstat.h`. The tests will include this header to work with the struct.

**Task 5**

Yayy! You now have a running Stride scheduler.

To verify the difference in behavior we described above let us test both  the RR and Stride scheduler on a CPU intensive workload, and use getpinfo to retrieve the scheduling information for them.

There is a `workload.c` file in the initial xv6 implementation, ensure you have _workload target in your `UPROGS` in the Makefile.

run the RR scheduler first (without modifying the scheduler flag)
```
make qemu-nox 
```

Now in xv6 run the workload with the following command:
```
workload &
```

Notice the `&` here, the workload runs for a long time so make sure to run it in the background or you may need to wait a substantial time before you can see your results.

You should now see periodic snapshots of the pstat of all the processes in the system.

Once you see "Done Measuring" printed to the output, 
run 
```
cat rr_process_stats.csv
```
Copy this file to your lab machine.

Now repeat the process for your xv6 with compiled with
```
make qemu-nox SCHEDULER=STRIDE  
```

The final results this time will be visible by: 
```
cat stride_process_stats.csv
```

Add both stride_process_stats.csv and rr_process_stats.csv inside your P4 directory.

**Task 6**

Analyze the results of the workloads in your csvs to compare how processes with different tickets are scheduled in both cases. What is the advantage of stride scheduling? What is the behavior/pattern of process runtimes observed because of dynamic process participation?

Add this brief explaination to your README.md.

Here is what
```
ls p4
```
should look like at time of submission:
```
README.md
solution/
tests/
rr_process_stats.csv
stride_process_stats.csv
partners.txt
slipdays.txt # optional
```

### Further Reading
- ["Stride Scheduling: Deterministic Proportional-Share Resource Management" by Carl A. Waldspurger and William E. Weihl. Technical Memo MIT/LCS/TM-528, MIT Laboratory for Computer Science, June 1995.](https://dl.acm.org/action/downloadSupplement?doi=10.5555%2F889650&file=mit___artificial_intelligence_laboratory_tm-528.ps)
- [OSTEP Chapter 9: Scheduling: Proportional Share.](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf)
Discusses concepts related to proportional-share scheduling algorithms.
