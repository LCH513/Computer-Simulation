/* External definitions for single-server queueing system. */

#include <stdio.h>
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */

#define BUSY      1  /* Mnemonics for server's being busy */
#define IDLE      0  /* and idle. */

int   next_event_type, num_custs_delayed, num_delays_required, num_events,
num_in_q, c, num_A, num_B, num_C, num_D;
float area_num_in_q, area_server_status, mean_interarrival, mean_service1, mean_service2, mean_service3, mean_service4,
sim_time, time_last_event, time_next_event[6];

float total_of_delays, lo, mu, lambda, W, Wq, L, Lq, mean_service_total, service_time_exp;
float total_of_delays_A, total_of_delays_B, mean_service_total_A, mean_service_total_B;
float total_of_delays_C, total_of_delays_D, mean_service_total_C, mean_service_total_D;
int   server_status[5];
FILE* infile, * outfile;

typedef struct node_structure
{
	float data;
	struct node_structure* next;
} node;

void  initialize(void);
void  timing(void);
void  arrive(int level);
void  depart(int level);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);
void insert_node(node* n1, node* n2);
void remove_node(node* n1);
node* create_node(float data);
node* queue_list, * temp_node, * queue_list2, * temp_node2, * queue_list3, * temp_node3, * queue_list4, * temp_node4;

