
all:	ssl-session-reuse-client ssl-write ssl-clients

ssl-session-reuse-client:	ssl-session-reuse-client.c
	gcc -o ssl-session-reuse-client -O2 -g ssl-session-reuse-client.c -lssl -lpthread

ssl-write:	ssl-write.c
	gcc -o ssl-write -O2 -g ssl-write.c -lssl -lpthread

ssl-clients:	ssl-clients.c
	gcc -o ssl-clients -O2 -g ssl-clients.c -lssl -lpthread

