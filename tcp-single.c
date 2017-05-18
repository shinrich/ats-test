#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/ssl.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#define _GNU_SOURCE
#include <pthread.h>

#define NUM_THREADS 10

//char *port_url[] = { "", "8080", "8081", "8082", "8083" , "8084", "8085", "8086", "8087", "8088", "8089", "8090", "8091", "8092", "8093", "8094", "8095", "8096" };
char *port_url[] = { "", "8080", "8081", "8082" };
char *host = "localhost";

char req_buf[1024];
char post_req_buf[1024];
char post_data_buf[2048];

pthread_mutex_t *mutex_buf = NULL;

struct thread_info 
{
  struct addrinfo *result, *rp;
  int count;
  int do_get;
}; 

void *spawn_same_session_send(void *arg) 
{
  int count = 0;
  char my_req_buf[1024];
  struct thread_info *tinfo = (struct thread_info *)arg;
  struct sockaddr_in addr;
  memcpy(&addr, tinfo->rp->ai_addr, tinfo->rp->ai_addrlen);
  int port_index = rand() % (sizeof(port_url)/sizeof(char *));

 {
  int num = rand();
  if (tinfo->do_get) {
    snprintf(my_req_buf, sizeof(my_req_buf), "GET /%s/%x/%d HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", port_url[port_index], tinfo, num, host);
  } else {
    snprintf(my_req_buf, sizeof(my_req_buf), "POST /%s/%x/%d/post_data.php HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Length: %d\r\n\r\n", port_url[port_index], tinfo, num, host, strlen(post_data_buf));
  }
  int sfd = socket(tinfo->rp->ai_family, tinfo->rp->ai_socktype,
                   tinfo->rp->ai_protocol);
  if (sfd == -1) 
  {
    printf("Failed to get socket");
    perror("Failed");
    close(sfd);
    pthread_exit((void *)1);
  }

  struct linger so_linger;
  so_linger.l_onoff = 1;
  so_linger.l_linger = 10;
  setsockopt(sfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
  if (connect(sfd, &addr, tinfo->rp->ai_addrlen) < 0)
  {
    printf("Failed to connect %d\n", sfd);
    perror("Failed");
    close(sfd);
    pthread_exit((void *)1);
  }

  fcntl(sfd, F_SETFL, O_NONBLOCK);
  
  int i;
  int ret;
  for (i = 0; i < tinfo->count; i++) {
    // Send request
    int total_wrote = 0;
    int amount_to_write = strlen(my_req_buf);
    if (tinfo->do_get) {
      do { 
        ret = write(sfd, my_req_buf + total_wrote, amount_to_write - total_wrote);
        if (ret > 0) total_wrote += ret; 
      } while (ret != 0 && total_wrote < amount_to_write);
      if (total_wrote != amount_to_write) {
        printf("Failed to write all of get request %d of %d\n", total_wrote, amount_to_write);
        pthread_exit((void *)4);
      }
    } else {
      struct iovec iovs[2];
      iovs[0].iov_base = my_req_buf;
      iovs[0].iov_len = amount_to_write;
      iovs[1].iov_base = post_data_buf;
      iovs[1].iov_len = strlen(post_data_buf); 
      
      total_wrote = 0;
      amount_to_write += strlen(post_data_buf);
      /*do { 
        ret = write(sfd, post_data_buf + total_wrote, amount_to_write - total_wrote);
        if (ret > 0) total_wrote += ret; 
      } while (ret != 0 && total_wrote < amount_to_write);
      */
      total_wrote = writev(sfd, iovs, 2);
      if (total_wrote != amount_to_write) {
        printf("Failed to write all of post data %d of %d\n", total_wrote, amount_to_write);
        pthread_exit((void *)3);
      }
    } 

    // Read result
    char input_buf[2048];
    input_buf[0] = '\0';
    int total_read = 0;
    int content_length = -1;
    int read_it_all = 0;
    int read_bytes;
    int do_timeout = 0;
    do {
      fd_set reads;
      fd_set writes;
      FD_ZERO(&reads);
      FD_ZERO(&writes);
      if (read_bytes > 0) {
        FD_SET(sfd, &reads);
      }
      else {
        FD_SET(sfd, &reads);
      }
      struct timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      int retval = select(sfd+1, &reads, &writes, NULL, &timeout);
      if (retval == 0) {
        printf("Timeout\n");
        do_timeout = 1;
        break;
      }
      if (FD_ISSET(sfd, &reads)) {
        read_bytes = read(sfd, input_buf + total_read, sizeof(input_buf) - total_read);
      }
      if (read_bytes > 0) {
        total_read += read_bytes;
        input_buf[read_bytes] = '\0';
        // Look for Content-Length
        if (content_length < 0) {
          char *cl = strstr(input_buf, "Content-Length:");
          if (cl) {
            cl += strlen("Content-Length: ");
            char *eol = strstr(cl, "\r\n");
            if (eol) {
              eol[0] = '\0';
              content_length = atoi(cl);
              eol[0] = '\r';
              char *eoh = strstr(cl, "\r\n\r\n");
              if (eoh) {
                // EOH is here
                total_read -= (eoh+4) - input_buf;
                //printf("Content-Length: %d and total read = %d\n", content_length, total_read);
              }
            } 
          } 
        }
      }
      if (content_length == total_read) {
        read_it_all = 1;
        break;
      }
    } while (!do_timeout && read_bytes != 0 && (content_length < 0 || total_read < content_length));

    if (total_read >= 0 && total_read < 2048) input_buf[total_read] = '\0';
    else input_buf[2048] = '\0';
    //printf("total_bytes=%d Received bytes=%d\n", total_read, read_bytes);
    if (!do_timeout && !read_it_all) {
      struct sockaddr_in local_addr;
      int addr_size = sizeof(local_addr);
      getsockname(sfd, &local_addr, &addr_size);
      printf("Thread failure, only read %d bytes method=%s client_port=%d rand=%d %d of %d\n", total_read, tinfo->do_get ? "GET" : "POST", ntohs(local_addr.sin_port), num, i, tinfo->count);
      printf("content_length=%d read_bytes=%d\n", content_length, read_bytes);
      close(sfd);
      pthread_exit(1);
    } 
    else if (do_timeout) {
       // break out and start a new connection
       break;
    }
    //sleep(10);
  }

  
  close(sfd);
}
  pthread_exit(NULL);
}

/** 
 * Connect to a server.
 * Handshake
 * Exit immediatesly
 */
int
main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, j;
    size_t len;
    ssize_t nread;

   if (argc < 4) {
        fprintf(stderr, "Usage: %s host thread-count client-count\n", argv[0]);
        exit(EXIT_FAILURE);
    }
   host = argv[1];

   int i;
   for (i = 0; i < 1024; i++) {
     post_data_buf[i] = 'a';
   }
   post_data_buf[i] = '\0';
   strcat(post_data_buf, "&no_data=\r\n\r\n");

   snprintf(req_buf, sizeof(req_buf), "GET /data_block HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", host);
   snprintf(post_req_buf, sizeof(post_req_buf), "POST /post_data.php HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Length: %d\r\n\r\n", host, strlen(post_data_buf));

   int thread_count = atoi(argv[2]);
   int client_count = atoi(argv[3]);

   /* Obtain address(es) matching host/port */

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
   hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
   hints.ai_flags = 0;
   hints.ai_protocol = 0;          /* Any protocol */

   s = getaddrinfo(host, "80", &hints, &result);
   if (s != 0) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
       exit(EXIT_FAILURE);
   }

   /* getaddrinfo() returns a list of address structures.
    * Try each address until we successfully connect(2).
    * socket(2) (or connect(2)) fails, we (close the socket
      and) try the next address. */

   for (rp = result; rp != NULL; rp = rp->ai_next) {
     sfd = socket(rp->ai_family, rp->ai_socktype,
                  rp->ai_protocol);
     if (sfd == -1)
       continue;
     if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
        break;                  /* Success */

   }
   close(sfd);

   if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
   }


  struct thread_info *tinfo;
  tinfo = (struct thread_info*)malloc(thread_count*sizeof(struct thread_info));
  pthread_t * threads = malloc(thread_count *sizeof(pthread_t));
  srand(time(NULL));
  for (i= 0; i < thread_count; i++) {
    tinfo[i].rp =rp;
    tinfo[i].count = client_count;
    float percent = (float)rand() / (float)RAND_MAX;
    //tinfo[i].do_get = (percent < 0.80);
    tinfo[i].do_get = 1;
    pthread_create(threads + i, NULL, spawn_same_session_send, tinfo + i);
  }

  void *retval;
  for (i = 0; i < thread_count; i++) {
    retval = NULL;
    if (!pthread_join(threads[i], &retval)) {
      if (retval != NULL) {
        printf("Thread %d failed 0x%x\n", i, retval);
      }
    }
  }

  //printf("All threads finished\n");
 
  exit(0);
}

