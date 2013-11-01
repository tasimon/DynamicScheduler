/************************
* Calculate Job Priority
* Tyler Simon
* 12/14/2011
* version 05 uses swf formatted files from
* http://www.cs.huji.ac.il/labs/parallel/workload/logs.html
*   1  0  6680  7560  2048  -1  -1  2048  10800  -1  -1  1  -1  -1  1  -1  -1  -1
* 
* To Compile:
* gcc -o jobpriority jobpriority.c -lm
*
* To Run:
* ./jobpriority
*
* Required: jobfile.txt
* <cores> <waittime> <runtime> <queue priority>
* 
* Required: tunables.txt
* <alpha> <beta> <gamma> <average runtime per job> <average cores per job> <system size>
* 
*************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include<string.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef IO 
#define IO 0
#endif

/*Globals*/
float alpha=0.0;
float beta=0.0;
float gam=0.0;
double avg_job_size=0.0;
double avg_runtime=0.0;
double queue_priority=0.0;
float median_cores;
float median_runtime;

FILE *ufp;
FILE *resfp;

typedef struct
        {
        int jobid;
	int cores;
	double arrvtime;  //time the job arrived
	int arrived; //1 if the job has arrived
	double waittime; //total waittime for job
	double runtime; //total job runtime
	double origruntime; //total job runtime
	double queuepr; //queue priority initial and fixed
	double priority; // scheduler assigned priority
	int running; //1 for yes 0 for no
	int complete; //1 for yes 0 for no
	int backfill; //1 if the job is being backfilled
//	char class[20];
        } Job;

int gettotalrunningjobs(int, Job *);
double gettotalrunning(int, Job *);
void backfill(Job *, int, int);
float mediancores(int, Job *);
float medianruntime(int, Job *);
int calc_prio(float,float,float, Job *, int);
void quick_struct(Job *, int);

