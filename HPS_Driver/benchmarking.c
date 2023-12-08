#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/* Stops the previously set timer, corrects */
/* for overflow on us and returns delay.    */
unsigned long long TimerStop(struct timeval endTime, struct timeval startTime)
{
	gettimeofday(&endTime, NULL);
	// Corrects potential .tv_usec overflow
	if (endTime.tv_usec < startTime.tv_usec)
	{
		endTime.tv_sec--;
		endTime.tv_usec += 1000000;
	}
	// returns the difference between start and end times
	return (unsigned long long)endTime.tv_sec
    - (unsigned long long)startTime.tv_sec
    + (unsigned long long)endTime.tv_usec
    - (unsigned long long)startTime.tv_usec;
}

/* Function to calculate trimmed average for a number of trials. */
void trimmed_averager(int m, char *name, unsigned long long *trial_arr,
    int trials, float *avg_arr, float *StdDev_arr,
        unsigned long long *trimmed_avg_arr, int *outlier_count_arr, int trim)
{
	int i = 0;
    float avg = 0;
    // Accumulates results of trials onto an average
    for(i = 0; i < trials; i++)
        avg += (float)trial_arr[i];
    // Computes the average
    avg = avg/trials;
    avg_arr[m] = avg;
    printf("%s average: %f\n", name, avg);
	if(trim)
    {
		float StdDev = 0;
		// Computes Standard Deviation
		for(i = 0; i < trials; i++)
			StdDev += pow((float)trial_arr[i] - (float)avg, 2);
		StdDev = sqrt(StdDev/trials);
		printf("%s standard deviation: %f\n", name, StdDev);
		// Stores Standard Deviation within global array
		StdDev_arr[m] = StdDev;
		// Computes Margin to be trimmed away
		StdDev = StdDev*3;
		int trim_count = 0;
		int outlier_count = 0;
		unsigned long long outlier_arr[trials];
		unsigned long long trimmed_arr[trials];
		// Sorts outliers out of dataset
		for(i = 0; i < trials; i++)
		{
			if((trial_arr[i] < StdDev + avg) && (trial_arr[i] > StdDev - avg))
			{
				trimmed_arr[trim_count] = trial_arr[i];
				trim_count++;
			}
			else
			{
				outlier_arr[outlier_count] = trial_arr[i];;
				outlier_count++;
			}
		}
		float trimmed_avg = 0;
		// Computes Trimmed Average
		for(i = 0; i < trim_count; i++)
			trimmed_avg += trimmed_arr[i];
		trimmed_avg = trimmed_avg/trim_count;
		printf("%s trimmed average: %f\n", name, trimmed_avg);
		trimmed_avg_arr[m] = trimmed_avg;
		// Prints Outliers
		if(outlier_arr[0])
		{
			printf("%s outliers:\n", name);
			for(i = 0; i < outlier_count; i++)
				printf("%llu\n", outlier_arr[i]);
		}
		// Stores Outlier Amount
		outlier_count_arr[m] = outlier_count;

		memset(outlier_arr, 0, sizeof outlier_arr);
	}
	memset(trial_arr, 0, sizeof trial_arr);
}