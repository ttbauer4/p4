# CS537 Fall 2024, Project 3

## Names: Aditya Kamasani & Trenton Bauer
## CS Logins: kamasani & trenton
## Student IDs: 9083172750 & 9083302258
## Wisc Email: kamasani@wisc.edu & ttbauer@wisc.edu

***Analyze the results of the workloads in your csvs to compare how processes with different tickets are scheduled in both cases. What is the advantage of stride scheduling? What is the behavior/pattern of process runtimes observed because of dynamic process participation?***  
  
In the Round Robin scheduler implementation, processes get scheduled irrespective of their ticket value (process priority). In our Stride scheduler, processes are scheduled proportional to their ticket value, giving more CPU time slices to higher-priority processes. The advantage of this is that we can ensure proportional fairness, prioritizing CPU use for more demanding processes. Because of dynamic process participation, processes joining the scheduler queue later are not given disproportionate access to the CPU; rather, the CPU is shared proportionally (with respect to process ticket values) among all processes, including the newly-arrived process.  