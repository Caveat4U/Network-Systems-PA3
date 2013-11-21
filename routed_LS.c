#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> /* select() */
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>   /* memset() */
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
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
	Router router;
	router.router_id =  argv[1][0];
	router.num_links = 0;
	
	//LogFileName argv[2]
	FILE* log_file;
	log_file = fopen(argv[2], "w+");
	if(log_file == NULL)
	{
		fprintf(stderr, "Log file %s could not be opened.\n", argv[2]);
		return EXIT_FAILURE;
	}
	
	//Initialization File argv[3]
	FILE* init_file;
	init_file = fopen(argv[3], "r");
	if(init_file == NULL)
	{
		fprintf(stderr, "Initialization file %s could not be opened.\n", argv[3]);
		return EXIT_FAILURE;
	}
	
	// Create a storage mechanism for all the link states.
	Link_State link;
	
	// Parse the initialization file.
	// File format: <source router, source TCP port, destination router, destination TCP port, link cost>
	//<A,9701,B,9704,4>
	
	// initialize variables
	char source_router = 'Z';
	int source_tcp_port = -999;
	char destination_router = 'Z';
	int destination_tcp_port = -999;
	int link_cost = -999;
	struct sockaddr_in local_addr, remote_addr;

	printf("Router: %c\n", router.router_id);
	printf("Immediate neighbors:\n");
	
	while( fscanf(init_file, "<%c,%d,%c,%d,%d>\n", &source_router, &source_tcp_port, 
					&destination_router, &destination_tcp_port, &link_cost) != EOF) 
	{
		// If we're looking at the correct part of the file.
		if ( source_router == router.router_id )
		{
			// Store data in the link state, increment num_links
			link = router.links[router.num_links];
			link.source_router = source_router;
			link.source_tcp_port = source_tcp_port;
			link.destination_router = destination_router;
			link.destination_tcp_port = destination_tcp_port;
			link.link_cost = link_cost;
			router.num_links++;
			printf("Destination RID: %c, Destination Port: %d, Source Port: %d, Cost: %d>\n", 
						destination_router, destination_tcp_port, source_tcp_port, link_cost);

		}
	}

	int i;
	for (i = 0; i < router.num_links; i++);
	{
		if((router.links[i].l_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		{
			printf("Link to Router %c failed.\n", router.links[i].source_router);
		}	
		
		// Clear socket buffers
		bzero(&router.links[i].local_addr, sizeof(router.links[i].local_addr));
		bzero(&router.links[i].remote_addr, sizeof(router.links[i].local_addr));
		
		// htons() sets the port # to network byte order
		// Setup for local
		router.links[i].local_addr.sin_family = AF_INET;
		router.links[i].local_addr.sin_addr.s_addr = INADDR_ANY;
		router.links[i].local_addr.sin_port = htons(link.source_tcp_port);
		// Setup for remote	
		router.links[i].remote_addr.sin_family = AF_INET;
		router.links[i].remote_addr.sin_addr.s_addr = INADDR_ANY;
		router.links[i].remote_addr.sin_port = htons(link.destination_tcp_port);
		
		// Flag to check on connection later
		router.links[i].connected = 0;
	}


	int j;
	for (j = 0; j < router.num_links; j++)
	{
		local_addr = router.links[j].local_addr;
		remote_addr = router.links[j].remote_addr;
		
		if(connect(router.links[j].l_sockfd, (struct sockaddr*)&(remote_addr),
					sizeof(remote_addr)) == 0)
		{
			router.links[j].connected = 1;
			router.links[j].sockfd = router.links[j].l_sockfd;
			printf("Connected from %c to %c \n", 
					router.links[j].source_router, router.links[j].destination_router);
			
		}else{
			router.links[j].connected = 0;
			printf("Failed to connect from %c to %c \n", 
					router.links[j].source_router, router.links[j].destination_router);
			//create socket to listen again
			if ((router.links[i].l_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("Socket from %c to %c could not be opened. \n", 
						router.links[j].source_router, router.links[j].destination_router);
			}
			//bind socket to local_addr
			if(bind(router.links[i].l_sockfd, (struct sockaddr*)&(local_addr), sizeof(local_addr)) < 0);
			{
				printf("Could not bind to socket for %c to %c \n", 
						router.links[j].source_router, router.links[j].destination_router);
				exit(1);
			}
			//make socket non blocking.
			if(fcntl(router.links[i].l_sockfd, F_SETFL, O_NDELAY) < 0)
			{
				printf("Could not un-block socket for %c to %c \n",
						router.links[j].source_router, router.links[j].destination_router);
			}
			//listen
			if(listen(router.links[i].l_sockfd, MAX_LINKED_STATES) < 0)
			{
				printf("Listen failed for %c to %c \n", 
						router.links[j].source_router, router.links[j].destination_router);
			}	
		}
	}
	
	// OK, so now we have an array of things in our current routing table.
	// They're bound and gagged/listening.
	
	// Next, we have to shoot out packets to any neighbors and exchange loving messages of adoration.
	// Possible commands to call:
	//  accept(), select()
	
	// So that we don't have hangover problems, let's sleep it off.
	sleep(20);
	
	// It's time to connect everything together. Hooray!
	int k;
	struct sockaddr_in client_shenanigans;
	for(k = 0; k < router.num_links; k++ )
	{
		// Everything else in the struct above still applies just fine, so just change the port.
		//this_sock_addr.sin_port = htons(local_states[i].destination_tcp_port);
		
		//if( connect(local_states[i].sockfd, (struct sockaddr *)&this_sock_addr, sizeof(this_sock_addr)) < 0 )
		{
			//fprintf(stderr, "Oh no! Our connect failed from %c to %c. Crap!\n", local_states[i].source_router, destination_router);
			//return EXIT_FAILURE;
		}
		
		// Zero out the struct.
		bzero(&client_shenanigans, sizeof(client_shenanigans)); 
			
		// accept everything we can.
		// accept() basically dequeues this from the listen() world.
		// All communication happens through this new socket, so let's overwrite the old one.
		// if((local_states[i].sockfd = accept(local_states[i].sockfd, (struct sockaddr *)&client_shenanigans, (socklen_t *)sizeof(client_shenanigans))) < 0) {
			//fprintf(stderr, "Could not accept stuff where we needed to accept stuff.\n");
			//return EXIT_FAILURE;
		// }
	}
	
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
