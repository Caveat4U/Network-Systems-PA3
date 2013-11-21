
#include <netinet/in.h>

#define MAX_LINKED_STATES 10
#define MAX_ROUTERS 24

typedef struct LSP_Info {
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

typedef struct Link_State {
	char source_router;
	int source_tcp_port;
	char destination_router;
	int destination_tcp_port;
	int link_cost;
	int l_sockfd; 	// Socket used for listening and connecting
	int sockfd; 	// Socket for maintaining connection
	int connected;
	struct sockaddr_in local_addr, remote_addr;
}Link_State;

typedef struct Routing_Table_Row {
	char destination_router;
	int source_tcp_port;
	int destination_tcp_port;
	int link_cost;
	char next_hop;
}Routing_Table_Row;

typedef struct Routing_Table {
	Routing_Table_Row table[MAX_ROUTERS];
	int length;
}Routing_Table;

typedef struct Router {
	char router_id;
	int num_links;
	LSP source_lsp;
	int lsp_seq;
	Link_State links[MAX_LINKED_STATES];
	Routing_Table r_table;
	Link_Archive l_archive;
}Router;
