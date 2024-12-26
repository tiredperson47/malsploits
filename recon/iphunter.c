#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define CYAN "\033[1;36m"
#define RED "\e[1;31m"
#define RESET "\033[0m"
#define THREADS 5
#define TASKS 85

typedef struct {
	char subnet[20];
	char start_ip[20];
	char end_ip[20];
} thread_data;

pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_cond = PTHREAD_COND_INITIALIZER;
thread_data task_queue[TASKS];
int task_count = 0;
int task_index = 0;

int get_cidr(const char *subnet) {
    int cidr = 0;
    if (sscanf(subnet, "%*[^/]/%d", &cidr) != 1 || cidr != 16 && cidr != 24) {
        fprintf(stderr, RED "[-]" RESET " Unsuported CIDR from subnet\n");
        return EXIT_FAILURE;  // Exit if failure
    }
    return cidr;
}


void *pingScan(void *arg) {
	while (1) {
		thread_data task;
        
		// Lock mutex to fetch task from queue
		pthread_mutex_lock(&task_mutex);

		// If no tasks left, exit the thread
		if (task_index >= task_count) {
		pthread_mutex_unlock(&task_mutex);
		break;
		}

		// Get the next task and increment the task index
		task = task_queue[task_index++];
		pthread_mutex_unlock(&task_mutex);
	}

	thread_data *data = (thread_data*)arg;
	char command[135];
	int start = atoi(data->start_ip);
	int end = atoi(data->end_ip);
	
	for (char i = start; i <= end; i++) {
		snprintf(command, sizeof(command), "ping -c 1 -W 1 %s.%d > /dev/null 2>&1 && echo \"" CYAN "[+]" RESET " %s.%d is Up\"", data->subnet, i, data->subnet, i);
		system(command);
	}
	pthread_exit(NULL);
}

void add_task(const char *subnet, const char *start_ip, const char *end_ip) {
	pthread_mutex_lock(&task_mutex);
	thread_data task;
	strncpy(task.subnet, subnet, sizeof(task.subnet));
	strncpy(task.start_ip, start_ip, sizeof(task.start_ip));
	strncpy(task.end_ip, end_ip, sizeof(task.end_ip));

	// Add task to the queue
	task_queue[task_count++] = task;
	pthread_mutex_unlock(&task_mutex);
	printf("Task: %s - %s to %s\n", subnet, start_ip, end_ip);
}

int main () {
	thread_data data;

	//Take user input and send it to teh subnet variable in the thread_data structure
	printf("Enter IP address and subnet range (ex. 192.168.1.0/24): ");
	scanf("%19s", data.subnet);
	
	if (strchr(data.subnet, '/') == NULL) {
		fprintf(stderr, RED "[-]" RESET " Invalid subnet format. Expected format: x.x.x.x/xx\n");
		return EXIT_FAILURE;
	}
	
	printf(CYAN "[+]" RESET " Using %s\n", data.subnet);
	
	//Get the cidr notation from the user input. Will be used to determine the ip range. 
	int cidr = get_cidr(data.subnet);
	char prefix[13];
	
	//Set temporary netmask to simplify the creation of end_ip
	char tmp_netmask[6] = ".255";
	
	if (cidr == 24) {
	//Set the prefix for the threads to run
	printf(CYAN "[+]" RESET " Creating /24 Prefix\n");
	size_t length = strlen(data.subnet) - 5;
	strncpy(prefix, data.subnet, length);
	strncpy(data.subnet, prefix, sizeof(data.subnet)); // Make sure to leave space for null terminator
	data.subnet[sizeof(data.subnet) - 1] = '\0';
	printf("%s (prefix)\n", data.subnet);
	
	//Set the start_ip
	strncpy(data.start_ip, data.subnet, length + 2);
	data.start_ip[length+2] = '\0';
	printf("%s (start ip)\n", data.start_ip);
	
	//Set the end_ip
	snprintf(data.end_ip, sizeof(data.end_ip), "%s%s", prefix, tmp_netmask);
	printf("%s (end ip)\n", data.end_ip);
	
	//Check if cidr range is /16
	} else if (cidr == 16) {
	printf(CYAN "[+]" RESET " Creating /16 Prefix\n");
	size_t length = strlen(data.subnet) - 7;
	strncpy(prefix, data.subnet, length);
	prefix[length] = '\0';
	
	//Set the start_ip
	printf(CYAN "[+]" RESET " Creating Starting IP\n");
	strncpy(data.start_ip, data.subnet, length + 2);
	data.start_ip[length+2] = '\0';
	
	//Set the end_ip
	printf(CYAN "[+]" RESET " Creating End IP\n");
	snprintf(data.end_ip, sizeof(data.end_ip), "%s%s%s", prefix, tmp_netmask, tmp_netmask);
	}
    
	//Start creating tasks
	printf(CYAN "[+]" RESET " Creating Tasks\n");
	int ips_per_task = 255 / THREADS;
	
	printf(CYAN "[+]" RESET " Running Commands With Threads\n");
	for (int i = 0; i < THREADS; i++) {
		char start_ip[20], end_ip[20];
		
		snprintf(start_ip, sizeof(start_ip), "%s.%d", data.subnet, i * ips_per_task + 1);
		int end_ip_num = (i == THREADS - 1) ? 254 : (i + 1) * ips_per_task;
		
		snprintf(end_ip, sizeof(end_ip), "%s.%d", data.subnet, end_ip_num);

		
		add_task(data.subnet, start_ip, end_ip);
	}

	pthread_t threads[THREADS];
	for (int i = 0; i < THREADS; i++) {
		if (pthread_create(&threads[i], NULL, pingScan, &task_queue[i]) != 0) {
		perror("Failed to create thread");
		return EXIT_FAILURE;
		}
	}

	// Wait for threads to complete
	for (int i = 0; i < THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("Ping scan completed.\n");
	return EXIT_SUCCESS; 
}
