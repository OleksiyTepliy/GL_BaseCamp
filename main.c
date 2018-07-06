#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "det.h"

/**
 * timespec_diff() - returns time difference in milliseconds for two timespecs.
 * @stop:	time after event.
 * @start:	time before event.
 *
 * Uses difftime() for time_t seconds calcultation.
 */
double timespec_diff(struct timespec *stop, struct timespec *start)
{
	double diff = difftime(stop->tv_sec, start->tv_sec);
	diff *= 1000;
	diff += (stop->tv_nsec - start->tv_nsec) / 1e6;
	return diff;
}

                
                /* CHOSE THE TYPE OF TEST WILL BE EXECUTED, UNCOMMENT RESPECTIVE LINE */
                
#define COMMAND_LINE_ARGUMETS_TEST
//#define RANDOM_TEST

int main(int argc, char const *argv[]){

#ifdef COMMAND_LINE_ARGUMETS_TEST
    /* Parse arguments */
    if (argc == 1){
        printf("Error: Provide elements of matrix in linear form\n\
        Usage: ./main m[0][0]...m[n][n]\n");
        return 1;
    }
    double q = sqrt((double) (argc - 1));
    if(fmod(q, 1.00)){
        printf("Error: quantity of elements should be sqare number\n");
        return 1;
    }
    
    /* Initialize matrix with command line arguments */
    int n = (int) q;
    printf("Matrix %dx%d\n", n, n);
    int indx = 1;  // argv[indx]
    int mat[n][n];
   
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            mat[i][j] = atoi(argv[indx++]);
            printf("%4d ", mat[i][j]);
        }
        printf("\n");
    }
#endif

#ifdef RANDOM_TEST
    /* no error checkings, just speed test of calculations*/
    printf("enter Matrix size, >= 1 : ");
    int n;  
    scanf("%d", &n);
    printf("\nMatrix %d x %d\n", n, n);
    int mat[n][n];
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            mat[i][j] = rand() % 10;
            printf("%4d ", mat[i][j]);
        }
        printf("\n");
    }
#endif
    struct timespec time_now, time_after;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_now);
	printf("DET = %d\n", det(n, mat));
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_after);

    /* How much time took calculations */
    printf("Finding DET of Matrix %d x %d took %g ms\n", 
	    n, n, timespec_diff(&time_after, &time_now));

    return 0;
}