int main(int argc, char **argv)
{
float a,b,g;

for(a=-1; a<=1; a+=0.1){
	for(b=-1; b<=1; b+=0.1){
		for(g=-1; g<=1; g+=0.1){
double cores;
double wt_time;
double runtime;
double arrtime;
FILE *fp;
FILE *jobfp;
char jobfile[255];
char line[80];
int jobcount=0;
int simtime;
int systemsize=0;
int totalavailablecores=0;
int nextjob=0;
double totalwork=0;
int longest=0, tmplong=0;
char *class;
float utilization;

int numjobs=0;

if (argc <3){
	printf("Usage: %s <jobfile> <systemsize>\n", argv[0]);
	exit(0);
}

sprintf(jobfile,"%s",argv[1]);

/*read in tunable parameter file*/
/*if(fp=fopen("tunables.txt", "r"))
	{
	fscanf(fp,"%f %f %f %d", &alpha,&beta,&gam,&systemsize);
	fclose(fp);
	printf("##Simulation_Parameters##\n");
	printf("#%s\t%s\t%s\t%s\n","Alpha(cores)","Beta(wait_t)","Gamma(run_t)","System_Size");
	printf("#%f\t%f\t%f\t%d\n", alpha,beta,gam,systemsize);
	printf("\n\n");
	}
	else {
	perror("File not Found");
	exit(0);
	}
*/
systemsize=atoi(argv[2]);

/* see how many lines (jobs) we have*/
if( (jobfp=fopen(jobfile, "r")) != NULL)
        {
        while(fgets(line, 80, jobfp) != NULL)
		{
		numjobs++;
		}
	}
	else {
	perror("Input File not Found");
	exit(0);
	}
	
fclose(jobfp);
rewind(jobfp);


Job jobqueue[numjobs]; //all waiting jobs
Job runqueue[numjobs]; //just running jobs, no priorities needed
Job waitqueue[numjobs]; //just for sorting, no priorities needed


/* Read in job file*/
if(DEBUG)printf("#JobID ArrTime WaitTime RunTime Cores QueuePriority\n");
if( (jobfp=fopen(jobfile, "r")) != NULL)
        {
        while(fgets(line, 80, jobfp) != NULL)
                {
                /* get a line, up to 80 chars from file  done if NULL */
                //sscanf(line,"%lf %lf %lf %lf", &cores,&wt_time,&runtime,&queue_priority);
//job number, submittime,waittime, runtime, cores, avg_cpu_time, used mem, requested procs, user requested time,
//we only want 1,2,3,4,5,9  
                sscanf(line,"%lf %lf %lf %lf %lf", &arrtime,&wt_time,&runtime, &cores,&queue_priority);


		jobqueue[jobcount].jobid=jobcount;
		jobqueue[jobcount].arrvtime=arrtime >= 0 ? arrtime : 0 ;
		jobqueue[jobcount].cores = cores > systemsize ? systemsize : cores;
		jobqueue[jobcount].waittime=wt_time >=0 ? wt_time : 0 ;
		jobqueue[jobcount].runtime=runtime >=0? runtime : 1;
		jobqueue[jobcount].origruntime=runtime;

		jobqueue[jobcount].priority=0;
		jobqueue[jobcount].queuepr=queue_priority;
		jobqueue[jobcount].complete=0;
		jobqueue[jobcount].arrived=0;


		/*initialize job state to not running for all jobs*/
		jobqueue[jobcount].running=0;

		/* initial calculations, static */
		totalwork+=(runtime*cores);
		//longest=runtime;	
		//if(longest>tmplong)tmplong=runtime;

	if(DEBUG)printf("%d\t%g\t%g\t%g\t%g\t%g\n", jobcount, arrtime, wt_time,runtime,cores,queue_priority);
		jobcount++;
		}
	
	}
fclose(jobfp);

/* gets average runtime and averge job size*/
avg_runtime=gettotalrunning(jobcount,jobqueue);

memcpy(waitqueue,jobqueue, sizeof(jobqueue));

median_runtime=medianruntime(jobcount,waitqueue);
median_cores=mediancores(jobcount,waitqueue);

//printf("avg_runtime = %g\n", avg_runtime);
//printf("median cores = %f\n", median_cores);
//printf("median runtime = %f\n", median_runtime);
//
//put abg here
//for(a=-1; a<=1; a+=0.1){
//	for(b=-1; b<=1; b+=0.1){
//		for(g=-1; g<=1; g+=0.1){

/* Start Simulation*/
int i=0;
int j=0;

if(DEBUG)printf("\n##### Start Simulation #####\n");

/*open output file*/
if(IO)ufp=fopen("utilization.txt", "w");

totalavailablecores=systemsize;

int k=0;
int alldone=0;
int arrived=0;
int l=0;


/* loop until all jobs are complete*/
while(alldone<jobcount)
{
if(DEBUG){
	printf("\n# Tick (%d)\n",i);
	printf("# Available cores =  %d/%d\n",totalavailablecores,systemsize);
	printf("#################################################\n");
	}
/****Incrementation Logic *******/
	calc_prio(a,b,g,runqueue, arrived);
	
/* for all jobs for this tick, see who have arrived*/
for (j=0; j<jobcount; j++)
{
	if(jobqueue[j].arrvtime==i)
		{
	if(DEBUG)printf("(%d)job %d arrived at %g, tick = %d\n", arrived,jobqueue[j].jobid, jobqueue[j].arrvtime, i);

		jobqueue[j].arrived=1;

		runqueue[j].jobid=jobqueue[j].jobid;
		runqueue[j].runtime=jobqueue[j].runtime;
		runqueue[j].cores=jobqueue[j].cores;
		runqueue[j].waittime=jobqueue[j].waittime;
		runqueue[j].running=0;
		runqueue[j].complete=0;
		runqueue[j].priority=0;
		arrived++;
		}

	//if we have at least 1 job
	//calc_prio(runqueue, arrived);
	
	/*decrement runtime,identify complete jobs*/
	if(runqueue[j].running==1 && runqueue[j].runtime > 0)
		{
		runqueue[j].runtime--;
		if(runqueue[j].runtime==0)
			{
			runqueue[j].complete=1;
			runqueue[j].running=0;
			totalavailablecores+=runqueue[j].cores;
			jobqueue[j].waittime=runqueue[j].waittime;
			alldone++;
	if(DEBUG)printf("Job %d %d cores completed!\n", runqueue[j].jobid,runqueue[j].cores);
			}
		}

	/* get next job to run  from sorted list by priority*/
	//if (jobqueue[j].running == 0 && jobqueue[j].complete == 0) 
	if (runqueue[j].running == 0 && runqueue[j].complete == 0) 
		{
		for (k=0; k<arrived; k++)
			{
			/* pull from top of priority queue*/
backfill:
			if ((runqueue[k].cores <= totalavailablecores) && runqueue[k].complete==0 && runqueue[k].running==0)
				{
				if(DEBUG)printf("k=%d, %d core job %d started\n", arrived, runqueue[k].cores,runqueue[k].jobid);
				 runqueue[k].running=1;
				 runqueue[k].priority=0;
				 totalavailablecores-=runqueue[k].cores;
				}
			else if(runqueue[k].complete==0 && runqueue[k].running==0)
				{
				//job doesnt fit, when can it start
				//jobqueue[k].backfill=1;
				//printf("cannot fit %d %d cores prio = %f\n",runqueue[k].jobid, runqueue[k].cores,runqueue[k].priority);
				// greedy backfill, just take next in line
				k++;
				goto backfill;
				//printf("%d %d %d\n",jobqueue[k].jobid, jobqueue[k].cores,jobqueue[k].backfill);
				//backfill(jobqueue,jobcount,jobqueue[k].cores);	
	//			break;
				}//end elseif
			
			}//arrived loop
		}

	/* standard wait state, not running and not complete*/
	if (runqueue[j].running == 0 && runqueue[j].complete == 0){ runqueue[j].waittime++; }


if(DEBUG && jobqueue[j].arrived ) printf("%d\t%d\t%g\t%g\t%2.3f\t%d\t%d\n", runqueue[j].jobid, runqueue[j].cores, runqueue[j].waittime, runqueue[j].runtime,runqueue[j].priority, runqueue[j].running, runqueue[j].complete);	
	}//end j


if(IO)fprintf(ufp,"%d %d %d %2.3f %d\n",i,systemsize, systemsize-totalavailablecores,(float)(systemsize-totalavailablecores)/systemsize,gettotalrunningjobs(arrived, runqueue));
	
i++; // i is our scheduler timer (tick) 
	}//end while 


if(IO)fclose(ufp);

utilization=(100*(float)totalwork/((double)systemsize*i));
double worktime=(double)totalwork/(i);

/*Print Final Report*/
if(DEBUG){
printf("#################################################\nDone!\n");
printf("%s\t%s\t%s\t%s\t%s\n","Alpha(cores)","Beta(wait_t)","Gamma(run_t)","Avg_job_size","System_Size");
printf("%f\t%12f\t%12f\t%12g\t%11d\n", alpha,beta,gam,avg_job_size,systemsize);
printf("%d Jobs run in %d ticks, total wait time = %d\n", jobcount,i, totalwaittime(jobcount,runqueue));

printf("Utilization = %2.2f percent\n", utilization);
printf("Work/Ticks = %5.3f\n", worktime);
printf("#################################################\n");
printf("ID\tARR_T\tWAIT_T\tRUN_T\tCORES\tEXP_F\tCLASS\n");
}

int llcount=0,sscount=0,slcount=0,lscount=0;
float sumll=0.0, sumss=0.0, sumsl=0.0, sumls=0.0;
float expfactor=0.0;


for (j=0; j<jobcount; j++)
	{
	//determine expansion factor
        expfactor = (jobqueue[j].origruntime+(jobqueue[j].waittime))/jobqueue[j].origruntime;

	//determine job class
        if((jobqueue[j].cores > median_cores) && (jobqueue[j].origruntime > median_runtime)){
	class="LrgLng"; //large long
	sumll+=expfactor;
	llcount++;
	}
        else if((jobqueue[j].cores > median_cores) && (jobqueue[j].origruntime < median_runtime)){
	class="LrgSht"; //large short 
	sumls+=expfactor;
	lscount++;
	}
        else if((jobqueue[j].cores < median_cores) && (jobqueue[j].origruntime > median_runtime)){
	class="SmlLng"; //small long
	sumsl+=expfactor;
	slcount++;
	}
        else {
	class="SmlSht"; // small short
	sumss+=expfactor;
	sscount++;
	}

if(DEBUG)printf("%d\t%g\t%g\t%g\t%d\t%2.2f\t%s\n", j,jobqueue[j].arrvtime, jobqueue[j].waittime,jobqueue[j].origruntime,jobqueue[j].cores,expfactor, class);
	}
	//calculate class average exp factor
 	printf("Class_Averages(LL,LS,SL,SS)(%01.1f %01.1f %01.1f) \t%2.2g \t%2.2g \t%2.2g \t%2.2g \t%2.2d \t%1.2f\n", a,b,g,sumll/llcount, sumls/lscount,sumsl/slcount,sumss/sscount, i, utilization);
	}//end gamma
       }//end beta
      }//end alpha


return 0;
} /*end Main*/

