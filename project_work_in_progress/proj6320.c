#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define QUEUE_CAPACITY 10
#define MAX_PACKETS 10000

#define max(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

typedef enum event_type
{
	ARRIVAL,
	DEPARTURE_FROM_QUEUE1,
	DEPARTURE_FROM_QUEUE2,
    DONE
} event_type;

typedef enum queue_selection
{
	UNIFORMLY_RANDOM,
	MIN_LENGTH_QUEUE
} queue_selection;

typedef struct packet
{
	double serv_t;
	double wait_t;
	struct packet *next;
} packet;

typedef struct queue
{
	int length;
	double front_serv_t; // Remaining service time for the serviced packet
	packet *front;
} queue;

typedef struct stats
{
	double avg_queue_length;
	double avg_wait_t;
	double blocked_rate;
} stats;

double expntl(double);        // Generate exponential RV with mean x
stats* simulateQ(double, double, queue_selection);
stats* theoretical(double, double, int);
void acceptPacket(queue*, packet*);

int main()
{
	double lambda = 5.0;
	double mu = 4.0;
	int k = 10, i = 0;
    double avg_queue_length = 0.0, avg_wait_t = 0.0, blocked_rate = 0.0;
	srand(time(0)); // time dependent random seed: comment out for fixed seed

	stats *s;
    while (i < k)
    {
        srand((long)(time(0)*pow(.7,i)));
        s = simulateQ(lambda, mu, UNIFORMLY_RANDOM);
        avg_queue_length += s->avg_queue_length;
        avg_wait_t += s-> avg_wait_t;
        blocked_rate += s-> blocked_rate;

        free(s);
        i++;
    }

    avg_queue_length /= k;
    avg_wait_t /= k;
    blocked_rate /= k;

    printf("stats avg q leng = %f, avg wait time = %f, block rate = %f\n", avg_queue_length, avg_wait_t, blocked_rate);

    stats *s2 = theoretical(lambda, mu, 10);
    printf("theoretical stats avg q leng = %f, avg wait time = %f, block rate = %f\n", s2->avg_queue_length, s2->avg_wait_t, s2->blocked_rate);
    /**
     *  Theoritical performance when lambda = 3.9 / 2, mu = 2.0 (uniformly random queueing)
     *  pi0 = 0.1028 (idle rate)
     *  pi10 = 0.0798 (blocking rate)
     * 
     *  E(B) = 1 - pi0 =  0.8972
     *  E(L) = pi1 + 2*pi2 + ... + 10*pi10 = 4.7476
     *  E(Lq) = E(L) - E(B) = 3.8504
     *  E(W) = E(L) / (lambda) = 1.9746
     */

    return 0;
}

