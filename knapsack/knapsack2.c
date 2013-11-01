#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*  Scheduling Simulator
 *
 * Greedy Fractional knapsack algorithm with dynamic job priorities
Tyler Simon
7/7/2012

Modified knapsack algorithm from:
 "Algorithms" by Johnsonbaugh and Schaefer 2004 pp. 316-317


Job priorities equation from:
"Scheduling Jobs on Parallel Systems Using a Relaxed Backfill Strategy, by Ward, Mahood, West (2001)


/* prints out some info */
#ifndef DEBUG
#define DEBUG 1
#endif

/* set to 0 or 1 if solving a continuous vs noncontinuous knapsack
continuous means we can take a fraction of an item, otherwise we have 0/1 knapsack */
#ifndef FRACTIONAL 
#define FRACTIONAL 0
#endif

/* Append to the end of the arff file if 1, otherwise create a new output file each time*/
#ifndef APPENDARFF
#define APPENDARFF 0 
#endif

/*global variables*/
int systemcapacity;
int coresavail;
double avg_job_size;
double avg_job_runtime;
float alpha;
float beta;
float gam;
int jobsrunning=0;
int jobscomplete=0;
double totalwork;

/* this is the wait queue*/
typedef struct{
	int id;
	int weight; //this is cores or tasks
	int origweight; //the original number of cores the jobs started with
	float totalruntime; // the totalruntime of the job
	int subjobs; // the number of subjobs to complete this jobs
	float timepertask; //totalruntime/origweight
        float waittime; //the total time the job spent waiting
	float origshare; //a fixed share value for the job
	float share; //a fixed share value for the job
	double dpriority; //dynamic priority
        int complete; //boolean 0 means waiting or running, not complete
	int running; //1 for at least 1 task is running
	}nums;

/* this is the running queue */
typedef struct{
	int id;
	int weight;
	float runtime;
	int complete;
	}runq;


/*function declarations*/
void runalljobs(nums*, runq*,int);
int longest_job(nums*, int);
void update_queues(nums*,runq *,int,int);
void print_results(nums *,runq *,int);
void print_runqueue(nums *,runq *,int);
void knapsack(nums *, runq *,int,int);
void print_summary(nums *,int,int,int);
int calc_prio(nums *, int);
int jobswaiting(nums *,int);
int corestorun(nums *,int);
int coresrunning(runq *,int);
void clearjob(runq *,int);
void increment_wait(nums *, int);
int secondstorun(runq *, int);

