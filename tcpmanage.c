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
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <signal.h>

#define MAXCOM 100
#define DEFAULTLEN 100000
int reportid = 0;
int reporting = 0;
int reportcount = 0;
struct pollfd list[100];

struct seg{
	int start;
	int end;
	struct seg *next;
} * head=NULL;


struct found 
{
	char name[128];
	long pid;
	int num;
	int fd;
	int thread;
};

struct perfect
{
	char name[128];
	long pid;
	int num;
};

struct found fff[100];
struct perfect ppp[10];

void store(char name[], long pid, int num){
	int i;
	
	for (i=0;i<10;i++){
		if (ppp[i].num == 0){
			ppp[i].num = num;
			
			memcpy(ppp[i].name, name, 128);   
			ppp[i].pid = pid;
			break;
		}
	}
}

void update(char name[], long pid, int fd, int tested, int thr){
	int i;
	for (i=0; i<100; i++){
		if(fff[i].fd == fd){
			
			fff[i].thread = thr;
			fff[i].pid = pid;
			strcpy(fff[i].name, name);
			
		}
	}

}

void updateversion(char name[], long pid, int fd, int tested, int thr){
	int i;
	for (i=0; i<100; i++){
		if(fff[i].fd == fd){
			fff[i].num += tested;
			fff[i].thread = thr;
			fff[i].pid = pid;
			strcpy(fff[i].name, name);
			
		}
	}

}


int w = 0, h = 0;
void findrange(int l){
   struct seg *p;
	p = head;
	while(p->next != NULL){
		if (p->end == p->next->start){
			p = p->next;
	  }else{
	  	w = p->end;
	  	h = p->next->start;
	  	struct seg *temp;
	    temp = (struct seg*)malloc(sizeof(struct seg));
	    temp->start = p->end;
	    temp->end = p->next->start;
	    temp->next = p->next;
	    p->next = temp;
	    return;
	  }
	}
	struct seg *temp;
	temp = (struct seg*)malloc(sizeof(struct seg));
	temp->start = p->end;
	if (l == 0){
	temp->end = temp->start+DEFAULTLEN;
    }else {
    temp->end = temp->start+l;	
    } 
	p->next = temp;
	temp->next = NULL;
	w = temp->start;
	h = temp->end;
	return;
}


void giverange(char name[], long pid, int fd, int a, int b, int d){
				char range[256];
				struct seg *p;	
				if (a == 0){		
				if(head == NULL){
					p = (struct seg*)malloc(sizeof(struct seg));
					p ->start = 1;
					p ->end = DEFAULTLEN;
					p ->next = head;
					head = p;
					
					sprintf(range, "R %d %d\n", p->start, p->end); 
					update(name, pid, fd, p->end-p->start, d);  
					write(fd, range, 256);
					
				}else{
					findrange(0);
					
					
					sprintf(range, "R %d %d\n", w, h); 
					update(name, pid, fd, h-w, d); 
					write(fd, range, 256);  
					
				}
			}else {
				    int length = (15*b)/a;
					findrange(length);
					
					sprintf(range, "R %d %d\n", w, h); 
					update(name, pid, fd, b, d); 
					write(fd, range, 256);  
					
			}

}

void regularreport(int fd){
	if (reporting == 1){
		perror("already reporting ");
		exit(1);
	}
	reportid = fd;
	reporting = 1;
	int i;
	char info[256];
	for(i=0;i<10;i++){
		if(ppp[i].num != 0){
		   memset(&info[0], 0, sizeof(info));
           sprintf(info, "Host: %s found perfect number %d\n", ppp[i].name, ppp[i].num); 
           write(reportid, info, 256);  
		}
	}	
	char detail[256];	
			for(i=1;i<100;i++){
		        if (list[i].fd == fd){		  	        
		  	        fff[i].fd = -1;		  			
		  		 //	break;
		  		}

		  		if (fff[i].fd != -1) {
		  			memset(&detail[0], 0, 256);
		  			sprintf(detail, "Host: %s has tested %d numbers\n", fff[i].name, fff[i].num); 
		  			write(reportid, detail, 256); 
		  			reportcount += fff[i].thread;
		  			memset(&info[0], 0, sizeof(info));
		  			sprintf(info, "D %d %d\n", 0, 0); 
					write(fff[i].fd, info, 256);  
		  		}
			}

}

void givereport(char name[], long pid, int a, int b, int c){
    char info[256];
    memset(&info[0], 0, sizeof(info));
    sprintf(info, "Host: %s pid: %li working from %d to %d is going to test %d\n", name, pid,  b, c, a); 
    write(reportid, info, 256);  
     reportcount -= 1;
     if (reportcount == 0){
     	write(reportid, "F\0\n", 3);
     	reporting = 0;
     	reportid = 0;
     }
}

void killthem(int fd){
	if (reporting == 1){
		perror("already reporting ");
		exit(1);
	}
	reportid = fd;
	reporting = 1;
	int i;
	char info[256];
	for(i=0;i<10;i++){
		if(ppp[i].num != 0){
			memset(&info[0], 0, sizeof(info));
           sprintf(info, "Host: %s found perfect number %d\n", ppp[i].name, ppp[i].num); 
           write(reportid, info, 256);  
		}
	}
		//char detail[256];
			for(i=1;i<100;i++){
		        if (list[i].fd == fd){		  	        
		  	        fff[i].fd = -1;		  			
		  		 //	break;
		  		}
		  		if (fff[i].fd != -1) {
		  		/*	memset(&detail[0], 0, sizeof(detail));
		  			sprintf(detail, "Host: %s pid %li has tested %d numbers\n", fff[i].name, fff[i].pid, fff[i].num); 
		  			write(reportid, detail, 256); */
		  			reportcount += fff[i].thread;
		  			kill(fff[i].pid, SIGINT);
					//write(fff[i].fd, info, 256);  
		  		}
			}

}

