
struct link{
	char destination_router;
	int link_cost
};

struct lsp{
	char router_id;
	int seq;
	int ttl;
	int length;
	link link_table[MAX_LINKED_STATES];
};

struct link_archive{
	lsp archive[MAX_LINKED_STATES]
	int length;
};

struct link_state {
	char source_router;
	int source_tcp_port;
	char destination_router;
	int destination_tcp_port;
	int link_cost;
	int l_sockfd; 	// Socket used for listening and connecting
	int sockfd; 	// Socket for maintaining connection
	int connected;
};

struct routing_table_row {
	char destination_router;
	int source_tcp_port;
	int destination_tcp_port;
	int link_cost;
	char next_hop;
};

struct routing_table {
	routing_table_row table[MAX_ROUTERS]
	int length;
};

struct router {
	char router_id;
	int num_links;
	int 
	link_state links[MAX_LINKED_STATES];
	routing_table r_table;
	link_archive l_archive;
};
