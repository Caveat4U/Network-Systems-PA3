#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#define MAX_LINKED_STATES 10
#define MAX_ROUTERS 24

typedef struct LSP_Info{
	char destination_router;
	int link_cost;
} LSP_Info;

typedef struct LSP{
	char router_id;
	int seq;
	int ttl;
	int length;
	LSP_Info link_table[MAX_LINKED_STATES];
}LSP;

typedef struct Link_Archive{
	LSP archive[MAX_LINKED_STATES];
	int length;
}Link_Archive;

typedef struct Link_State{
	char source_router;
	int source_tcp_port;
	char destination_router;
	int dest_tcp_port;
	int link_cost;
	int l_sockfd; 	// Socket used for listening and connecting
	int sockfd; 	// Socket for maintaining connection
	int connected;
	struct sockaddr_in local_addr, remote_addr;
}Link_State;

typedef struct Routing_Table_Row{
	char destination_router;
	int source_tcp_port;
	int dest_tcp_port;
	int link_cost;
	char next_hop;
}Routing_Table_Row;

typedef struct Routing_Table{
	Routing_Table_Row row[MAX_ROUTERS];
	int length;
}Routing_Table;

typedef struct Router{
	char router_id;
	int num_links;
	int seq;
	time_t time;
	LSP lsp;
	Link_State links[MAX_LINKED_STATES];
	Routing_Table r_table;
	Link_Archive l_archive;
}Router;

void print_header(Router* r, FILE* file)
{
	int i;
	char str[1024];
	sprintf(str, "Routing table for %c", r->router_id);
	
	if(file)
	{ 
		fwrite(str, sizeof(char), strlen(str), file);
	}
		
	//Routing_Table r_table;
		//Routing_Table_Row row[MAX_ROUTERS];
			//char destination_router;
			//int source_tcp_port;
			//int dest_tcp_port;
			//int link_cost;
			//char next_hop;
		//int length;
	for( i = 0; i < r->r_table.length; i++)
	{
		sprintf(str,
		"\n Routing Table Row %d:\n \
			Destination: %c\n \
			Ports: %d => %d\n \
			Cost: %d\n \
			Next Hop: %c\n",
		i,
		r->r_table.row[i].destination_router,
		r->r_table.row[i].source_tcp_port,
		r->r_table.row[i].dest_tcp_port,
		r->r_table.row[i].link_cost,
		r->r_table.row[i].next_hop);	
		
		if(file)
		{ 
			fwrite(str,sizeof(char),strlen(str),file);
		}
	}
	
	
	//Link_Archive l_archive;
	
}