/* probability distributions for simulation (future)*/
double poisson(int, int);
double factorial(int);
double expdist(int);

  /* A Quicksort for structures */
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
    x = items[(left+right)/2].dpriority;

    do {
      while((isgreater(items[i].dpriority,x)) && (i < right)) i++;
      while((isless(items[j].dpriority,x)) && (j > left)) j--;
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
FILE *parameters,*jobfp;
nums *mynums;
runq *runjobs;

if (argc != 6)
	{
	printf("Usage: %s <alpha> <beta> <gamma> <capacity> <num elements>\n", argv[0]);
	return 0;
	exit(0);
	}
else
	{
	jobfp=fopen("knapsack2.inp","r");
	alpha=atof(argv[1]);
	beta=atof(argv[2]);
	gam=atof(argv[3]);
	systemcapacity=atoi(argv[4]);
	n=atoi(argv[5]);
	}

mynums=malloc(n*sizeof(nums));

double sumweight;
double sumruntime;
int quantum=1; //time value for simlulation, start at 1 second not zero

sumruntime=0;
sumweight=0;
for (i=0;i<n; i++) 
	{
	fscanf(jobfp, "%d %d %f %f %f", &mynums[i].id, &mynums[i].origweight, &mynums[i].timepertask,&mynums[i].waittime,&mynums[i].origshare);
	mynums[i].weight=mynums[i].origweight;	//we start at the max weight (cores) 
	mynums[i].totalruntime=mynums[i].timepertask;
	mynums[i].share=mynums[i].origshare;
	mynums[i].subjobs=0;
	sumweight+=mynums[i].origweight;
	sumruntime+=mynums[i].totalruntime;
	}
fclose(jobfp);

runjobs=malloc(n*sizeof(runq)*1000);
if(DEBUG)print_results(mynums,runjobs,n);

totalwork=sumruntime*sumweight;

avg_job_runtime=sumruntime/n;
avg_job_size=sumweight/n;
//printf("#Average Job Runtime = %f\n",avg_job_runtime);
//printf("#Average Job Size = %f\n",avg_job_size);


/*quicksort O(N^2) worst case, O(n) best*/ 

int totalcorestorun;
int totalcoresrunning;
int coresavail=systemcapacity; //knapsack starts empty;
int j;
int myid;

/* simulation logic */
//while(n!=jobswaiting(mynums,n)) //while there are still jobs to run
while((corestorun(mynums,n)>0) || secondstorun(runjobs,jobsrunning)) //while there are still jobs to run or there are jobs in the runqueue with time left
	{
	if(DEBUG){
	printf("\n########### tick %d #############\n", quantum);
	printf("seconds needed to run = %d\n", secondstorun(runjobs,jobsrunning));
	}
	
	/* (1) calculate the priorities of all waiting jobs*/
	calc_prio(mynums,n);


	/* (2) start the top n tasks  from the highest priority jobs that can fit the available capacity*/
	/* if all of the remaining cores can fit start them all based on their order in the queue*/
	totalcorestorun=corestorun(mynums,n); //check waiting queue
	totalcoresrunning=coresrunning(runjobs,jobsrunning); //check running queue
	
	/*fill the knapsack with tasks proportional to the fair(QOS) value*/
	if(totalcorestorun > 0)
	{ 
	if(totalcorestorun > systemcapacity)knapsack(mynums, runjobs,(systemcapacity-totalcoresrunning),n);
	else if( totalcorestorun <= (systemcapacity-totalcoresrunning) ) {
		runalljobs(mynums,runjobs,n);
	      }
	}

	/* (4) check who is waiting and running; if waiting increase wait time, if running decrease runtime */
	update_queues(mynums,runjobs,jobsrunning,n);

	/* update waittime if any jobs are not running or waiting*/
	increment_wait(mynums,n);	

	if (DEBUG){
		print_results(mynums, runjobs,n);
		print_runqueue(mynums, runjobs,jobsrunning);
		}
	quantum++;
	}

/* printf the results in ARFF format */
print_summary(mynums,n,systemcapacity,quantum);

/* here is where we can play with statisticall distributions of arrival rates, etc...*/
//printf("poiss(2,3) = %g%%\n", 100*poisson(2,3));
free(mynums);
return 0;
}//end main

void runalljobs(nums *mynums, runq *runjobs, int n){
int i=0;
if(DEBUG)printf("!!!!!starting all remaining jobs!!!!!!!!!\n");
	for (i=0; i<n; i++){
	//mynums[i].share=1.0;

	/* place work in the running queue*/
	runjobs[jobsrunning].id=mynums[i].id;
	runjobs[jobsrunning].weight=mynums[i].weight;

	mynums[i].running=1;
	mynums[i].weight=0; //now set the jobs weight to zero in the waiting queue 
	mynums[i].subjobs++;

	//runjobs[jobsrunning].runtime=ceilf((mynums[i].timepertask*runjobs[jobsrunning].weight*100+0.5)/100); //set the runtime for the subset of tasks
	runjobs[jobsrunning].runtime=mynums[i].timepertask; //set the runtime for the subset of tasks
	jobsrunning++;
	}
}

int longest_job(nums *mynums, int n){
int i;
int temp=1;
for (i=0; i<n; i++){
	if (temp<mynums[i].totalruntime) temp=mynums[i].totalruntime;
	}
return temp;
}
/* increase waittime only for jobs that  have 0 tasks in the runqueue*/
void increment_wait(nums *mynums, int n){
int i;
for (i=0; i<n; i++)
	{
	if ((mynums[i].weight > 0) && (mynums[i].running==0)) mynums[i].waittime++; // new version waittime++ for any job not running
	}
}//end increment wait


int secondstorun(runq *runnums, int jrunning){
int total=0;
int i;
	for(i=0; i < jrunning; i++){
	total+=runnums[i].runtime;
	}
return (total<0)?0:total; //no negatives, due to rounding we get a few seconds overhang
}//end secondstorun


void update_queues(nums *mynums,runq *runnums,int jrunning, int n){
int i;
int j;
/* for all jobs in run queue, check if completed */
for (i=0; i<jrunning; i++){
	if( runnums[i].runtime > 0){
	runnums[i].runtime-=1;
	}
	else if((runnums[i].runtime == 0) && (runnums[i].complete == 0)){
		runnums[i].complete=1; //stop it, its done
		if(DEBUG)printf("(%d) completed!\n", runnums[i].id);
		for (j=0; j<n; j++){
		if(runnums[i].id == mynums[j].id)mynums[i].running=0;
		}
		clearjob(runnums,i);
		}
	}
}//end update queues


