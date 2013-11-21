#include <stdio.h>
#include <netinet/in.h>

#define MAX_LINKED_STATES 10
#define MAX_ROUTERS 24

typedef struct {
	char destination_router;
	int link_cost;
} LSP_Info;

typedef struct {
	char router_id;
	int seq;
	int ttl;
	int length;
	LSP_Info link_table[MAX_LINKED_STATES];
}LSP;

typedef struct {
	LSP archive[MAX_LINKED_STATES];
	int length;
}Link_Archive;

typedef struct {
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

typedef struct {
	char destination_router;
	int source_tcp_port;
	int dest_tcp_port;
	int link_cost;
	char next_hop;
}Routing_Table_Row;

typedef struct {
	Routing_Table_Row row[MAX_ROUTERS];
	int length;
}Routing_Table;

typedef struct {
	char router_id;
	int num_links;
	int seq;
	time_t time;
	LSP lsp;
	Link_State links[MAX_LINKED_STATES];
	Routing_Table r_table;
	Link_Archive l_archive;
}Router;

void print_header(Router r)
{
	int i;
	fprintf(stderr,
	"Router: %c\
	Num Links: %d\
	Seq: %d\
	Time: %d\
	LSP:\
		seq: %d\
		ttl: %d\
		length: %d\
		link_table:\
	",
	r.router_id,
	r.num_links,
	r.seq,
	(int)r.time,
	r.lsp.seq,
	r.lsp.ttl,
	r.lsp.length
	);
	
	// Print out the link table in lsp.
	for( i = 0; i < MAX_LINKED_STATES; i++)
	{
		// TODO: Leaving out invalid states...should I?
		if(r.lsp.link_table[i].destination_router != 0)
		{
			fprintf(stderr,
			"		LSP %d Destination:%c with cost: %d",
			i,
			r.lsp.link_table[i].destination_router,
			r.lsp.link_table[i].link_cost
			);
		}
	}
	
	//Link_State links[MAX_LINKED_STATES];
		//char source_router;
		//int source_tcp_port;
		//char destination_router;
		//int dest_tcp_port;
		//int link_cost;
		//int l_sockfd; 	// Socket used for listening and connecting
		//int sockfd; 	// Socket for maintaining connection
		//int connected;
		//struct sockaddr_in local_addr, remote_addr;
	for ( i = 0; i < MAX_LINKED_STATES; i++)
	{
		fprintf(stderr,
		"Link State %d:\
			Source: %c on %d\
			Destination: %c on %d\
			Cost: %d\
			Connected? %d",
		i,
		r.links[i].source_router,
		r.links[i].source_tcp_port,
		r.links[i].destination_router,
		r.links[i].dest_tcp_port,
		r.links[i].link_cost,
		r.links[i].connected
		);
	}
	
	//Routing_Table r_table;
		//Routing_Table_Row row[MAX_ROUTERS];
			//char destination_router;
			//int source_tcp_port;
			//int dest_tcp_port;
			//int link_cost;
			//char next_hop;
		//int length;
	for( i = 0; i < r.r_table.length; i++)
	{
		fprintf(stderr,
		"Routing Table Row %d:\
			Destination: %c\
			Ports: %d => %d\
			Cost: %d\
			Next Hop: %c",
		i,
		r.r_table.row[i].destination_router,
		r.r_table.row[i].source_tcp_port,
		r.r_table.row[i].dest_tcp_port,
		r.r_table.row[i].link_cost,
		r.r_table.row[i].next_hop
		);
	}
	
	
	//Link_Archive l_archive;
	
}
