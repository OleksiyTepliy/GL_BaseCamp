#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sched.h>
#include <pthread.h>


static const char help[] = {
  "[-h] [-v] -t NUM_THREADS -n ARRAY_SIZE\n"
  "Evaluate the time required to do a simple threaded map-reduce operation"
  "on randomly generated array of doubles"
};

static cpu_set_t all_cores(void)
{
	cpu_set_t cpuset;
	pthread_t this = pthread_self();
	pthread_getaffinity_np(this, sizeof(cpu_set_t), &cpuset);
	int numcores = sysconf(_SC_NPROCESSORS_ONLN);
	for (int id = 0; id < numcores; id++) {
		CPU_SET(id, &cpuset);
	}
	pthread_setaffinity_np(this, sizeof(cpu_set_t), &cpuset);
	return cpuset;
}

/**
 * timespec_diff() - returns time difference in milliseconds for two timespecs.
 * @stop:	time after event.
 * @start:	time before event.
 *
 * Uses difftime() for time_t seconds calcultation.
 */
static double timespec_diff(struct timespec *stop, struct timespec *start)
{
	double diff = difftime(stop->tv_sec, start->tv_sec);
	diff *= 1000;
	diff += (stop->tv_nsec - start->tv_nsec) / 1e6;
	return diff;
}

/* Error-related stuff */
enum _errors {
	E_OK = 0,
	E_FOPEN,
	E_FREAD,
	E_ALLOC,
	E_CPUSET
};

static const char * const _error_msg[] = {
	[E_OK] = "Success",
	[E_FOPEN] = "Failed to open '/dev/random'",
	[E_FREAD] = "Failed to read from '/dev/random'",
	[E_ALLOC] = "Failed to allocate memory",
	[E_CPUSET] = "Could not link thread to all CPU cores"
};

struct thread_data {
	struct timespec start_time, end_time;
	double *arrptr;		/* Points to start of array slice */
	long long num_items;	/* Elements in slice */
	double *resptr;		/* Pointer to result(shared) */
	pthread_mutex_t *lock;	/* Lock for result */
};

void *threadfunc(void *args)
{
	/* Struct is passed via args at pthread_create so its type is known */
	struct thread_data *data = args;
	double *array = data->arrptr;	/* arrptr is a slice of original array */
	long long len = data->num_items;

	/* We check the time spent in each thread and the global time */
	clock_gettime(CLOCK_REALTIME, &data->start_time);

	/* make some calculations */
	double lg_avg = 0;
	for (long long i = 0; i < len; i++) {
		lg_avg += pow (2., log(array[i]) / log(array[i]));
	}

	clock_gettime(CLOCK_REALTIME, &data->end_time);
	pthread_mutex_lock(data->lock); /* wait till acquire */
	/* Now we own a lock */
	*data->resptr += lg_avg;	/* manipulate the shared data */ 
	pthread_mutex_unlock(data->lock);      /* release lock for the others */

	return 0;
}

