==============
HPC scheduling simulator
Tyler A. Simon
version 5, SWF

Runs through all alpha, beta and gamma combinations for SWF formatted files.
Looks at performance of job classes using arrival times instead of bulk traffic like version 4.

============

To compile: make

Change DEBUG to 1 or IO to 1 to get more output.
If you set Debug to 1, you will get a tick by tick printout, it will take a very long time to get a complete run.

IO=1 will print a utilization.txt which can be plotted using:
gnuplot -persist plotutil.gp

To run:

./sched_sim_05 <input file> <system size>

example:
./sched_sim_05 data/test.inp 65536 | less

Class_Averages(LL,LS,SL,SS)(-1.0 -1.0 -1.0)     1.1     1.3      1      1.1     201     0.25
Class_Averages(LL,LS,SL,SS)(-1.0 -1.0 -0.9)     1.1     1.3      1      1.1     201     0.25
Class_Averages(LL,LS,SL,SS)(-1.0 -1.0 -0.8)     1.1     1.3      1      1.1     201     0.25


input file format:
<start time> <waittime> <runtime> <cores> <priority>
0 1 10 128 1
0 1 20 256 1
0 1 20 256 1
0 1 200 32 1
1 1 5 64 1
5 1 5 1
5 1 10 32 1
6 1 13 256 1


output format:
Each job class' expansion factor is listed, then total ticks, then utilizaton for each a,b,g combo.

for ./sched_sim_05 data/test.inp 65536

Class_Averages(LL,LS,SL,SS)(0.9 0.8 0.8)        1.1     1.3      1      1.1     201     0.25
Class_Averages(LL,LS,SL,SS)(0.9 0.8 0.9)        1.1     1.3      1      1.1     201     0.25
========
SWF data
========

Is in the data directory and can be obtained from 
http://www.cs.huji.ac.il/labs/parallel/workload/logs.html


process it first with data/procswf.sh
