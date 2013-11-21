
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