void clearjob(runq *runnums,int n){
runnums[n].weight=0;
runnums[n].runtime=0;
runnums[n].complete=1;
}

int jobswaiting(nums *mynums,int n){
int total=0;
int i;
for (i=0; i<n; i++)
	{
	total+=mynums[i].complete;
	}
if(DEBUG)printf("jobs complete = %d\n", total);
return total;
}

int corestorun(nums *mynums,int n){
int total=0;
int i;
for (i=0; i<n; i++)
	{
	total+=mynums[i].weight;
	}
if(DEBUG)printf("tasks needed to run = %d\n", total);
return total;
}

int coresrunning(runq *mynums,int n){
int total=0;
int i;
for (i=0; i<n; i++)
	{
	total+=mynums[i].weight;
	}
return total;
}

/* The Knapsack Algorithm; Greedy and Cruel */
void knapsack(nums *mynums, runq *runjobs,int capacity,int n){
int i=0;
float fairshare=0;
float fillfairshare=0;
int knapsackweight=0;
float ratio;
while((i<n) && (knapsackweight < capacity)) 
{

 if(FRACTIONAL==1)fairshare=ceil(mynums[i].share*mynums[i].weight); //get fairshare cores from the highest priority job
	else fairshare=mynums[i].weight;


	if ((knapsackweight+fairshare) < capacity  && fairshare > 0 )
	{
	if(DEBUG)printf("(%d) selected %f/%d tasks share = %2.1f%% prio = %g time per task = %f\n", mynums[i].id,fairshare,mynums[i].weight,100*mynums[i].share, mynums[i].dpriority,mynums[i].timepertask);
	
	knapsackweight += fairshare;	//increment the weight in the knapsack
	mynums[i].weight-=fairshare; //update the cores in wait queue
	runjobs[jobsrunning].id=mynums[i].id;
	runjobs[jobsrunning].weight=fairshare;

	mynums[i].running=1;
	mynums[i].subjobs++;

	//note if we select all of the remaining tasks for a ob the runtime must not be proportinal, they must be all remaining runtime
	if(DEBUG)printf("fair = %f weight= %f\n", fairshare,(float)mynums[i].weight);
		
	runjobs[jobsrunning].runtime=mynums[i].timepertask; //set the runtime for the subset of tasks

	if(DEBUG){
	printf("running for %f seconds\n", runjobs[jobsrunning].runtime);
	printf("knapsack weight= %d\n", knapsackweight);
	}
	jobsrunning++;
	}
	else { 
	if(FRACTIONAL==0)break; //if it all doesn't fit just break 0/1
	//calculate fill value which below the fairshare
	fillfairshare=ceilf(capacity-knapsackweight);	

	if(DEBUG)printf("(%d) fill:selected %f/%d tasks share = %2.1f%% prio = %g time per task = %f\n", mynums[i].id,fillfairshare,mynums[i].weight,100*mynums[i].share, mynums[i].dpriority,mynums[i].timepertask);
	mynums[i].weight-=fillfairshare; //decrement knapsack capacity

	runjobs[jobsrunning].id=mynums[i].id;
	runjobs[jobsrunning].weight=fillfairshare;

	mynums[i].running=1;
	mynums[i].subjobs++;
	if(DEBUG)printf("fillfair = %f weight= %d\n", fillfairshare,mynums[i].weight);

	runjobs[jobsrunning].runtime=mynums[i].timepertask; //set the runtime for the subset of tasks

	knapsackweight=capacity;
	if(DEBUG){
	printf("fill:running for %f seconds\n", runjobs[jobsrunning].runtime);
	printf("fill:knapsack weight = %d\n", knapsackweight);
	}
	jobsrunning++;
	}

coresavail=systemcapacity-coresrunning(runjobs,jobsrunning);

i++; //next job in wait queue
}//end while
if(DEBUG)printf("cores avail = %d\n", coresavail);
	
}//end knapsack

void print_results(nums *mynums, runq *runjobs,int n)
{
int i;
printf("### Ordered Wait Queue ###\n");
   	for(i=0;i<n;i++)
	{
        printf("(%d) (c%d) (r%d) id= %d origw= %d origt= %f weight= %d share= %f pr= %g wt= %f tpt= %f\n",i,mynums[i].complete, mynums[i].running,mynums[i].id,mynums[i].origweight,mynums[i].totalruntime,mynums[i].weight,mynums[i].share, mynums[i].dpriority, mynums[i].waittime, mynums[i].timepertask);
    	}
}

