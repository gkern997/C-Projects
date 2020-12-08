#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>

#define MSG "* running cpubench %s using %s with size %s and %s threads...\n"

#define USAGE "usage: ./cpubench <mode> <type> <size> <threads> \n" \
"     - mode: flops / matrix \n" \
"     - type: single / double \n" \
"     - size: 10 / 100 / 1000 / 1024 / 4096 / 16386 \n" \
"     - threads: 1 / 2 / 4 \n"

#define GIGAFLOPS 1000000000
#define GIGABYTES 1024*1024*1024
#define MAX_THREADS 4

typedef struct multArgsD // Struct for double matrices.
{
	double **mat1;
	double **mat2;
	double **res;
	int threadID, numThreads, size;

}multArgsD;

typedef struct multArgsI // Struct for integer matrices.
{
	int **mat1;
	int **mat2;
	int **res;
	int threadID, numThreads, size;

}multArgsI;

typedef struct flopArgs // Struct for flops.
{
	int size, numThreads, threadID, intResult;
	double doubleResult;

}flopArgs;

// This function multiplies mat1[][] and mat2[][],
// and stores the result in res[][]
void *multiply_int(void *args)
{
	int i, j, k, temp;

	multArgsI *margs; // Pass in matrix arguments. 
	margs = (multArgsI *) args;

	for(i = 0; i < margs -> size; i++)
	{
		for(j = 0; j < margs -> size; j++)
		{
			temp = margs -> mat2[i][j];
			margs -> mat2[i][j] = margs -> mat2[j][i]; // Transpose the second matrix.
			margs -> mat2[j][i] = temp;
		}
	}

	for(i = margs -> threadID; i < margs -> size; i += margs -> numThreads)
	{
		for(j = margs -> threadID; j < margs -> size; j += margs -> numThreads)
		{
			for(k = margs -> threadID; k < margs -> size; k += margs -> numThreads)
			{
				margs -> res[i][j] += margs -> mat1[i][k] * margs-> mat2[j][k]; // Compute dot products.
			}
		}
	}

	pthread_exit(NULL);
}

// This function multiplies mat1[][] and mat2[][],
// and stores the result in res[][]
void *multiply_double(void *args)
{
	multArgsD *margs; // Pass in matrix arguments.
	int i, j, k;
	double temp;

	margs = (multArgsD *) args;

	for(i = 0; i < margs -> size; i++)
	{
		for(j = 0; j < margs -> size; j++)
		{
			temp = margs -> mat2[i][j];
			margs -> mat2[i][j] = margs -> mat2[j][i]; // Transpose.
			margs -> mat2[j][i] = temp;
		}
	}

	for(i = margs -> threadID; i < margs -> size; i+= margs -> numThreads)
	{
		for(j = margs -> threadID; j < margs -> size; j+= margs -> numThreads)
		{
			for(k = margs -> threadID; k < margs -> size; k+= margs -> numThreads)
			{
				margs -> res[i][j] += margs -> mat1[i][k] * margs -> mat2[j][k]; // Dot products.
			}
		}
	}

	pthread_exit(NULL);
}


void *compute_flops_int(void *args)
{
	flopArgs *fargs; // Pass in flop arguments.
	fargs = (flopArgs *) args;

	unsigned long long int index;
	int numFlops = (fargs -> size) / (fargs -> numThreads);
	unsigned long long int loops = (numFlops * (unsigned long long) GIGAFLOPS) / 2; // Amount of flops.

	for (index = fargs -> threadID; index < loops; index += fargs -> numThreads)
	{
		fargs -> intResult = fargs -> intResult + (index * 2); // Perform computations.
	}

	printf("%ull\n", fargs -> intResult); // Print result so calculations aren't optimized too much.
	pthread_exit(NULL);
}

