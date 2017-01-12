// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Kaixin Ma

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int main(int argc, char *argv[]){
    char *host =  argv[1];
    int port  = atoi(argv[2]);

    int sfd;
    char sendbuff[256];
    struct sockaddr_in sin;
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      perror("socket");
      exit(1);
    } 
    struct hostent *server = gethostbyname(host);
    if (server == NULL) {
        perror("server");
        exit(1);
    }
    sin.sin_family = AF_INET;
    sin.sin_port = port;  
    memcpy(&sin.sin_addr.s_addr, server->h_addr, server->h_length);   
    if (connect(sfd,(struct sockaddr *) &sin, sizeof(sin)) < 0){ 
      perror("connect");
       exit(1);       
    }   
   char word[10];
   strcpy(word, "report");
   if (argc > 3) {
   	    
    	if (strcmp(argv[3],"-k")==0){
    		
    		snprintf(sendbuff, 256, "K %s %li %d %d %d %d\n", word, (long)getpid(), 0, 0, 0, 0); 
    		write(sfd, sendbuff, 256);
    	}
    }else {
    	   
    	    snprintf(sendbuff, 256, "O %s %li %d %d %d %d\n", word, (long)getpid(), 0, 0, 0, 0); 
    		write(sfd, sendbuff, 256);
    }
    
     char rebuff[256];
      while(read(sfd, rebuff, sizeof(rebuff)) >0){
      	 if (strcmp(rebuff,"F") == 0){
      	 	printf("finish reporting \n");
      	 	break;
      	 }	
      	 
      }

    return 0;
    }