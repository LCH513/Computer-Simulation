/* External definitions for single-server queueing system. */
// mean system delays and mean system lengths for the M/M/1

#include <stdio.h>  
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */

#define Q_LIMIT_1 100  /* Limit on queue length. */
#define Q_LIMIT_2 100  /* Limit on queue length. */


#define BUSY      1  /* Mnemonics for server's being busy */
#define IDLE      0  /* and idle. */

int   next_event_type, num_custs_delayed, num_delays_required, num_events,
num_in_q[3], server_status[3], nums[3];
float mean_interarrival, mean_service,
sim_time, time_arrival_1[Q_LIMIT_1 + 1], time_arrival_2[Q_LIMIT_2 + 1], time_last_event, time_next_event[4],
total_of_delays[3], total_system_delays[3];
FILE* infile, * outfile;

void  initialize(void);
void  timing(void);
void  arrive(int node);
void  depart(int node);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);


main()  /* Main function. */
{
	/* Open input and output files. */
	infile = fopen("mm1_0.75.in", "r");
	outfile = fopen("mm1_0.75.out", "w");

	/* Specify the number of events for the timing function. */

	num_events = 3;

	/* Read input parameters. */

	fscanf(infile, "%f %f %d", &mean_interarrival, &mean_service, &num_delays_required);

	/* Write report heading and input parameters. */

	fprintf(outfile, "Single-server queueing system\n\n");
	fprintf(outfile, "Mean interarrival time%11.3f minutes\n\n", mean_interarrival);
	fprintf(outfile, "Mean service time%16.3f minutes\n\n", mean_service);
	fprintf(outfile, "Number of customers%14d\n\n", num_delays_required);

	/* Initialize the simulation. */

	initialize();

	/* Run the simulation while more delays are still needed. */

	while (num_custs_delayed < num_delays_required)
	{
		/* Determine the next event. */

		timing();

		/* Update time-average statistical accumulators. */

		update_time_avg_stats();

		/* Invoke the appropriate event function. */

		switch (next_event_type)
		{
		case 1:
			arrive(1);
			break;
		case 2:
			depart(1);
			arrive(2);
			break;
		case 3:
			depart(2);
			break;
		}
	}

	/* Invoke the report generator and end the simulation. */

	report();

	fclose(infile);
	fclose(outfile);

	return 0;
}


void initialize(void)  /* Initialization function. */
{
	/* Initialize the simulation clock. */

	sim_time = 0.0;

	/* Initialize the state variables. */


	time_last_event = 0.0;

	/* Initialize the statistical counters. */

	num_custs_delayed = 0;
	for (int i = 0; i < 3; i++)
	{
		num_in_q[i] = 0;
		server_status[i] = IDLE;
		total_of_delays[i] = 0.0;
		total_system_delays[i] = 0.0;
		nums[i] = 0;
	}

	/* Initialize event list.  Since no customers are present, the departure
	   (service completion) event is eliminated from consideration. */

	time_next_event[1] = sim_time + expon(mean_interarrival);
	time_next_event[2] = 1.0e+30;
	time_next_event[3] = 1.0e+30;
}


void timing(void)  /* Timing function. */
{
	int   i;
	float min_time_next_event = 1.0e+29;

	next_event_type = 0;

	/* Determine the event type of the next event to occur. */

	for (i = 1; i <= num_events; ++i)
		if (time_next_event[i] < min_time_next_event)
		{
			min_time_next_event = time_next_event[i];
			next_event_type = i;
		}

	/* Check to see whether the event list is empty. */

	if (next_event_type == 0)
	{
		/* The event list is empty, so stop the simulation. */

		fprintf(outfile, "\nEvent list empty at time %f", sim_time);
		exit(1);
	}

	/* The event list is not empty, so advance the simulation clock. */

	sim_time = min_time_next_event;
}


