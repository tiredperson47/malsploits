#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define CYAN "\033[1;36m"
#define RED "\e[1;31m"
#define RESET "\033[0m"
#define THREADS 85

typedef struct {
	char subnet[20];
	char start_ip[15];
	char end_ip[17];
} thread_data;


int get_cidr(const char *subnet) {
    int cidr = 0;
    if (sscanf(subnet, "%*[^/]/%d", &cidr) != 1 || cidr != 16 || cidr != 24) {
        fprintf(stderr, RED "[-]" RESET " Failed to extract CIDR from subnet\n");
        return EXIT_FAILURE;  // Exit if failure
    }
    return cidr;
}


void *pingScan(void *arg) {
	thread_data *data = (thread_data*)arg;
	char command[135];
	int start = atoi(data->start_ip);
	int end = atoi(data->end_ip);
	
	for (char i = start; i <= end; i++) {
		snprintf(command, sizeof(command), "ping -c 1 -W 1 %s.%d > /dev/null 2>&1 && echo \"" CYAN "[+]" RESET " %s.%d is Up\" &", data->subnet, i, data->subnet, i);
		system(command);
	}
	pthread_exit(NULL);
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
	size_t length = strlen(data.subnet) - 5;
	strncpy(prefix, data.subnet, length);
	prefix[length] = '\0';
	
	//Set the start_ip
	strncpy(data.start_ip, data.subnet, length + 2);
	data.start_ip[length+2] = '\0';
	
	//Set the end_ip
	snprintf(data.end_ip, sizeof(data.end_ip), "%s%s", prefix, tmp_netmask);
	
	//Check if cidr range is /16
	} else if (cidr == 16) {
	size_t length = strlen(data.subnet) - 7;
	strncpy(prefix, data.subnet, length);
	prefix[length] = '\0';
	
	//Set the start_ip
	strncpy(data.start_ip, data.subnet, length + 2);
	data.start_ip[length+2] = '\0';
	
	//Set the end_ip
	snprintf(data.end_ip, sizeof(data.end_ip), "%s%s%s", prefix, tmp_netmask, tmp_netmask);
	
	} else {
	printf(RED "[-]" RESET " Unsupported CIDR range");
	return EXIT_FAILURE;
	}
    
	//Start creating threads
	pthread_t threads[THREADS];
	thread_data thread_data[THREADS];
	int ips_per_thread = 255 / THREADS;
	
	for (int i = 0; i < THREADS; i++) {
		snprintf(thread_data[i].start_ip, sizeof(thread_data[i].start_ip), "%d", i * ips_per_thread + 1);
		snprintf(thread_data[i].end_ip, sizeof(thread_data[i].end_ip), "%d", (i == THREADS - 1) ? 254 : (i + 1) * ips_per_thread);

		strncpy(thread_data[i].subnet, data.subnet, sizeof(thread_data[i].subnet));

		if (pthread_create(&threads[i], NULL, pingScan, &thread_data[i]) != 0) {
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
