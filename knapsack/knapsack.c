/* Continuous Knapsack with job priorities and failures
Tyler Simon
2/7/2012

format of input.dat
[jobid] [weight] [priority]
1 10 50
2 15 80
3 20 20
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


double poisson(float, int);
double factorial(int);
double expdist(float);

/* set to 0 or 1 if solving a continuous vs noncontinuous knapsack
continuous means we can take a fraction of an item, otherwise we have 0/1 knapsack */
#define CONTINUOUS 0

typedef struct{
	int id;
	float weight;
	float prio;
	double ratio;
	}nums;


  /* A Quicksort for structures of type address. */
  void quick_struct(nums items[], int count)
  {
    qs_struct(items,0,count-1);
  }

  int qs_struct(nums items[], int left, int right)
  {

    register int i, j;
    double x;
    nums temp;

    i = left; j = right;
    x = items[(left+right)/2].ratio;

    do {
      while((isgreater(items[i].ratio,x)) && (i < right)) i++;
      while((isless(items[j].ratio,x)) && (j > left)) j--;
      if(i <= j) {
        temp = items[i];
        items[i] = items[j];
        items[j] = temp;
        i++; j--;
      }
    } while(i <= j);

    if(left < j) qs_struct(items, left, j);
    if(i < right) qs_struct(items, i, right);
  }

int main(int argc, char **argv){

int i;
int n=0;
int capacity;
double weight=0;
float r=0;
nums *mynums;
FILE *fp;
/* the average task arrival rate per hour */
float lambda=0;
double cumuprob=0;

if (argc != 4)
	{
	printf("Usage: %s <capacity> <num elements> <failure rate>\n", argv[0]);
	return 0;
	exit(0);
	}
else
	{
	fp=fopen("input.dat","r");
	if (!fp){
		perror("input.dat not found");	
		exit(0);
		}
	
	capacity=atoi(argv[1]);
	n=atoi(argv[2]);
	lambda = atof(argv[3]);
	}

mynums=malloc(n*sizeof(nums));


for (i=0;i<n; i++) 
	{
	fscanf(fp, "%d %g %g", &mynums[i].id, &mynums[i].weight, &mynums[i].prio);
	mynums[i].ratio = mynums[i].prio*mynums[i].weight;
	printf("id %i, priority * weight = %f\n", mynums[i].id,mynums[i].ratio);
	}
fclose(fp);

/*quicksort O(N^2) worst case, O(n) best*/ 
quick_struct(mynums, n);

 
i=0;
while((i<n) && (weight < capacity)) 
{
	if (weight <= capacity)
	{
	printf("selected all of %d\n", mynums[i].id);
	weight +=mynums[i].weight;	
	}
	else { 
	if(CONTINUOUS==0) break; //if it all doesn't fit just break 0/1
	r = (capacity-weight)/mynums[i].weight;
	printf("selected %2.2f%% of %d\n", 100*r, mynums[i].id);
	weight=capacity;
	}
i++;
}//end while
	

	printf("### Ordered By Ratio ###\n");
    for(i=0;i<n;i++){
       printf("id = %d weight = %f priority = %f ratio = %g\n",mynums[i].id,mynums[i].weight,mynums[i].prio,mynums[i].ratio);
    }

//note for below that the mean and variance is 2, result should be 18%
printf("poiss(2,3) = %g%%\n", 100*poisson(2,3));

printf("\n###FAILURE Stats failure rate = %f ###\n", lambda); 
printf("#item(i) arrival_time prob_1_failure prob_i_failures\n"); 
for (i=1; i<=n; i++){
	//cumuprob+=poisson(lambda,n);
	printf(" %d, %g, %g, %g\n", i,expdist(lambda), poisson(lambda,1),poisson(lambda,i));
	}

  }//end main

double factorial(int n)
{
   if(n==0)
      return(1);
   else
      return(n*factorial(n-1));
}


/* Generate a Poisson probability distribution */
//Poisson(u,x) = probability of x occuring given u rate of occurence.
// u = average rate occurences of event, 2 a day(lambda).
// x = total occurence we would like to know, what is the probability of 3 failures if we know the average is 2(u) a day.
// Note: The mean and variance of the distribution is u
double poisson(float u, int x) { return(exp(-u)*powf(u,x))/(double)factorial(x);}

/* exponentiallly random distributed arrival rate */
//digital dice, Paul Nahin, Appendix 8 pg 253
//Time = -ln(rand())/lambda;
// lambda average arrival rate per hour
double expdist(float lambda){return(ceil(3600*log(random())/lambda));}

//A cumulative Poisson probability refers to the probability that the Poisson random variable is greater than some specified lower limit and less than some specified upper limit. 
//P(x < 3, 5) = P(0; 5) + P(1; 5) + P(2; 5) + P(3; 5)
//P(x < 3, 5) = [ (e-5)(50) / 0! ] + [ (e-5)(51) / 1! ] + [ (e-5)(52) / 2! ] + [ (e-5)(53) / 3! ]
//P(x < 3, 5) = [ (0.006738)(1) / 1 ] + [ (0.006738)(5) / 1 ] + [ (0.006738)(25) / 2 ] + [ (0.006738)(125) / 6 ]
//P(x < 3, 5) = [ 0.0067 ] + [ 0.03369 ] + [ 0.084224 ] + [ 0.140375 ]
//P(x < 3, 5) = 0.2650 