void arrive(int node)  /* Arrival event function. */
{
	float delay;

	/* Check to see whether server is busy. */

	if (server_status[node] == BUSY)
	{
		/* Server is busy, so increment number of customers in queue. */

		++num_in_q[node];

		/* Check to see whether an overflow condition exists. */

		switch (node)
		{
		case 1:
			time_next_event[1] = sim_time + expon(mean_interarrival);
			if (num_in_q[node] > Q_LIMIT_1)
			{
				/* The queue has overflowed, so stop the simulation. */
				--num_in_q[node];
				break;
				//fprintf(outfile, "\nOverflow of the array time_arrival at");
				//fprintf(outfile, " time %f", sim_time);
				//exit(2);
			}
			time_arrival_1[num_in_q[node]] = sim_time;
			break;
		case 2:
			if (num_in_q[node] > Q_LIMIT_2)
			{
				/* The queue has overflowed, so stop the simulation. */
				--num_in_q[node];
				break;
				//fprintf(outfile, "\nOverflow of the array time_arrival at");
				//fprintf(outfile, " time %f", sim_time);
				//exit(2);
			}
			time_arrival_2[num_in_q[node]] = sim_time;
			break;
		}
	}

	else
	{
		/* Server is idle, so arriving customer has a delay of zero.  (The
		   following two statements are for program clarity and do not affect
		   the results of the simulation.) */
		switch (node)
		{
		case 1:
			time_next_event[1] = sim_time + expon(mean_interarrival);
			delay = 0.0;
			total_of_delays[node] += delay;
			total_system_delays[node] += delay;

			/* Increment the number of customers delayed, and make server busy. */

			++num_custs_delayed;
			server_status[node] = BUSY;

			/* Schedule a departure (service completion). */

			time_next_event[2] = sim_time + expon(mean_service);
			total_system_delays[node] += time_next_event[2] - sim_time;
			break;
		case 2:
			//time_next_event[2] = sim_time + expon(mean_interarrival);
			delay = 0.0;
			total_of_delays[node] += delay;
			total_system_delays[node] += delay;

			/* Increment the number of customers delayed, and make server busy. */

			//++num_custs_delayed;
			server_status[node] = BUSY;

			/* Schedule a departure (service completion). */

			time_next_event[3] = sim_time + expon(mean_service);
			total_system_delays[node] += time_next_event[3] - sim_time;
			break;
		}
		++nums[node];
	}
}


void depart(int node)  /* Departure event function. */
{
	int   i;
	float delay;

	/* Check to see whether the queue is empty. */

	if (num_in_q[node] == 0)
	{
		/* The queue is empty so make the server idle and eliminate the
		   departure (service completion) event from consideration. */

		server_status[node] = IDLE;
		switch (node)
		{
		case 1:
			time_next_event[2] = 1.0e+30;
			break;
		case 2:
			time_next_event[3] = 1.0e+30;
			break;
		}
	}

	else
	{
		--num_in_q[node];
		switch (node)
		{
		case 1:
			delay = sim_time - time_arrival_1[1];
			total_of_delays[node] += delay;
			++num_custs_delayed;
			time_next_event[2] = sim_time + expon(mean_service);
			total_system_delays[node] += time_next_event[2] - sim_time;
			for (i = 1; i <= num_in_q[node]; ++i)
				time_arrival_1[i] = time_arrival_1[i + 1];
			break;
		case 2:
			delay = sim_time - time_arrival_2[1];
			total_of_delays[node] += delay;
			//++num_custs_delayed;
			time_next_event[3] = sim_time + expon(mean_service);
			total_system_delays[node] += time_next_event[3] - sim_time;
			for (i = 1; i <= num_in_q[node]; ++i)
				time_arrival_2[i] = time_arrival_2[i + 1];
			break;
		}
		++nums[node];
	}
}


void report(void)  /* Report generator function. */
{
	/* Compute and write estimates of desired measures of performance. */

	fprintf(outfile, "\n\nAverage delay in queue%11.3f minutes\n\n",
		total_of_delays[1] / num_custs_delayed);


	fprintf(outfile, "Time simulation ended%12.3f minutes\n\n", sim_time);

	printf("%d", nums[1]);
	fprintf(outfile, "Mean system delays %11.3f minutes\n\n",
		(total_of_delays[1] + total_system_delays[1] + total_of_delays[2] + total_system_delays[2]) / (nums[1]));
	fprintf(outfile, "Mean system delays 1%11.3f minutes\n\n",
		total_of_delays[1] / nums[1] + total_system_delays[1] / nums[1]);
	fprintf(outfile, "Mean system delays 2%11.3f minutes\n\n",
		total_of_delays[2] / nums[2] + total_system_delays[2] / nums[2]);
}


void update_time_avg_stats(void)  /* Update area accumulators for time-average
									 statistics. */
{
	float time_since_last_event;

	/* Compute time since last event, and update last-event-time marker. */

	time_since_last_event = sim_time - time_last_event;
	time_last_event = sim_time;
}


float expon(float mean)  /* Exponential variate generation function. */
{
	/* Return an exponential random variate with mean "mean". */

	return -mean * log(lcgrand(1));
}