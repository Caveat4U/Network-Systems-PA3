all: clean routed_LS

routedLS: routed_LS.c
	gcc -Wall routed_LS.c -o routed_LS

clean:
	rm -f routed_LS
