
all:	ssl-session-reuse-client ssl-write ssl-clients tcp-clients tcp-single ssl-post

ssl-session-reuse-client:	ssl-session-reuse-client.c
	gcc -o ssl-session-reuse-client -O2 -g ssl-session-reuse-client.c -lssl -lpthread

ssl-write:	ssl-write.c
	gcc -o ssl-write -O2 -g ssl-write.c -lssl -lpthread

ssl-post:	ssl-post.c
	gcc -o ssl-post -O2 -g ssl-post.c -lssl -lpthread -lcrypto

ssl-clients:	ssl-clients.c
	gcc -o ssl-clients -O2 -g ssl-clients.c -lssl -lpthread

tcp-clients:	tcp-clients.c
	gcc -o tcp-clients -O2 -g tcp-clients.c -lpthread

tcp-single:	tcp-single.c
	gcc -o tcp-single -O2 -g tcp-single.c -lpthread

