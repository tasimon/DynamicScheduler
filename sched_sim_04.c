/************************
* HPC Scheduler using 
* single Queue with individual job Priorities
* Tyler Simon
* 10/14/2012
* 
* 
*************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* If set prints utilization table*/
#ifndef DEBUG
#define DEBUG 1
#endif


/*Globals*/
double alpha=0.0;
double beta=0.0;
double gam=0.0;
double avg_job_size=0.0;
double avg_runtime=0.0;
double queue_priority=0.0;

FILE *ufp;
FILE *resfp;

typedef struct
        {
        int jobid;
	int cores;
	double arrtime;  //time the job arrived
	double waittime; //total waittime for job
	double runtime; //total job runtime
	double origruntime; //initial job runtime
	double queuepr; //queue priority initial and fixed
	double priority; // scheduler assigned priority
	int running; //1 for yes 0 for no
	int complete; //1 for yes 0 for no
	int backfill; //1 if the job is being backfilled
        } Job;

void expfactor(int, Job *,int);
int gettotalrunningjobs(int, Job *);
double gettotalrunning(int, Job *);
void backfill(Job *, int, int);
double factorial(int);
double poisson(int, int);

void quick_struct(Job *, int);

int main(int argc, char **argv)
{
double cores;
double wt_time;
double runtime;
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

int numjobs=0;

if (argc <6){
	printf("Usage: %s <jobfile>\n", argv[0]);
	exit(0);
}

alpha=atof(argv[1]);
beta=atof(argv[2]);
gam=atof(argv[3]);
systemsize=atoi(argv[4]);
sprintf(jobfile,"%s",argv[5]);

/*read in tunable parameter file
 * I've added them as cmd args but sometime it might be nice to just have them in a file?
 */
/*
if(fp=fopen("tunables.txt", "r"))
	{
	fscanf(fp,"%lf %lf %lf %d", &alpha,&beta,&gam,&systemsize);
	fclose(fp);
	if(DEBUG){
		printf("##Simulation_Parameters##\n");
		printf("#%s\t%s\t%s\t%s\n","Alpha(cores)","Beta(wait_t)","Gamma(run_t)","System_Size");
		printf("#%g\t%g\t%g\t%d\n", alpha,beta,gam,systemsize);
		printf("\n\n");
		}
	}
	else {
	perror("File not Found");
	exit(0);
	}
*/

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

Job jobqueue[numjobs];

/* Read in job file*/
if(DEBUG)printf("#JobID Cores WaitTime RunTime QueuePriority\n");
if( (jobfp=fopen(jobfile, "r")) != NULL)
        {
        while(fgets(line, 80, jobfp) != NULL)
                {
                /* get a line, up to 80 chars from file  done if NULL */
                sscanf(line,"%lf %lf %lf %lf", &cores,&wt_time,&runtime,&queue_priority);
		jobqueue[jobcount].jobid=jobcount;
		jobqueue[jobcount].cores = cores > systemsize ? systemsize : cores;
		jobqueue[jobcount].waittime=wt_time;
		jobqueue[jobcount].runtime=runtime;
		jobqueue[jobcount].origruntime=runtime;
		jobqueue[jobcount].priority=0;
		jobqueue[jobcount].queuepr=queue_priority;
		jobqueue[jobcount].complete=0;

		/*initialize job state to not running for all jobs*/
		jobqueue[jobcount].running=0;

		/* initial calculations, static */
		totalwork+=(runtime*cores);
		longest=runtime;	
		if(longest>tmplong)tmplong=runtime;

	if(DEBUG)printf("%d\t%g\t%g\t%g\t%g\n", jobcount,cores, wt_time,runtime,queue_priority);
		jobcount++;
		}
	
	}

avg_runtime=gettotalrunning(jobcount,jobqueue);

/* Start Simulation*/
int i=0;
int j;

if(DEBUG)printf("\n##### Start Simulation #####\n");

/*open output file*/
ufp=fopen("utilization.txt", "w");

totalavailablecores=systemsize;

int k=0;
int alldone=0;

/* loop until all jobs are complete*/
while(alldone<jobcount)
{
i++; // i is our scheduler timer (tick) 
if(DEBUG){
printf("\n# Tick (%d)\n",i);
printf("# Available cores =  %d/%d\n",totalavailablecores,systemsize);
printf("#################################################\n");
	}

//check arrival time
//if(arr_time==tick);


	/* calculate priorities*/
	calc_prio(jobqueue, jobcount);
	

/****Incrementation Logic *******/
	for(j=0; j<jobcount; j++)
	{
	/*decrement runtime,identify complete jobs*/
	if(jobqueue[j].running==1 && jobqueue[j].runtime > 0)
		{
		jobqueue[j].runtime--;
			if(jobqueue[j].runtime==0)
			{
			jobqueue[j].complete=1;
			jobqueue[j].running=0;
			totalavailablecores+=jobqueue[j].cores;
			alldone++;
	if(DEBUG)printf("Job %d %d cores completed!\n", jobqueue[j].jobid,jobqueue[j].cores);
			}
		}

	/* get next job to run  from sorted list by priority*/
	//if (jobqueue[j].running == 0 && jobqueue[j].complete == 0 ) 
//		{
		for (k=0; k<jobcount; k++)
			{
			rerun:
			/* pull from top of priority queue*/
			if ((jobqueue[k].cores <= totalavailablecores) && jobqueue[k].complete==0 && jobqueue[k].running==0)
				{
				if(DEBUG)printf("%d core job %d started %f\n", jobqueue[k].cores,jobqueue[k].jobid,jobqueue[k].priority);
				 jobqueue[k].running=1;
				 jobqueue[k].priority=0;
				 totalavailablecores-=jobqueue[k].cores;
				}
			else if(jobqueue[k].complete==0 && jobqueue[k].running==0)
				{
				/*job doesnt fit, when can it start*/
				//jobqueue[k].backfill=1;
			//	printf("cannot fit %d %d cores prio = %f\n",jobqueue[k].jobid, jobqueue[k].cores,jobqueue[k].priority);
			//	printf("%d %d %d\n",jobqueue[k].jobid, jobqueue[k].cores,jobqueue[k].backfill);
				//backfill(jobqueue,jobcount,jobqueue[k].cores);	
				k++;
				goto rerun;
				break;
				}//end elseif
			}

		//}

	/* standard wait state*/
	if (jobqueue[j].running == 0 && jobqueue[j].complete == 0) jobqueue[j].waittime++;


if(DEBUG && jobqueue[j].complete==0)printf("%d\t%d\t%g\t%g\t%g\t%d\t%d\n", jobqueue[j].jobid, jobqueue[j].cores, jobqueue[j].waittime, jobqueue[j].runtime,jobqueue[j].priority, jobqueue[j].running, jobqueue[j].complete);	
	}//end j



if(DEBUG){
//prints utilization
fprintf(ufp,"%d %d %d %2.3f %d\n",i,systemsize, systemsize-totalavailablecores,(float)(systemsize-totalavailablecores)/systemsize,gettotalrunningjobs(jobcount, jobqueue));
}
	
	}//end while 
fclose(ufp);

/*Print Final Report*/
FILE *waitfp;
waitfp=fopen("wait.txt", "a");
fprintf(waitfp, "%d %f %f %f %d %d %2.2f\n", systemsize, alpha, beta, gam, i, totalwaittime(jobcount, jobqueue), 100*(float)totalwork/(systemsize*i)); 
fclose(waitfp);

if(DEBUG){
printf("#################################################\nDone!\n");
printf("%s\t%s\t%s\t%s\t%s\n","Alpha(cores)","Beta(wait_t)","Gamma(run_t)","Avg_job_size","System_Size");
printf("%g\t%12g\t%12g\t%12g\t%11d\n", alpha,beta,gam,avg_job_size,systemsize);

printf("%d Jobs run in %d ticks, total wait time = %d\n\
Size*Ticks = %d\n\
Total Work = sum(cores*runtime) = %g\n", jobcount,i, totalwaittime(jobcount,jobqueue), systemsize*i,totalwork);
printf("Utilization = %2.2f percent\n", 100*(float)totalwork/(systemsize*i));
double worktime=(double)totalwork/(i);
printf("Work/Ticks = %5.3f\n", worktime);

//print per job expansion factor
expfactor(jobcount,jobqueue, systemsize);
printf("#################################################\n");
}

return 0;
} /*end Main*/