void *compute_flops_double(void *args)
{
	flopArgs *fargs; // Pass in flop arguments.
	fargs = (flopArgs *) args;
	unsigned long long int index;
	int numFlops = (fargs -> size) / (fargs -> numThreads);
	unsigned long long int loops = (numFlops * (unsigned long long) GIGAFLOPS) / 2; // Amount of flops.

	for (index = fargs -> threadID; index < loops; index += fargs -> numThreads)
	{
		fargs -> doubleResult = fargs -> doubleResult + (index * 2); // Computations
	}
	
	printf("%f\n", fargs -> doubleResult); // Print result to avoid optimization.
	pthread_exit(NULL);
}


int main(int argc, char **argv)
{
	time_t t;
	srand((unsigned) time(&t));
	
    	if (argc != 5) 
    	{
        	printf(USAGE);
        	exit(1);
    	} 
    	else 
    	{
		int mode = -1;

        	if(strcmp(argv[1],"flops") == 0)
        		mode = 0;

        	else if(strcmp(argv[1],"matrix") == 0)
        		mode = 1;

        	else
        		mode = -1;

		int type = -1;

        	if(strcmp(argv[2],"single") == 0)
        		type = 0;

        	else if(strcmp(argv[2],"double") == 0)
        		type = 1;

        	else
        		type = -1;

		
        	unsigned long long int size = atoi(argv[3]);
        	int num_threads = atoi(argv[4]);
		int i, j, k, r;
		double **mat1, **mat2, **res;
		int **mat1I, **mat2I, **resI;
		struct timeval start, end;
		multArgsD margsD[num_threads];
		multArgsI margsI[num_threads];
		flopArgs fargs[num_threads];
		pthread_t threads[num_threads];

		if (mode == 0 && type == 0) // flops int
		{	
			for(i = 0; i < num_threads; i++)
			{
				fargs[i].size = size;
				fargs[i].numThreads = num_threads; // Init flops struct.
				fargs[i].threadID = i;
				fargs[i].intResult = 0;
				fargs[i].doubleResult = 0;
			}

			gettimeofday(&start, NULL);

			for(i = 0; i < num_threads; i++)
			{
				r = pthread_create(&threads[i], NULL, compute_flops_int, (void *) &fargs[i]); // Create threads to run int flops.

				if(r)
				{
					printf("Error: unable to create one or more threads\n");
					return 1;
				}
			}

			for(i = 0; i < num_threads; i++)
			{
				r = pthread_join(threads[i], NULL); // Join the threads.

				if(r)
				{
					printf("Error: unable to join one or more threads\n");
					return 1;
				}
			}

			gettimeofday(&end, NULL);
		}
		else if (mode == 0 && type == 1) // flops double 
		{	
			
			for(i = 0; i < num_threads; i++) // Only difference between flops single are the types.
			{
				fargs[i].size = size;
				fargs[i].numThreads = num_threads;
				fargs[i].threadID = i;
				fargs[i].intResult = 0;
				fargs[i].doubleResult = 0;
			}
			
			gettimeofday(&start, NULL);

			for(i = 0; i < num_threads; i++)
			{
				r = pthread_create(&threads[i], NULL, compute_flops_double, (void *) &fargs[i]);

				if(r)
				{
					printf("Error: unable to create one or more threads\n");
					return 1;
				}
			}

			for(i = 0; i < num_threads; i++)
			{
				r = pthread_join(threads[i], NULL);

				if(r)
				{
					printf("Error: unable to join one or more threads\n");
				}
			}

		   	gettimeofday(&end, NULL);
		}		
		else if (mode == 1 && type == 0) // matrix int
		{
			mat1I = (int **) malloc(sizeof(int *) * size);
			mat2I = (int **) malloc(sizeof(int *) * size);
			resI = (int **) malloc(sizeof(int *) * size);

			for(i = 0; i < size; i++)
			{
				mat1I[i] = (int *) malloc(sizeof(int) * size);
				mat2I[i] = (int *) malloc(sizeof(int) * size); // Allocate the memory needed for 3 matrices.
				resI[i] = (int *) malloc(sizeof(int) * size);

				for(j = 0; j < size; j++)
				{
					mat1I[i][j] = (int) rand();
					mat2I[i][j] = (int) rand();
					resI[i][j] = 0;
				}
			}

			for(k = 0; k < num_threads; k++)
			{
				margsI[k].mat1 = mat1I;
				margsI[k].mat2 = mat2I;
				margsI[k].res = resI; // Struct init.
				margsI[k].threadID = k;
				margsI[k].numThreads = num_threads;
				margsI[k].size = size;
			}

			gettimeofday(&start, NULL);

			for(i = 0; i < num_threads; i++)
			{
				r = pthread_create(&threads[i], NULL, multiply_int, (void *) &margsI[i]); // Thread creation and joining.

				if(r)
				{
					printf("Error: unable to create one or more threads\n");
					return 1;
				}
			}

			for(i = 0; i < num_threads; i++)
			{
				r = pthread_join(threads[i], NULL);

				if(r)
				{
					printf("Error: unable to join one or more threads\n");
					return 1;
				}
			}

			gettimeofday(&end, NULL);

			for(i = 0; i < size; i++)
			{
				free(mat1I[i]);
				free(mat2I[i]);
				free(resI[i]);
			}

			free(mat1I);
			free(mat2I); // Free the allocated memory.
			free(resI);
		}
		else if (mode == 1 && type == 1) // matrix double
		{
			mat1 = (double **) malloc(sizeof(double *) * size);
			mat2 = (double **) malloc(sizeof(double *) * size); // Again largely the same as matrix single save for the types.
			res = (double **) malloc(sizeof(double *) * size);

			for(i = 0; i < size; i++)
			{
				mat1[i] = (double *) malloc(sizeof(double) * size);
				mat2[i] = (double *) malloc(sizeof(double) * size);
				res[i] = (double *) malloc(sizeof(double) * size);

				for(j = 0; j < size; j++)
				{
					mat1[i][j] = rand();
					mat2[i][j] = rand();
					res[i][j] = 0.0;
				}
			}

			for(k = 0; k < num_threads; k++)
			{
				margsD[k].mat1 = mat1;
				margsD[k].mat2 = mat2;
				margsD[k].res = res;
				margsD[k].threadID = k;
				margsD[k].numThreads = num_threads;
				margsD[k].size = size;	
			}
			
			gettimeofday(&start, NULL);

			for(k = 0; k < num_threads; k++)
			{
				r = pthread_create(&threads[k], NULL, multiply_double, (void *) &margsD[k]);

				if(r)
				{
					printf("Error: unable to create one or more threads\n");
					return 1;
				}
			}
			

			for(k = 0; k < num_threads; k++)
			{
				r = pthread_join(threads[k], NULL);

				if(r)
				{
					printf("Error: failed to join one or more threads\n");
					return 1;
				}
			}

		   	gettimeofday(&end, NULL);

			for(i = 0; i < size; i++)
			{
				free(mat1[i]);
				free(mat2[i]);
				free(res[i]);
			}
			
			free(mat1);
			free(mat2);
			free(res);
		}
		else
		{
        		printf(USAGE);
			printf("unrecognized option, exiting...\n");
        		exit(1);
		}

		double elapsed_time_sec = (((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)))/1000000.0; // Compute elapsed time.
		double  num_giga_ops = 0;
		
		if (size * GIGAFLOPS < 0)
		{
			printf("error in size, check for overflow; exiting...\n");
			exit(1);
		}
		if (elapsed_time_sec < 0)
		{
			printf("error in elapsed time, check for proper timing; exiting...\n");
			exit(1);
		}
		if (elapsed_time_sec == 0)
		{
			printf("elapsed time is 0, check for proper timing or make sure to increase amount of work performed; exiting...\n");
			exit(1);
		}
		
		if(mode == 0)
		{
			num_giga_ops = size;
		}
		else if(mode == 1)
		{
			num_giga_ops = (size * size * size) / (GIGABYTES);
		}

		double throughput = num_giga_ops/elapsed_time_sec;
		printf("mode=%s type=%s size=%lld threads=%d time=%lf throughput=%lf\n",argv[1],argv[2],size,num_threads,elapsed_time_sec,throughput); // Display benchmark results.
 
    }

    return 0;
}
