#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <signal.h>

#define GREEN "\e[1;32m"
#define CYAN "\e[1;36m"
#define RED "\e[1;31m"
#define RESET "\033[0m"
#define THREADS 85
#define TASKS 85

typedef struct {
	unsigned int ip_int;
	char subnet[20];
	int start_ip;
	int end_ip;
} thread_data;

pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_cond = PTHREAD_COND_INITIALIZER;
thread_data task_queue[TASKS];
int task_count = 0;
int task_index = 0;

// To use function: int_to_ip(ip_int, ipv4)
void int_to_ip(int ip, char *buffer) {
	sprintf(buffer, "%d.%d.%d.%d",
	(ip >> 24) & 0xFF,
	(ip >> 16) & 0xFF,
	(ip >> 8) & 0xFF,
	ip & 0xFF);
}

int get_cidr(const char *subnet) {
    int cidr = 0;
    sscanf(subnet, "%*[^/]/%d", &cidr);
    return cidr;
}


void *pingScan(void *arg) {
	
	thread_data *data = (thread_data*)arg;
	char command[150];
	
	for (int i = data->start_ip; i <= data->end_ip; i++) {
		char ipv4[20];
		int_to_ip(i, ipv4);
		snprintf(command, sizeof(command), "ping -c 1 -W 1 %s > /dev/null 2>&1 && echo \"" GREEN "[+]" RESET " %s is Up\"", ipv4, ipv4);
		system(command);
	}
	pthread_exit(NULL);
}


void add_task(int start_ip, int end_ip) {
	pthread_mutex_lock(&task_mutex);
	thread_data task;
	
	task.start_ip = start_ip;
	task.end_ip = end_ip;

	// Add task to the queue
	task_queue[task_count++] = task;
	pthread_mutex_unlock(&task_mutex);
}


int main () {
	printf(CYAN "    _       __  __            __           \n" RESET);
	printf(CYAN "   (_)___  / / / /_  ______  / /____  _____\n" RESET);
	printf(CYAN "  / / __ \\/ /_/ / / / / __ \\/ __/ _ \\/ ___/\n" RESET);
	printf(CYAN " / / /_/ / __  / /_/ / / / / /_/  __/ /    \n" RESET);
	printf(CYAN "/_/ .___/_/ /_/\\__,_/_/ /_/\\__/\\___/_/     \n" RESET);
	printf(CYAN " /_/                                       \n\n" RESET);
	printf(RED "Authors: Ryan W. (tiredperson47) and Jerry S. (jsouliss)\n\n\n" RESET);
	
	thread_data data[THREADS];

	//Take user input and send it to the subnet variable in the thread_data structure
	printf("Enter IP address and subnet range (ex. 192.168.1.0/24): ");
	scanf("%19s", data->subnet);
	
	if (strchr(data->subnet, '/') == NULL) {
		fprintf(stderr, RED "[-]" RESET " Invalid subnet format. Expected format: x.x.x.x/xx\n");
		return EXIT_FAILURE;
	}
	
	printf(CYAN "[+]" RESET " Using %s\n", data->subnet);
		
	//Get the cidr notation from the user input. Will be used to determine the ip range. 
	int cidr = get_cidr(data->subnet);
	if (cidr >= 32) {
        	fprintf(stderr, RED "[-]" RESET " Unsuported CIDR from subnet. Limit is /31\n");
        	return EXIT_FAILURE;  // Exit if failure
        }
	
	
	//Changes the subnet from ip/24 to ip
	//ip will be used to set the start ip and end ip. (will figure out how)
	char ip[20];
	size_t length = strlen(data->subnet) - 3;
	strncpy(ip, data->subnet, length);
	ip[length] = '\0';
	
	
	//Set the prefix for the threads to run
	//ip_int is the variable that will be tossed around and used for start IP and end IP
	data->ip_int = ntohl(inet_addr(ip));
	
	//Calculate the total possible IP addresses to scan (will be divided by number of threads: 5)
	int h = (32 - cidr);
	float possible_ip = pow(2, h) - 2;
	
	
	// this is for a /24 subnet. will have to figure out how to implement /16 and possible /8. (or find a way to actually calculate the range)
	int ips_per_task = ceil(possible_ip / THREADS);
	
	for (int i = 0; i < THREADS; i++) {
		int start_ip, end_ip;
		int thread_ip_int = (i * ips_per_task) + 1;
		start_ip = data->ip_int + thread_ip_int; 
		end_ip = start_ip + ips_per_task - 1;
		
		add_task(start_ip, end_ip);
	}

	pthread_t threads[THREADS];
	for (int i = 0; i < THREADS; i++) {
		if (pthread_create(&threads[i], NULL, pingScan, &task_queue[i]) != 0) {
			perror("Failed to create thread");
			return EXIT_FAILURE;
		}
	}

	// Join the threads
	for (int i = 0; i < THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("Ping scan completed.\n");
	return 0; 
}
