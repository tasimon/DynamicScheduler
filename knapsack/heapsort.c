#include<stdlib.h>
#include<stdio.h>

void heapSort(double *, int);
void siftDown(double *, int,int);
void printarray(double *,int);
//void test_main(void);


//void test_main()
int main(int argc, char ** argv)
{
double num[255];
int i;
int max=20;

srand(time(NULL));

for (i=0;i<max; i++)num[i]=drand48();

printf("presorted\n");
printarray(num,max);

heapSort(num,max);

printf("postsorted\n");
printarray(num,max);

}//end main


void printarray(double array[],int max)
	{
	int i;
	for(i=0; i<max; i++)printf("num[%d]=%2.3f\n", i,array[i]);
	}

void heapSort(double numbers[], int array_size)
    {
      int i;
      double temp;

      for (i = (array_size / 2)-1; i >= 0; i--)
        siftDown(numbers, i, array_size);

      for (i = array_size-1; i >= 1; i--)
      {
        temp = numbers[0];
        numbers[0] = numbers[i];
        numbers[i] = temp;
        siftDown(numbers, 0, i-1);
      }
    }


void siftDown(double numbers[], int root, int bottom)
    {
      int done, maxChild;
      double temp;

      done = 0;
      while ((root*2 <= bottom) && (!done))
      {
        if (root*2 == bottom)
          maxChild = root * 2;
        else if (numbers[root * 2] < numbers[root * 2 + 1])
          maxChild = root * 2;
        else
          maxChild = root * 2 + 1;

        if (numbers[root] > numbers[maxChild])
        {
          temp = numbers[root];
          numbers[root] = numbers[maxChild];
          numbers[maxChild] = temp;
          root = maxChild;
        }
        else
          done = 1;
      }
    }