void deadupdate(char name[], long pid, int fd, int a, int b, int c){
     int i;
     for (i=1; i<100; i++){
     	if (fff[i].fd == fd){
     		fff[i].num += (a-b);
             break;
     	}    	
     }
    struct seg *p;
	p = head;
	if (p->start == b){
			p->end = a;
			return;
	  }
	while(p->next != NULL){
		p = p->next;
		if (p->start == b){
			p->end = a;
			break;
	  }	   
	}
}

void deadreport(char name[], long pid, int a, int b, int c){
    char info[256];
    memset(&info[0], 0, sizeof(info));
    sprintf(info, "Host: %s pid: %li working from %d to %d is going to test %d\n", name, pid,  b, c, a); 
    if (reporting != 1){
    	perror("not reporting");	
    	return;
    }
    write(reportid, info, 256);  
     reportcount -= 1;
     if (reportcount == 0){
         char detail[256];
         int i;
			for(i=1;i<100;i++){	       
		  		if (fff[i].pid == pid) {
		  			memset(&detail[0], 0, sizeof(detail));
		  			sprintf(detail, "Host: %s pid %li has tested %d numbers\n", fff[i].name, fff[i].pid, fff[i].num); 
		  			write(reportid, detail, 256); 
		  			break;
		  		}
			}
     	write(reportid, "F\0\n", 3);
     	reporting = 0;
     	reportid = 0;
     	exit(1);
     }
}

void decode(char buff[], int fd){
  char type;
  int a, b, c, d;
  char name[128];
  long pid;
  char *tok = strtok(buff,"\n");
  while (tok) {
  	 sscanf(tok, "%c %128s %li %d %d %d %d", &type, name, &pid, &a, &b, &c, &d); 
  if (type == 'R'){
      giverange(name, pid, fd, a, b, d);  	
  }
  if (type == 'P'){
  	 store(name, pid, a);
  }
  if (type == 'S'){
  	 deadupdate(name, pid, fd, a, b, c);
  	 deadreport(name, pid, a, b, c);   
  }
  if (type == 'B'){
  	 givereport(name, pid, a, b, c);
  }
  if (type == 'O'){
     regularreport(fd);
  }
  if (type == 'K'){
  	 killthem(fd);
  }
  	tok = strtok(NULL,"\n");
  }
 
}

int main(int argc, char *argv[]){


	struct sockaddr_in sin; /* socket address for destination */
	int s;
	int fd;
	
	sin.sin_family= AF_INET;
	sin.sin_addr.s_addr= INADDR_ANY;
	sin.sin_port = atoi(argv[1]);

	char hostname[128];
	hostname[127] = '\0';
	gethostname(hostname, 127);
	printf("Hostname: %s\n", hostname);
	

		/* Get an internet socket for stream connections */
	if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("Socket");
		exit(1);
		}

		/* Do the actual bind */
	if (bind (s, (struct sockaddr *) &sin, sizeof (sin)) <0) {
		perror("bind");
		exit(2);
		}
	
	listen(s,MAXCOM);
	
	
	int i;

	for(i=0;i<100;i++){
		list[i].fd = -1;
		fff[i].fd = -1;
	}
	list[0].fd = s;
	list[0].events = POLLIN;
	int len;
	
		/* Now loop accepting connections */
	
	while (1) {
		
		poll(list, 100, -1);
		
		if (list[0].revents & POLLIN){
		    if ((fd= accept (s, (struct sockaddr *) &sin,  &len)) <0) {
			    perror ("accept");
			    exit(3);
			}
			for(i=1;i<100;i++){
		        if (list[i].fd == -1){
		  	        list[i].fd = fd;
		  	        fff[i].fd = fd;
		  			list[i].events = POLLIN;
		  			break;
		  		}
			}
		}
		for(i=1;i<100;i++){
		 	if(list[i].fd == -1){
		 		continue;
		 	}else if (list[i].revents & POLLHUP){	
                close(list[i].fd);
                printf("manage: disconnected fd %d\n", list[i].fd);
                fff[i].fd = -1;
                list[i].fd = -1;
                list[i].events = 0;
		 	}else if (list[i].revents & POLLNVAL){	
                printf("manage: invalid fd %d\n", list[i].fd);
                fff[i].fd = -1;
                list[i].fd = -1;
                list[i].events = 0;
		 	}else if (list[i].revents & POLLERR){	
                printf("manage: error condition fd %d\n", list[i].fd);
                fff[i].fd = -1;
                list[i].fd = -1;
                list[i].events = 0;
		 	}else if (list[i].revents & POLLIN){
		 		
				char buff[1024];
				memset(&buff[0], 0, 1024);
			    read(list[i].fd, buff, 1024);
			    
			    decode(buff, list[i].fd);		
				
		 	}
		 	
		}
   	}
 	return 0;
}
