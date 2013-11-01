==========

knapsack scheduler
Tyler Simon

==========


knapsack:
single cycle, selects maximum value using priority and has an associated failure rate

to compile:

cc -o knapsack knapsack.c -lm

inputfile : input.dat

example: ./knapsack <capacity> <number of elements> <failure rate>

./knapsack 100 5 2.3

Output:
./knapsack 100 5 2.3
id 1, priority * weight = 2.400000
id 2, priority * weight = 0.200000
id 3, priority * weight = 0.050000
id 4, priority * weight = 0.800000
id 5, priority * weight = 0.100000
selected all of 1
selected all of 4
selected all of 2
selected all of 5
selected all of 3
### Ordered By Ratio ###
id = 1 weight = 8.000000 priority = 0.300000 ratio = 2.4
id = 4 weight = 4.000000 priority = 0.200000 ratio = 0.8
id = 2 weight = 1.000000 priority = 0.200000 ratio = 0.2
id = 5 weight = 1.000000 priority = 0.100000 ratio = 0.1
id = 3 weight = 1.000000 priority = 0.050000 ratio = 0.05
poiss(2,3) = 18.0447%

###FAILURE Stats failure rate = 2 ###
#item(i) arrival_time prob_1_failure prob_i_failures
 1, 38365, 0.270671, 0.270671
 2, 37003, 0.270671, 0.270671
 3, 38238, 0.270671, 0.180447
 4, 38273, 0.270671, 0.0902235
 5, 38512, 0.270671, 0.036089

=================
knapsack2: is a fractional  (or 0/1) knapsack scheduler used to simulate Map/Reduce workloads
=================

to compile: cc -o knapsack2 knapsack2.c 

to Run: ./knapsack2 1 1 1 1000 5


output is in .arff format to be viewed with the WEKA machine learning toolkit
http://www.cs.waikato.ac.nz/ml/weka/
