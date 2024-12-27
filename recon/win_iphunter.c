#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <math.h>

#define THREADS 4
#define TASKS 85

typedef struct {
	unsigned int ip_int;
	int start_ip;
	int end_ip;
} thread_data;

CRITICAL_SECTION task_mutex;
HANDLE task_cond;
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


unsigned int __stdcall pingScan(void *arg) {
	
	thread_data *data = (thread_data*)arg;
	char command[150];
	
	for (int i = data->start_ip; i <= data->end_ip; i++) {
		char ipv4[20];
		int_to_ip(i, ipv4);
        _snprintf(command, sizeof(command), "cmd /c \"ping -n 1 -w 800 %s > NUL 2>&1 && echo [+] %s is Up\"", ipv4, ipv4);
		system(command);
	}
	return 0;
}


void add_task(int start_ip, int end_ip) {
	EnterCriticalSection(&task_mutex);
	thread_data task;
	
	task.start_ip = start_ip;
	task.end_ip = end_ip;

	// Add task to the queue
	task_queue[task_count++] = task;
	LeaveCriticalSection(&task_mutex);
}


int main (int argc, char *argv[]) {

	printf("    _       __  __            __           \n");
	printf("   (_)___  / / / /_  ______  / /____  _____\n");
	printf("  / / __ \\/ /_/ / / / / __ \\/ __/ _ \\/ ___/\n");
	printf(" / / /_/ / __  / /_/ / / / / /_/  __/ /    \n");
	printf("/_/ .___/_/ /_/\\__,_/_/ /_/\\__/\\___/_/     \n");
	printf(" /_/                                       \n\n");
	printf("Authors: Ryan W. (tiredperson47) and Jerry S. (jsouliss)\n\n\n");
	
	WSADATA wsa;
	
	if (argc != 2) {  // Check if the user provided exactly one argument
		fprintf(stderr, "[-] Invalid usage. Expected format: x.x.x.x/xx\n");
		return EXIT_FAILURE;
	}
	char *subnet = argv[1];
	
	InitializeCriticalSection(&task_mutex);
	thread_data data[THREADS];

	//Take user input and send it to the subnet variable in the thread_data structure
	
	if (strchr(subnet, '/') == NULL) {
		fprintf(stderr, "[-] Invalid subnet format. Expected format: x.x.x.x/xx\n");
		WSACleanup();
		return EXIT_FAILURE;
	}
		
	//Get the cidr notation from the user input. Will be used to determine the ip range. 
	int cidr = get_cidr(subnet);
	if (cidr >= 32) {
        	fprintf(stderr, "[-] Unsuported CIDR from subnet. Limit is /31\n");
		WSACleanup();
        	return EXIT_FAILURE;  // Exit if failure
        }
	
	
	//Changes the subnet from ip/24 to ip
	//ip will be used to set the start ip and end ip. (will figure out how)
	char ip[20];
	size_t length = strlen(subnet) - 3;
	strncpy(ip, subnet, length);
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

	HANDLE threads[THREADS];
	for (int i = 0; i < THREADS; i++) {
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, pingScan, &task_queue[i], 0, NULL);
        if (threads[i] == 0) {
            perror("Failed to create thread");
            WSACleanup();
            return EXIT_FAILURE;
        }
    }

    WaitForMultipleObjects(THREADS, threads, TRUE, INFINITE);

    DeleteCriticalSection(&task_mutex);
    WSACleanup();

	printf("Ping scan completed.\n");
	return 0; 
}