void print_runqueue(nums *mynums, runq *runjobs,int n)
{
int i;
printf("### The Running Queue ( total jobs = %d) ###\n",n);
   	for(i=0;i<n;i++)
	{
	if (runjobs[i].runtime > 0)
        printf(" (%d) (c%d) id= %d weight= %d rt= %f\n",i,runjobs[i].complete,runjobs[i].id,runjobs[i].weight, runjobs[i].runtime);
    	}
}

double factorial(int n)
{
   if(n==0)
      return(1);
   else
      return(n*factorial(n-1));
}

/* The priority calculation from Ward et al*/
int calc_prio(nums *jobqueue, int jobcount) {
int i;
for (i=0;i<jobcount; i++) {
	if(jobqueue[i].complete==0 && jobqueue[i].weight > 0){
	jobqueue[i].dpriority=(pow(jobqueue[i].waittime,alpha)*(pow((jobqueue[i].timepertask/avg_job_runtime),beta))*(pow(jobqueue[i].weight/avg_job_size,gam)));
        quick_struct(jobqueue, jobcount);
		}
	else jobqueue[i].dpriority=0;

	}//end job loop

return 0;
}//end job prioritization

void print_summary(nums *mynums, int n, int capacity, int ticks){
int i;
int longest;
int work;
double perfratio;
FILE *out;
char *class;
float expfactor;
/* ARFF format
jobID tasks runtime waittime failval alpha beta gamma capacity utilization totaljobs
*/
if (APPENDARFF==0){
out=fopen("summary.arff","w");
fprintf(out,"@relation fairshare\n\
@attribute jobid numeric\n\
@attribute tasks numeric\n\
@attribute runtime numeric\n\
@attribute waittime numeric\n\
@attribute fairval numeric\n\
@attribute alpha numeric\n\
@attribute beta numeric\n\
@attribute gamma numeric\n\
@attribute capacity numeric\n\
@attribute totaljobs numeric\n\
@attribute tics numeric\n\
@attribute timepertask numeric\n\
@attribute ResponseValue numeric\n\
@attribute class {LrgLng,LrgSht,SmlLng,SmlSht}\n\
@data\n");
}

if (APPENDARFF==1)out=fopen("summary.arff", "a");

//avg_job_runtime=sumruntime/n;
//avg_job_size=sumweight/n;

work = capacity*ticks;
perfratio=((double)longest/ticks);


/* collect and print the data */
for (i=0;i<n; i++) {

expfactor = ((mynums[i].timepertask*mynums[i].subjobs)+mynums[i].waittime)/(mynums[i].timepertask*mynums[i].subjobs); 

if(DEBUG)printf("(%d)  ran %d jobs %2.1f each, total time = %g  waittime = %2.1f  exp = %2.1f ticks = %d work = %d\n", mynums[i].id, mynums[i].subjobs, mynums[i].timepertask,(mynums[i].subjobs*mynums[i].timepertask),mynums[i].waittime, expfactor, ticks,work);

	
	/*classify jobs (size,duration)*/
	if((mynums[i].origweight > avg_job_size) && (mynums[i].totalruntime > avg_job_runtime))class="LrgLng"; //large long
	else if((mynums[i].origweight > avg_job_size) && (mynums[i].totalruntime < avg_job_runtime))class="LrgSht"; //large short 
	else if((mynums[i].origweight < avg_job_size) && (mynums[i].totalruntime > avg_job_runtime))class="SmlLng"; //small long
	else class="SmlSht"; // small short

	fprintf(out,"%d, %d, %2.1f, %2.1f, %g, %2.1f, %2.1f, %2.1f, %d, %d, %d, %2.1f, %2.1f, %s\n",
	mynums[i].id, mynums[i].origweight, mynums[i].timepertask*mynums[i].subjobs, mynums[i].waittime,mynums[i].origshare, alpha,beta,gam, capacity, n, ticks, mynums[i].timepertask, expfactor,class);
	}
fclose(out);

}

/* Generate a Poisson probability distribution */
//Poisson(u,x) = probability of x occuring given u rate of occurence.
// u = average rate occurences of event, 2 a day(lambda).
// x = total occurence we would like to know, what is the probability of 3 failures if we know the average is 2(u) a day.
double poisson(int u, int x) { return(exp(-u)*pow(u,x))/(double)factorial(x);}

/* exponentiallly random distributed arrival rate */
//digital dice, Paul Nahin, Appendix 8 pg 253
//Time = -ln(rand())/lambda;
// lambda average arrival rate per hour
double expdist(int lambda){return(ceil(3600*log(random())/lambda));}


