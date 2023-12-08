#ifndef _BENCHMARKING_H
#define _BENCHMARKING_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/* Stops the previously set timer, corrects */
/* for overflow on us and returns delay.    */
unsigned long long TimerStop(struct timeval endTime, struct timeval startTime);

/* Function to calculate trimmed average for a number of trials. */
void trimmed_averager(int m, char *name, unsigned long long *trial_arr,
    int trials, float *avg_arr, float *StdDev_arr,
        unsigned long long *trimmed_avg_arr, int *outlier_count_arr, int trim);

#endif /* _BENCHMARKING_H */