void expfactor(int jobs, Job *jobqueue, int syssize){
int i;
char *class;
double efactor=0.0;
FILE *fp;

class=malloc(10*sizeof(char));
fp=fopen("expf.txt", "w");

fprintf(fp,"#ID\tcores\truntime\twaittime\texpFactor ratio_d ratio_s\n");
for(i=0;i<jobs;i++){
efactor=(jobqueue[i].origruntime+jobqueue[i].waittime)/jobqueue[i].origruntime;


      if((jobqueue[i].cores >= (syssize/2)) && (jobqueue[i].origruntime > avg_runtime))class="LrgLng"; //large long
        else if((jobqueue[i].cores > 0.5*syssize) && (jobqueue[i].origruntime < avg_runtime))class="LrgSht"; //large short 
        else if((jobqueue[i].cores < 0.5*syssize) && (jobqueue[i].origruntime > avg_runtime))class="SmlLng"; //small long
        else class="SmlSht"; // small short

fprintf(fp,"%3d %3d %3g %3g %2.2f %2.2f %2.2f %s\n",jobqueue[i].jobid, jobqueue[i].cores,jobqueue[i].origruntime,jobqueue[i].waittime, efactor, jobqueue[i].origruntime/avg_runtime,(float)jobqueue[i].cores/syssize, class);
}
fclose(fp);
}

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
double totalsize=0;
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

int calc_prio(Job * jobqueue, int jobcount)
	{
	int i;		
	double *nextjob;

	nextjob=malloc(jobcount*sizeof(double));
	
	for (i=0;i<jobcount; i++)
		{
		if(jobqueue[i].running==0 && jobqueue[i].complete==0)
		{
	jobqueue[i].priority=(pow((jobqueue[i].cores/avg_job_size),alpha)*(pow(jobqueue[i].waittime,beta))*(pow(jobqueue[i].runtime/avg_runtime,gam)) * jobqueue[i].queuepr);	
	/* sort */
        quick_struct(jobqueue, jobcount);
		}
		else jobqueue[i].priority=0;
	
		}//end job loop

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

/* Generate a Poisson probability distribution */
//Poisson(u,x) = probability of x occuring given u rate of occurence.
// u = average rate occurences of event, 2 a day(lambda).
// x = total occurence we would like to know, what is the probability of 3 failures if we know the average is 2(u) a day.
double poisson(int u, int x) { return(exp(-u)*pow(u,x))/(double)factorial(x);}
double factorial(int n)
{
   if(n==0)
      return(1);
   else
      return(n*factorial(n-1));
}