main()  /* Main function. */
{
	/* Open input and output files. */
	infile = fopen("mm1_0.9.in", "r");
	outfile = fopen("mm1_0.9.out", "w");

	/* Specify the number of events for the timing function. */


	num_events = 5;

	/* Read input parameters. */

	fscanf(infile, "%f %f %f %f %f %d", &mean_interarrival, &mean_service1, &mean_service2, &mean_service3, &mean_service4,
		&num_delays_required);

	/* Write report heading and input parameters. */

	fprintf(outfile, "Single-server queueing system\n\n");
	fprintf(outfile, "Mean interarrival time %11.3f minutes\n\n",
		mean_interarrival);
	fprintf(outfile, "Mean service time %16.3f minutes\n\n", mean_service1);
	fprintf(outfile, "Mean service time %16.3f minutes\n\n", mean_service2);
	fprintf(outfile, "Mean service time %16.3f minutes\n\n", mean_service3);
	fprintf(outfile, "Mean service time %16.3f minutes\n\n", mean_service4);
	fprintf(outfile, "Number of customers %14d\n\n", num_delays_required);

	/* Initialize the simulation. */

	initialize();

	/* Run the simulation while more delays are still needed. */

	while (num_D < num_delays_required) {

		/* Determine the next event. */

		timing();

		/* Update time-average statistical accumulators. */

		update_time_avg_stats();

		/* Invoke the appropriate event function. */

		switch (next_event_type) {
		case 1:
			arrive(1);
			break;
		case 2:
			depart(1);
			arrive(2);
			break;
		case 3:
			depart(2);
			arrive(3);
			break;
		case 4:
			depart(3);
			arrive(4);
			break;
		case 5:
			depart(4);
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

	server_status[1] = IDLE;
	server_status[2] = IDLE;
	server_status[3] = IDLE;
	server_status[4] = IDLE;
	num_in_q = 0;
	time_last_event = 0.0;

	/* Initialize the statistical counters. */

	num_custs_delayed = 0;
	total_of_delays = 0.0;
	area_num_in_q = 0.0;
	area_server_status = 0.0;

	/* Initialize event list.  Since no customers are present, the departure
	   (service completion) event is eliminated from consideration. */

	time_next_event[1] = sim_time + expon(mean_interarrival);
	time_next_event[2] = 1.0e+30;
	time_next_event[3] = 1.0e+30;
	time_next_event[4] = 1.0e+30;
	time_next_event[5] = 1.0e+30;

	/* M/M/1 */
	mean_service_total = 0;
	c = 1;
	service_time_exp = 0;
	queue_list = create_node(0);
	temp_node = queue_list;
	queue_list2 = create_node(0);
	temp_node2 = queue_list2;
	queue_list3 = create_node(0);
	temp_node3 = queue_list3;
	queue_list4 = create_node(0);
	temp_node4 = queue_list4;
	num_A = 0;
	num_B = 0;
	num_C = 0;
	num_D = 0;
	total_of_delays_A = 0;
	total_of_delays_B = 0;
	total_of_delays_C = 0;
	total_of_delays_D = 0;
}


void timing(void)  /* Timing function. */
{
	int   i;
	float min_time_next_event = 1.0e+29;

	next_event_type = 0;

	/* Determine the event type of the next event to occur. */

	for (i = 1; i <= num_events; ++i)
		if (time_next_event[i] < min_time_next_event) {
			min_time_next_event = time_next_event[i];
			next_event_type = i;
		}

	/* Check to see whether the event list is empty. */

	if (next_event_type == 0) {

		/* The event list is empty, so stop the simulation. */

		fprintf(outfile, "\nEvent list empty at time %f", sim_time);
		exit(1);
	}

	/* The event list is not empty, so advance the simulation clock. */

	sim_time = min_time_next_event;
}


void arrive(int level)  /* Arrival event function. */
{
	float delay;

	/* Check to see whether server is busy. */
	if (server_status[level] == BUSY) {
		switch (level) {
		case 1:
			time_next_event[1] = sim_time + expon(mean_interarrival);
			insert_node(temp_node, create_node(sim_time));
			temp_node = temp_node->next;
			break;
		case 2:
			insert_node(temp_node2, create_node(sim_time));
			temp_node2 = temp_node2->next;
			break;
		case 3:
			insert_node(temp_node3, create_node(sim_time));
			temp_node3 = temp_node3->next;
			break;
		case 4:
			insert_node(temp_node4, create_node(sim_time));
			temp_node4 = temp_node4->next;
			break;
		}

	}

	else {

		/* Server is idle, so arriving customer has a delay of zero.  (The
		   following two statements are for program clarity and do not affect
		   the results of the simulation.) */

		delay = 0.0;
		total_of_delays += delay;
		++num_custs_delayed;

		switch (level) {
		case 1:
			time_next_event[1] = sim_time + expon(mean_interarrival);
			++num_A;
			total_of_delays_A += delay;
			server_status[level] = BUSY;
			service_time_exp = expon(mean_service1);
			time_next_event[2] = sim_time + service_time_exp;
			mean_service_total += service_time_exp;
			mean_service_total_A += service_time_exp;
			break;
		case 2:
			++num_B;
			total_of_delays_B += delay;
			server_status[level] = BUSY;
			service_time_exp = expon(mean_service2);
			time_next_event[3] = sim_time + service_time_exp;
			mean_service_total += service_time_exp;
			mean_service_total_B += service_time_exp;
			break;
		case 3:
			++num_C;
			total_of_delays_C += delay;
			server_status[level] = BUSY;
			service_time_exp = expon(mean_service3);
			time_next_event[4] = sim_time + service_time_exp;
			mean_service_total += service_time_exp;
			mean_service_total_C += service_time_exp;
			break;
		case 4:
			++num_D;
			total_of_delays_D += delay;
			server_status[level] = BUSY;
			service_time_exp = expon(mean_service4);
			time_next_event[5] = sim_time + service_time_exp;
			mean_service_total += service_time_exp;
			mean_service_total_D += service_time_exp;
			break;
		}


	}
}


void depart(int level)  /* Departure event function. */
{
	int   i;
	float delay;

	/* Check to see whether the queue is empty. */

	switch (level) {
	case 1:
		if (queue_list->next == NULL) {
			server_status[level] = IDLE;
			time_next_event[2] = 1.0e+30;
		}

		else {
			delay = sim_time - queue_list->next->data;
			total_of_delays += delay;
			total_of_delays_A += delay;

			++num_custs_delayed;
			++num_A;
			service_time_exp = expon(mean_service1);
			time_next_event[2] = sim_time + service_time_exp;
			mean_service_total += service_time_exp;
			mean_service_total_A += service_time_exp;
			/* Move each customer in queue (if any) up one place. */

			remove_node(queue_list);
			if (queue_list->next == NULL) {
				temp_node = queue_list;
			}
		}
		break;
	case 2:
		if (queue_list2->next == NULL) {
			server_status[level] = IDLE;
			time_next_event[3] = 1.0e+30;
		}

		else {
			delay = sim_time - queue_list2->next->data;
			total_of_delays += delay;
			total_of_delays_B += delay;

			++num_custs_delayed;
			++num_B;
			service_time_exp = expon(mean_service2);
			time_next_event[3] = sim_time + service_time_exp;
			mean_service_total += service_time_exp;
			mean_service_total_B += service_time_exp;
			/* Move each customer in queue (if any) up one place. */

			remove_node(queue_list2);
			if (queue_list2->next == NULL) {
				temp_node2 = queue_list2;
			}
		}
		break;
	case 3:
		if (queue_list3->next == NULL) {
			server_status[level] = IDLE;
			time_next_event[4] = 1.0e+30;
		}

		else {
			delay = sim_time - queue_list3->next->data;
			total_of_delays += delay;
			total_of_delays_C += delay;

			++num_custs_delayed;
			++num_C;
			service_time_exp = expon(mean_service3);
			time_next_event[4] = sim_time + service_time_exp;
			mean_service_total += service_time_exp;
			mean_service_total_C += service_time_exp;
			/* Move each customer in queue (if any) up one place. */

			remove_node(queue_list3);
			if (queue_list3->next == NULL) {
				temp_node3 = queue_list3;
			}
		}
		break;
	case 4:
		if (queue_list4->next == NULL) {
			server_status[level] = IDLE;
			time_next_event[5] = 1.0e+30;
		}

		else {
			delay = sim_time - queue_list4->next->data;
			total_of_delays += delay;
			total_of_delays_D += delay;

			++num_custs_delayed;
			++num_D;
			service_time_exp = expon(mean_service4);
			time_next_event[5] = sim_time + service_time_exp;
			mean_service_total += service_time_exp;
			mean_service_total_D += service_time_exp;
			/* Move each customer in queue (if any) up one place. */

			remove_node(queue_list4);
			if (queue_list4->next == NULL) {
				temp_node4 = queue_list4;
			}
		}
		break;
	}

}


void report(void)  /* Report generator function. */
{
	/* Compute and write estimates of desired measures of performance. */

	/* W. */
	fprintf(outfile, "\nMean system delay Node 1 %15.4f\n\n\n",
		total_of_delays_A / num_A);
	fprintf(outfile, "\nMean system delay Node 2 %15.4f\n\n\n",
		total_of_delays_B / num_B);
	fprintf(outfile, "\nMean system delay Node 3 %15.4f\n\n\n",
		total_of_delays_C / num_C);
	fprintf(outfile, "\nMean system delay Node 4 %15.4f\n\n\n",
		total_of_delays_D / num_D);

	fprintf(outfile, "Time simulation ended %12.3f minutes", sim_time);
}


void update_time_avg_stats(void)  /* Update area accumulators for time-average
									 statistics. */
{
	float time_since_last_event;

	/* Compute time since last event, and update last-event-time marker. */

	time_since_last_event = sim_time - time_last_event;
	time_last_event = sim_time;

	/* Update area under number-in-queue function. */

	area_num_in_q += num_in_q * time_since_last_event;

	/* Update area under server-busy indicator function. */
	int i;
	for (i = 1; i < 5; i++) {
		area_server_status += server_status[i] * time_since_last_event;
	}
}


float expon(float mean)  /* Exponential variate generation function. */
{
	/* Return an exponential random variate with mean "mean". */

	return -mean * log(lcgrand(1));
}

node* create_node(float data)
{
	node* new_node = (node*)malloc(sizeof(node));

	new_node->data = data;
	new_node->next = NULL;

	return new_node;
}

void insert_node(node* n1, node* n2)
{
	n2->next = n1->next;
	n1->next = n2;
}

void remove_node(node* n1)
{
	n1->next = n1->next->next;
}

void print_lists(node* lists)
{
	node* n = lists;

	while (n != NULL)
	{
		printf("%.3f ", n->data);

		n = n->next;
	}

	printf("\n");
}