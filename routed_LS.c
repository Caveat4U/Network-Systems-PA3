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

// Making these global so they can be killed by the signal exit.
Router router;
FILE* log_file;

void graceful_exit();

int main(int argc, char *argv[]) {
	if (argc != 4)
	{
		fprintf(stderr, "Incorrect number of arguments. Usage should be:\n %s\n", USAGE);
		return EXIT_FAILURE;
	}
	
	//routerID argv[1]
	router.router_id =  argv[1][0];
	router.num_links = 0;
	
	//LogFileName argv[2]
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

	// Parse the initialization file.
	// File format: <source router, source TCP port, destination router, destination TCP port, link cost>
	//<A,9701,B,9704,4>
	
	// initialize variables and data structures.
	char source_router;
	int source_tcp_port;
	char destination_router;
	int destination_tcp_port;
	int link_cost;
	
	time_t curr_time;
	router.l_archive.length = 0;
	LSP buffer;

	printf("Router: %c\n", router.router_id);
	printf("Immediate neighbors:\n");
	
	while( fscanf(init_file, "<%c,%d,%c,%d,%d>\n", &source_router, &source_tcp_port, 
					&destination_router, &destination_tcp_port, &link_cost) != EOF) 
	{
		// If we're looking at the correct part of the file.
		if (source_router == router.router_id)
		{
			// Store data in the link state, increment num_links
			router.links[router.num_links].source_router = source_router;
			router.links[router.num_links].source_tcp_port = source_tcp_port;
			router.links[router.num_links].destination_router = destination_router;
			router.links[router.num_links].dest_tcp_port = destination_tcp_port;
			router.links[router.num_links].link_cost = link_cost;
			router.links[router.num_links].connected = 0;
			printf("Link Num: %d Destination RID: %c, Destination Port: %d, Source Port: %d, Cost: %d\n", 
						router.num_links+1, router.links[router.num_links].destination_router, 
						router.links[router.num_links].dest_tcp_port, router.links[router.num_links].source_tcp_port, 
						router.links[router.num_links].link_cost);
			router.num_links += 1;
		}
	}
	fclose(init_file);
		
	int i;
	for (i = 0; i < router.num_links; i++)
	{
		if((router.links[i].l_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Socket creation for Router %c failed.\n", router.links[i].source_router);
		}	
		// Clear socket buffers
		bzero(&router.links[i].local_addr, sizeof(router.links[i].local_addr));
		bzero(&router.links[i].remote_addr, sizeof(router.links[i].local_addr));
		
		// htons() sets the port # to network byte order
		// Setup for local
		router.links[i].local_addr.sin_family = AF_INET;
		router.links[i].local_addr.sin_addr.s_addr = INADDR_ANY;
		router.links[i].local_addr.sin_port = htons(router.links[i].source_tcp_port);
		// Setup for remote	
		router.links[i].remote_addr.sin_family = AF_INET;
		router.links[i].remote_addr.sin_addr.s_addr = INADDR_ANY;
		router.links[i].remote_addr.sin_port = htons(router.links[i].dest_tcp_port);
		
		// Flag to check on connection later
		router.links[i].connected = 0;
	}
	sleep(10);
	
	int j;
	for (j = 0; j < router.num_links; j++)
	{
		if(connect(router.links[j].l_sockfd, (struct sockaddr*)&(router.links[j].remote_addr), sizeof(router.links[j].remote_addr)) == 0)
		{
			router.links[j].connected = 1;
			router.links[j].sockfd = router.links[j].l_sockfd;
			printf("Connected from %c to %c (%d -> %d)\n", 
					router.links[j].source_router, router.links[j].destination_router,
					router.links[j].source_tcp_port, router.links[j].dest_tcp_port);
			
		}else{
			router.links[j].connected = 0;
			printf("Failed to connect from %c to %c (%d -> %d)\n", 
					router.links[j].source_router, router.links[j].destination_router,
					router.links[j].source_tcp_port, router.links[j].dest_tcp_port);
			//create socket to listen again
			if ((router.links[j].l_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("Socket from %c to %c could not be opened. (%d -> %d)\n", 
						router.links[j].source_router, router.links[j].destination_router,
						router.links[j].source_tcp_port, router.links[j].dest_tcp_port);
			}
			//bind socket to local_addr
			if(bind(router.links[j].l_sockfd, (struct sockaddr*)&(router.links[j].local_addr), sizeof(router.links[j].local_addr)) < 0)
			{
				printf("Could not bind to socket for %c to %c (%d -> %d)\n", 
						router.links[j].source_router, router.links[j].destination_router,
						router.links[j].source_tcp_port, router.links[j].dest_tcp_port);
				fclose(log_file);
	
				for (i=0; i<router.num_links; i++)
				{
					close(router.links[i].sockfd);
					close(router.links[i].l_sockfd);
				}
				exit(1);
			}
			//make socket non blocking.
			if(fcntl(router.links[j].l_sockfd, F_SETFL, O_NDELAY) < 0)
			{
				printf("Could not un-block socket for %c to %c (%d -> %d)\n",
						router.links[j].source_router, router.links[j].destination_router,
						router.links[j].source_tcp_port, router.links[j].dest_tcp_port);
			}
			//listen
			if(listen(router.links[j].l_sockfd, MAX_LINKED_STATES) < 0)
			{
				printf("Listen failed for %c to %c (%d -> %d)\n", 
						router.links[j].source_router, router.links[j].destination_router,
						router.links[j].source_tcp_port, router.links[j].dest_tcp_port);
			}	
		}
	}
	
	//Start timer for timeouts/sleep
	time(&router.time);
	// Lets make an LSP! (TTL = 10)
	router.seq = 1;
	router.lsp.router_id = router.router_id;
	router.lsp.length = router.num_links;
	router.lsp.seq = router.seq;
	router.lsp.ttl = 10;
	
	//Update link data on lsp's
	int k;
	for(k = 0; k < router.num_links; k++)
	{
		router.lsp.link_table[k].destination_router = router.links[k].destination_router;
		router.lsp.link_table[k].link_cost = router.links[k].link_cost;
	}
	
	//Setup routing table
	router.r_table.length = router.num_links;
	for(i = 0; i < router.num_links; i++)
	{
		router.r_table.row[i].destination_router = router.links[i].destination_router;
		router.r_table.row[i].next_hop = router.links[i].destination_router;
		router.r_table.row[i].link_cost = router.links[i].link_cost;
		router.r_table.row[i].source_tcp_port = router.links[i].source_tcp_port;
		router.r_table.row[i].dest_tcp_port = router.links[i].dest_tcp_port;
	}
	
	print_header(&router, log_file);
	//Listen for LSP's
	while(1)
	{
		signal(SIGINT, graceful_exit);
		for(j = 0; j < router.num_links; j++)
		{
			//Accept link
			if(router.links[j].connected == 0)
			{
				router.links[j].sockfd = accept(router.links[j].l_sockfd, NULL, (socklen_t*)sizeof(struct sockaddr_in));
				//Establish connection
				if(router.links[j].sockfd > 0)
				{
					router.links[j].connected = 1;
					printf("Connected from %c to %c \n", 
							router.links[j].source_router, router.links[j].destination_router);
					//unblock socket
					if(fcntl(router.links[j].sockfd, F_SETFL, O_NDELAY) < 0)
					{
						printf("Could not unblock socket for %c to %c \n",
								router.links[j].source_router, router.links[j].destination_router);
					}
				}
			}
		}
		//update/synchronize time and send lsp's
		int nbytes;
		time(&curr_time);
		if(difftime(curr_time, router.time) >= 5.0)
		{
			router.time = curr_time;
			router.lsp.seq++;
			for(k = 0; k < router.num_links; k++)
			{
				if(router.links[k].connected)
				{
					if((nbytes = send(router.links[k].sockfd, &router.lsp, sizeof(LSP), 0)) == -1)
					{
						printf("Failed to send from %c to %c \n",
								router.links[k].source_router, router.links[k].destination_router);
					}else{
						printf("Sent LSP from %c to %c \n",
								router.links[k].source_router, router.links[k].destination_router);
					}
				}
			}
		}
		for( i = 0; i < router.num_links; i++)
		{
			if(router.links[i].connected)
			{
				if((nbytes = recv(router.links[i].sockfd, &buffer, sizeof(LSP), 0)) < 0)
				{
					print_header(&router, log_file);

					// If we receive an exit gracefully code.
					if(buffer.seq == -999)
					{
						
						router.lsp.seq = -999;
						
						//close the connections
						for (i=0; i<router.num_links; i++)
						{
							// spam it to the neighbors -- we don't care if they're connected.
							send(router.links[i].sockfd, &router.lsp, sizeof(LSP), 0);
							close(router.links[i].sockfd);
							close(router.links[i].l_sockfd);
						}
						
						//close the log file
						fclose(log_file);
						return EXIT_SUCCESS;
					}
					//printf("LSP %d Received from %c", buffer.router_id, buffer.seq);
				}
			}
		}
		
	}
	
    for (i=0; i<router.num_links; i++)
    {
        close(router.links[i].sockfd);
        close(router.links[i].l_sockfd);
    }
    return 0;
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

void graceful_exit()
{ 		 
	// This is here in case the OS reset the signal handler after each call.
	// That way this program can finish even if you press ctrl+c a bunch of times.
	signal(SIGINT, graceful_exit);
 
	printf("Now closing all other windows.\n");
	
	int i, nbytes;
	// Send the closing packet
	for(i = 0; i < router.num_links; i++)
	{
		if(router.links[i].connected)
		{
			router.lsp.seq = -999;
			if((nbytes = send(router.links[i].sockfd, &router.lsp, sizeof(LSP), 0)) == -1)
			{
				printf("Failed to send from %c to %c \n",
						router.links[i].source_router, router.links[i].destination_router);
			}else{
				printf("Sent LSP from %c to %c \n",
						router.links[i].source_router, router.links[i].destination_router);
			}
		}
	}
	
	// Close the local connections.
	for (i=0; i<router.num_links; i++)
	{
		close(router.links[i].sockfd);
		close(router.links[i].l_sockfd);
	}
	
	// Close the log file
	fclose(log_file);
	sleep(10);
	exit(0);
}