int gettotalrunningjobs(int jobs, Job *jobqueue){
int total=0;
int i;
for(i=0;i<jobs;i++)
	{
	total+=jobqueue[i].running;	
	//if(jobqueue[i].running)total+=jobqueue[i].cores;
	}
return total;
}

double gettotalrunning(int jobs, Job *jobqueue){
int total=0;
int i;
double avgtotal;
double avg;
double totalsize;
double avgsize;
for(i=0;i<jobs;i++)
	{
	avgtotal+=jobqueue[i].runtime;	
	totalsize+=jobqueue[i].cores;	
	}

avg=avgtotal/jobs;
avg_job_size=totalsize/jobs;
return avg;
}

/* Get the total time all jobs spend waiting*/
int totalwaittime(int jobs, Job *jobqueue){
int totalwt=0;
int i;
for(i=0;i<jobs;i++)
	{
	totalwt+=jobqueue[i].waittime;
	}
return totalwt;
}

void backfill(Job * jobqueue, int jobcount, int coresneeded){

double time, cores, offset;
int i;

for (i=0;  i<jobcount; i++)
	{
	//look at currently running jobs and get their runtimes and cores
	if (jobqueue[i].running == 1 && jobqueue[i].complete == 0)
	cores+= jobqueue[i].cores;
	time += jobqueue[i].runtime; 


	//coresneeded 
	}

/*set job "a" as backfill target
note that only 1 job shuold be backfill target at a time 
x =  calculate time for n cores to be available
* sort currently running jobs by time until we reach a.cores
see how many lower priority jobs we can run in x time.

see how many lowest priority jobs we will need to offset and their core count
calculate wasted cores due to backfill
*/
} //end backfill