int main(int argc, char *argv[])
{
	int num_threads = 0;
	long long arr_size = 0;

	/* For more on getopt: http://bit.ly/getopt_rus */
	//opterr = 0; /* No getopt def err out -- we do it manually */
	char argopt;
	/* "ht:n:" means h,t,n switches, t & n require argument */
	while ((argopt = getopt(argc, argv, "ht:n:")) != -1) {
		switch(argopt) {
		case 'h':
			printf("Usage: %s %s\n", argv[0], help);
			exit(0);
		case 't':
			num_threads = atoi(optarg);
			break;
		case 'n':
			arr_size = atoll(optarg);
			break;
		default:
			fprintf(stderr, "Unknown option '%s'\n", optarg);
			exit(EXIT_FAILURE);
		}
	}

	if (argc <= 1) {
		printf("Usage: %s %s\n", argv[0], help);
		exit(0);
	}
	if (num_threads <= 0 || arr_size <= 0) {
		fprintf(stderr, "NUM_THREADS and ARRAY_SIZE aren't ints > 0\n");
		exit(EXIT_FAILURE);
	}
	if (arr_size % num_threads) {
		fprintf(stderr, "NUM_THREADS is not a divisor of ARRAY_SIZE\n");
		exit(EXIT_FAILURE);
	}

	pthread_t threads[num_threads];
	struct thread_data th_dat[num_threads];

	enum _errors errlvl = E_OK;
	/* Fill array with randoms */
	FILE *fp_rand = fopen("/dev/random", "rb");
	if (NULL == fp_rand) {
		errlvl = E_FOPEN;
		goto exc_fopen;
	}
	unsigned int seed;
	fread(&seed, sizeof(seed), 1, fp_rand);
	if (ferror(fp_rand)) {
		errlvl = E_FREAD;
		goto exc_fread;
	}
	srand(seed);

	double *array = malloc(arr_size * sizeof *array);
	if (NULL == array) {
		errlvl = E_ALLOC;
		goto exc_alloc;
	}
		/* Fill Array with random numbers */
	for (long long i = 0; i < arr_size; i++)
		array[i] = (2. / RAND_MAX) * rand();

	/* Configure thread flags */
	pthread_attr_t thread_attrs;
	pthread_attr_init(&thread_attrs); /* fill with default attributes */
	
	// Set scheduler to FIFO for spawned threads
	// This allows for less strict implementation requirements
	pthread_attr_setschedpolicy(&thread_attrs, SCHED_FIFO);
	// Set maximum priority for main and other threads
	// As long as on Linux they compete for overall system resources
	pthread_setschedprio(pthread_self(), sched_get_priority_max(SCHED_FIFO));
	struct sched_param param;
	pthread_attr_getschedparam(&thread_attrs, &param);
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_attr_setschedparam(&thread_attrs, &param);

	cpu_set_t cpuset = all_cores();
	int ret = pthread_attr_setaffinity_np(&thread_attrs, sizeof(cpu_set_t), &cpuset);
	if (ret < 0) {
		errlvl = E_CPUSET;
		goto exc_aff;
	}

		/* mutex init  */
	pthread_mutex_t sharedlock;
	pthread_mutex_init(&sharedlock, NULL);

	double result = 0.;
	struct timespec time_now, time_after;
	
		/* do same calculations in main thread */
	clock_gettime(CLOCK_REALTIME, &time_now);
	double lg_avg = 0;
	for (long long i = 0; i < arr_size; i++) {
		lg_avg += pow (2., log(array[i]) / log(array[i]));
	}
	clock_gettime(CLOCK_REALTIME, &time_after);
	double took_main = timespec_diff(&time_after, &time_now);
	printf("main took: %g ms\nValue (result): %g\n", took_main, lg_avg);
			/* end */

		/* Now spawn threads */
	clock_gettime(CLOCK_REALTIME, &time_now);
	for (int i = 0; i < num_threads; i++) {
		long long slice = arr_size / num_threads;
		th_dat[i].arrptr = &(array[i * slice]);	/* Points to start of array slice */
		th_dat[i].num_items = slice;		/* Elements in slice */
		th_dat[i].resptr = &result;		/* Pointer to result(shared) */
		th_dat[i].lock = &sharedlock;		/* Lock for result */
		pthread_create(&threads[i], &thread_attrs,
                        	&threadfunc, &th_dat[i]);
	}

	for (int i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	clock_gettime(CLOCK_REALTIME, &time_after);

		/* Calculate the resulting times */
	double took_threads = timespec_diff(&time_after, &time_now);
	double took_avg = 0.;
	for (int i = 0; i < num_threads; i++) {
		took_avg += timespec_diff(&(th_dat[i].end_time), 
					  &(th_dat[i].start_time));	
	}
	took_avg /= num_threads;

	printf("Numbers: %lld\nThreads: %d\nValue (result): %g\n"
	       "Average thread time, ms: %g\nCalculation took, ms: %g\n", 
	       arr_size, num_threads, result, took_avg, took_threads);
	
	double eff = 100. * (took_main - took_threads) / took_threads;
	printf("Efficiency increased by %.4g%%\n", eff);
	
	pthread_mutex_destroy(&sharedlock);
	free(array);

		/* goto to Errors section*/
	exc_fopen:
	exc_fread: fclose(fp_rand);
	exc_alloc:
	exc_aff: pthread_attr_destroy(&thread_attrs);

	if (E_OK == errlvl)
		return 0;

	/* Error handler itself */
	fprintf(stderr, "Error: %s\n", _error_msg[errlvl]);
	exit(EXIT_FAILURE);
}