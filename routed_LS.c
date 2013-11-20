#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>   /* memset() */
#include <sys/time.h> /* select() */
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "routed_LS.h"

#define USAGE "./routed_LS <RouterID> <LogFileName> <Initialization file>"
#define INF -999
#define MAX_LINKED_STATES 10

int main(int argc, char *argv[]) {
	if (argc != 4)
	{
		fprintf(stderr, "Incorrect number of arguments. Usage should be:\n %s\n", USAGE);
		return EXIT_FAILURE;
	}
	
	//routerID argv[1]
	char routerID = argv[1][0];
	
	//LogFileName argv[2]
	FILE* log_file;
	log_file = fopen(argv[2], "w+");
	if(log_file == NULL)
	{
		fprintf(stderr, "Log file %s could not be opened.\n", argv[2]);
		return EXIT_FAILURE;
	}
	
	//Initialization File argv[3]
	FILE* initialization_file;
	initialization_file = fopen(argv[3], "r");
	if(initialization_file == NULL)
	{
		fprintf(stderr, "Initialization file %s could not be opened.\n", argv[3]);
		return EXIT_FAILURE;
	}
	
	// Create a storage mechanism for all the link states.
	struct link_state local_states[MAX_LINKED_STATES];
	
	// Parse the initialization file.
	// File format: <source router, source TCP port, destination router, destination TCP port, link cost>
	//<A,9701,B,9704,4>
	
	char source_router = 'Z';
	int source_tcp_port = -999;
	char destination_router = 'Z';
	int destination_tcp_port = -999;
	int link_cost = -999;
	struct sockaddr_in this_sock_addr;
	this_sock_addr.sin_family = AF_INET;      		//address family
	this_sock_addr.sin_addr.s_addr = INADDR_ANY;    //supplies the IP address of the local machine
	
	int count = 0;
	
	while( fscanf(initialization_file, "<%c,%d,%c,%d,%d>\n", &source_router, &source_tcp_port, &destination_router, &destination_tcp_port, &link_cost) != EOF) 
	{
		// If we're looking at the correct part of the file.
		if ( source_router == routerID )
		{
			// store the data
			local_states[count].source_router = source_router;
			local_states[count].source_tcp_port = source_tcp_port;
			local_states[count].destination_router = destination_router;
			local_states[count].destination_tcp_port = destination_tcp_port;
			local_states[count].link_cost = link_cost;
			
			// htons() sets the port # to network byte order		
			this_sock_addr.sin_port = htons(local_states[count].source_tcp_port); 
			
			
			// create the necessary socket and bind it
			local_states[count].sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (bind(local_states[count].sockfd, (struct sockaddr *)&this_sock_addr, sizeof(this_sock_addr))<0)
			{
				fprintf(stderr, "Could not bind socket to port.");
				return EXIT_FAILURE;
			}
			
			// TODO - can we go ahead and throw our listen in here?
			// Can we use MAX_LINKED_STATES?
			if (listen(local_states[count].sockfd, 10) < 0)
			{
				fprintf(stderr, "There was a problem listening.\n");
				return EXIT_FAILURE;
			}
			count++;
			printf("<%c,%d,%c,%d,%d>\n", source_router, source_tcp_port, destination_router, destination_tcp_port, link_cost);
		}
			
		//sleep(5);
		
	}
	
	// OK, so now we have an array of things in our current routing table.
	// Next, we have to shoot out packets to any neighbors and exchange loving messages of adoration.
	// Possible command to call:
	// listen(), accept(), select()
	
	
	
	return EXIT_SUCCESS;
}


//Dijkstra’s Shortest Path Algorithm
//Let N = set of nodes in graph
//l(i,j) = link cost between i,j (= infinity if not neighbors)
//SPT = set of nodes in shortest path tree thus far, initially empty except for source
//S = source node
//C(n) = cost of path from S to node n
//Initialize shortest path tree SPT = {S}
//For each n not in SPT, C(n) = l(s,n)
//While (SPT<>N)
	//SPT = SPT U {w} such that C(w) is minimum for all w in (N-SPT). Remember its parent-child link.
	//For each n in (N-SPT)
		//C(n) = MIN (C(n), C(w) + l(w,n))

//Initialize shortest path tree SPT = {B}
//For each n not in SPT, C(n) = l(s,n)
	//C(E) = 1, C(A) = 3, C(C) = 4, C(others) = infinity
//Add closest node to the root: SPT = SPT U {E} since C(E) is minimum for all w not in SPT.
//Recalculate C(n) = MIN (C(n), C(E) + l(E,n)) for all nodes n not yet in SPT
//Loop again, select node A = has lowest cost path
//continues on slide 11