int calc_prio(float a, float b, float g, Job * runqueue, int jobcount)
	{
	int i;		
	for (i=0;i<jobcount; i++)
		{
		if(runqueue[i].running==0 && runqueue[i].complete==0)
		{
	runqueue[i].priority=(pow((runqueue[i].cores/median_cores),a)*(pow(runqueue[i].waittime,b))*(pow(runqueue[i].runtime/median_runtime,g)));	
//		printf("a= %f b= %f g=%f id = %d, cores=%d, waittime=%g priority = %g, runtime = %g, avg_job_size = %g, avg_runtime = %g\n", alpha, beta, gam,runqueue[i].jobid, runqueue[i].cores, runqueue[i].waittime, runqueue[i].priority, runqueue[i].runtime,avg_job_size, avg_runtime);

	/* sort */
    //    quick_struct(runqueue, jobcount);
		}
		else{runqueue[i].priority=0;}

	
		}//end job loop
        quick_struct(runqueue, jobcount);

	return 0;
	}//end job prioritization

 void quick_struct(Job items[], int count)
  {
    qs_struct(items,0,count-1);
  }

  int qs_struct(Job items[], int left, int right)
  {

    register int i, j;
    double x;
    Job temp;

    i = left; j = right;
    x = items[(left+right)/2].priority;

    do {
      while((isgreater(items[i].priority,x)) && (i < right)) i++;
      while((isless(items[j].priority,x)) && (j > left)) j--;
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


float mediancores(int n, Job * queue) {
    float temp;
    int i, j;
    // the following two loops sort the array x in ascending order
    for(i=0; i<n-1; i++) {
    for(j=i+1; j<n; j++) {
    if(queue[j].cores < queue[i].cores) {
    // swap elements
    temp = queue[i].cores;
    queue[i].cores = queue[j].cores;
    queue[j].cores = temp;
    }
    }
    }
    
   if(n%2==0) {
    // if there is an even number of elements, return mean of the two elements in the middle
    return((queue[n/2].cores + queue[n/2 - 1].cores) / 2.0);
    } else {
    //else return the element in the middle
    return queue[n/2].cores;
    }
    }

float medianruntime(int n, Job * queue) {
    float temp;
    int i, j;
    // the following two loops sort the array x in ascending order
    for(i=0; i<n-1; i++) {
    for(j=i+1; j<n; j++) {
    if(queue[j].runtime < queue[i].runtime) {
    // swap elements
    temp = queue[i].runtime;
    queue[i].runtime = queue[j].runtime;
    queue[j].runtime = temp;
    }
    }
    }
    
   if(n%2==0) {
    // if there is an even number of elements, return mean of the two elements in the middle
    return((queue[n/2].runtime + queue[n/2 - 1].runtime) / 2.0);
    } else {
    //else return the element in the middle
    return queue[n/2].runtime;
    }
    }