// Selection strategy: 0 = uniformly random, 1 = min length queue
stats* simulateQ(double lambda, double mu, queue_selection selection_strategy)
{
	if (lambda == 0 || mu == 0)
	{
		perror("lambda or mu cannot be zero!\n");
		return NULL;
	}
    if (lambda/(2*mu) >= 1.0)
    {
        perror("lambda/2mu must be less than 1!\n");
        return NULL;
    }

	double arrival_t_mean = 1 / lambda;
	double serv_t_mean = 1 / mu;

	double interarrival_ts[MAX_PACKETS];
	int i = 0;

    // for stat calculation
    int blocked = 0;
    int total_length = 0;
    double total_wait_t = 0.0;
    int accepted = 0;

	// Set up two queues
	queue *q1 = (queue *)malloc(sizeof(queue));
	queue *q2 = (queue *)malloc(sizeof(queue));

	q1->length = 0;
	q1->front_serv_t = INFINITY;
	q1->front = NULL;
	q2->length = 0;
	q2->front_serv_t = INFINITY;
	q2->front = NULL;

	// Pre-generate interarrival times of packets
	while (i < MAX_PACKETS)
	{
		interarrival_ts[i] = expntl(arrival_t_mean);
        i++;
	}

	double next_arrival = interarrival_ts[0];
	event_type next_event = ARRIVAL;
	i = 1;

    int infloop = 0;

	// Loop while there are packets remaining to either arrive or depart
    while (next_event != DONE)
	{
        infloop++;
        //printf("loop counter: %d", infloop);
        if (infloop > (2*MAX_PACKETS + 10))
        {
            printf("Inf loop while simulating\n");
            return NULL;
        }

		if (next_event == ARRIVAL)
		{
            // Tick time
			double time_elapsed = next_arrival;
			if (q1->length > 0)
			{
				q1->front_serv_t -= time_elapsed;
			}
			if (q2->length > 0)
			{
				q2->front_serv_t -= time_elapsed;
			}

            // set up a packet
            packet *newPacket = (packet *)malloc(sizeof(packet));
            newPacket->next = NULL;
            newPacket->serv_t = expntl(serv_t_mean);
            newPacket->wait_t = 0.0;

            if (selection_strategy == MIN_LENGTH_QUEUE && q1->length != q2->length)
            {
                if (q1->length >= 10 && q2->length >= 10) printf("min length q insertion when full capacity!\n");
                if (q1->length < q2->length)
                {
                    total_length += max(0, q1->length - 1); // exclude servicing packet
                    acceptPacket(q1, newPacket);
                    accepted++;
                }
                else
                {
                    total_length += max(0, q2->length - 1); // exclude servicing packet
                    acceptPacket(q2, newPacket);
                    accepted++;
                }
            }
            else // (selection_strategy == UNIFORMLY_RANDOM) or two queues have same length
            {
                if ((rand() % 2) == 0)
                {
                    if (q1->length < 10)
                    {
                        total_length += max(0, q1->length - 1); // exclude servicing packet
                        acceptPacket(q1, newPacket);
                        accepted++;
                    } 
                    else 
                    {
                        blocked++;
                        free(newPacket);
                    }
                } 
                else
                {
                    if (q2->length < 10)
                    {
                        total_length += max(0, q2->length - 1); // exclude servicing packet
                        acceptPacket(q2, newPacket);
                        accepted++;
                    }
                    else
                    {
                        blocked++;
                        free(newPacket);
                    }
                }
            }

            // set timer for the next packet arrival
            if (i < MAX_PACKETS) next_arrival = interarrival_ts[i];
            else next_arrival = INFINITY;
			i++;
		}
		else if (next_event == DEPARTURE_FROM_QUEUE1)
		{
            // Tick time
            double time_elapsed = q1->front_serv_t;
            if (i < MAX_PACKETS) next_arrival -= time_elapsed;
            if (q2->length > 0) q2->front_serv_t -= time_elapsed;

            // stat calculation
            total_wait_t += q1->front->wait_t;
            
            // remove a serviced packet from queue
            packet *departing = q1->front;
            q1->length -= 1;
            q1->front = q1->front->next;
            if (q1->length > 0) q1->front_serv_t = q1->front->serv_t;
            else q1->front_serv_t = INFINITY;

            free(departing);
		}
		else if (next_event == DEPARTURE_FROM_QUEUE2)
		{
            // Tick time
            double time_elapsed = q2->front_serv_t;
            if (i < MAX_PACKETS) next_arrival -= time_elapsed;
            if (q1->length > 0) q1->front_serv_t -= time_elapsed;

            // stat calculation
            total_wait_t += q2->front->wait_t;

            // remove a serviced packet from queue
            packet *departing = q2->front;
            q2->length -= 1;
            q2->front = q2->front->next;
            if (q2->length > 0) q2->front_serv_t = q2->front->serv_t;
            else q2->front_serv_t = INFINITY;

            free(departing);
		}
		else
		{
			perror("Wrong event type encountered while simulating\n");
			return NULL;
		}

        // determine next event
        if (next_arrival < q1->front_serv_t && next_arrival < q2->front_serv_t)
        {
            next_event = ARRIVAL;
        }
        else if (q1->front_serv_t < q2->front_serv_t)
        {
            next_event = DEPARTURE_FROM_QUEUE1;
        }
        else if (q2->length > 0)
        {
            next_event = DEPARTURE_FROM_QUEUE2;
        }
        else next_event = DONE;
	}

	stats *s = (stats *)malloc(sizeof(stats));
	s->avg_queue_length = (1.0*total_length) / (MAX_PACKETS - blocked);
	s->blocked_rate = (1.0*blocked) / MAX_PACKETS;
	s->avg_wait_t = total_wait_t / (MAX_PACKETS - blocked);
    //printf("loop counter: %i, total blocked: %d, total length: %d, total wait: %f\n", infloop, blocked, total_length, total_wait_t);
    //printf("  accepted: %d\n", accepted);

    return s;
}

void acceptPacket(queue* q, packet* p)
{
	if (q->length == 0)
	{
		q->length += 1;
		q->front = p;
		q->front_serv_t = p->serv_t;
		
		return;
	}
	packet* line = q->front;
	double wait_t = q->front_serv_t;

	while (line->next != NULL)
	{
		line = line->next;
		wait_t += line->serv_t;
	}

	q->length += 1;
	line->next = p;
	p->wait_t = wait_t;
}

// calculate theoretical performance of random select two queues with same service rate
stats* theoretical(double lambda, double mu, int k)
{
    double adjLambda = lambda/2;
	stats *s = (stats *)malloc(sizeof(stats));

    if (adjLambda > mu)
    {
        s->avg_queue_length = k-1;
        s->blocked_rate = 1.0;
        s->avg_wait_t = (k-1)/mu;

        return s;
    }

    double rho = adjLambda / mu;
    double pi0 = 1.0, pi10 = 0.0;
    double B, L, Lq, W, br;
    int i = 0;

    while (i < k)
    {
        pi0 += pow(rho, i+1);
        i++;
    }

    pi0 = 1/pi0;
    B = 1 - pi0;
    pi10 = pow(rho, k) * pi0;

    i = 0;
    L = 0.0;
    while (i < k)
    {
        L += (i+1)*pow(rho, i+1)*pi0;
        i++;
    }

    Lq = L - B;         // Avg queue length
    W = Lq / adjLambda; // Avg wait time
    br = pi10;      // Blocking rate

	s->avg_queue_length = Lq;
	s->blocked_rate = br;
	s->avg_wait_t = W;

    return s;
}

//==============================================================================
//=  Function to generate exponentially distributed RVs using inverse method   =
//=    - Input:  x (mean value of distribution)                                =
//=    - Output: Returns with exponential RV                                   =
//==============================================================================
double expntl(double x)
{
	double z; // Uniform random number from 0 to 1

	// Pull a uniform RV (0 < z < 1)
	do
	{
		z = ((double)rand() / RAND_MAX);
	} while ((z == 0) || (z == 1));

	return(-x * log(z));
}