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
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/utsname.h>
#include <time.h>


char *localname;
int sfd;
char sendbuff[256];
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtxss = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t varss = PTHREAD_COND_INITIALIZER;
int pre;
int press;
int flag;
int start, end;
int threadcount;
int glaf;
sigset_t set;
int numthread;

void block(){
  int s = pthread_sigmask(SIG_BLOCK, &set, NULL);
  if (s!=0){
    perror("block");
  }
}

void status(int signum){
  flag = threadcount;
  
  sleep(5);
  
  close(sfd);
  if(flag == 0){    
  exit(1);
 }else {
  perror("flag");
  exit(3);
 }
      
}

void *sighandle()
{
   int sig;
   if (sigwait(&set, &sig)!=0){
    perror("sigwait");
    exit(1);
   }
   status(sig);
   return NULL;
}

void send_func(char *msg) {
        pthread_mutex_lock(&mtxss);
        while(press){
           pthread_mutex_unlock(&mtxss);
           pthread_mutex_lock(&mtxss);
        }
        memset(&sendbuff[0], 0, sizeof(sendbuff));
        strcpy(sendbuff, msg);
        press = 1; 
        pthread_cond_signal(&varss);   
        pthread_mutex_unlock(&mtxss);     
}

void requestrange(int t, int length){
        char lbuff[256];
        memset(&lbuff[0], 0, sizeof(lbuff));
        snprintf(lbuff, 256, "R %s %li %d %d %d %d\n", localname, (long)getpid(),t,length,0, numthread); 
        send_func(lbuff);
}

void findperfect(int k){
        char lbuff[256];
        memset(&lbuff[0], 0, sizeof(lbuff));
        snprintf(lbuff, 256,  "P %s %li %d %d %d %d\n", localname, (long)getpid(), k , 0, 0, numthread); 
        send_func(lbuff);
}

void deadnote(int k, int from, int to){
        char lbuff[256];
        memset(&lbuff[0], 0, sizeof(lbuff));
        snprintf(lbuff, 256, "S %s %li %d %d %d %d\n", localname, (long)getpid(), k, from, to, numthread); 
        send_func(lbuff);
}

void livenote(int k, int from, int to){
        char lbuff[256];
        memset(&lbuff[0], 0, sizeof(lbuff)); 
        snprintf(lbuff, 256, "B %s %li %d %d %d %d\n", localname, (long)getpid(), k, from, to, numthread);     
        send_func(lbuff);  
}

int decode(char buff[]){
  char type;
  int a, b;
  sscanf(buff, "%c %d %d", &type, &a, &b); 
  if (type == 'R'){
    start = a;
    end = b;
    return 0;
  }
  if (type == 'D'){
      glaf = threadcount;
    return 1;
  }
   return 1;
}

void *sendmesg(void *sp)

{
    block();   
    pthread_mutex_lock(&mtxss); 
    while(1){
   // printf("send lock mtxss\n");
    pthread_cond_wait(&varss, &mtxss); 
    press = 0;    
    printf("sendbuff content %s\n",sendbuff);       
    if (write(sfd, sendbuff , strlen(sendbuff)) == -1){
      perror("send write");
    }
    memset(&sendbuff[0], 0, sizeof(sendbuff));
    printf ("compute sending %s \n",sendbuff );
  }
    pthread_mutex_unlock(&mtxss);
    return NULL;
}


void *receive(void *rp)
{
    block();
    char buff[256];
    while(read(sfd, buff, sizeof(buff)) >0){
        printf("compute reading %s\n", buff);
        pthread_mutex_lock(&mtx);
        while(pre){
           pthread_mutex_unlock(&mtx);
           pthread_mutex_lock(&mtx);
        }    
        int com = decode(buff); 
        if (com == 0){
           pre = 1;
           pthread_cond_signal(&var);
        }
        pthread_mutex_unlock(&mtx);
      }
      return NULL;
}

void *perfect(){   
   block();
   int i, sum;   
   int k;
   int from, to;
   int tid;
   tid = (int)pthread_self();
   int spend=0, length=0;
   while(1){
      pthread_mutex_lock(&mtx);   
      requestrange(spend,length);
      do {
          int test = pthread_cond_wait(&var, &mtx);
          if (test == 0){
              printf("compute %d released  !!!!!!!!!!!!!!! \n", tid);
            }else if (test == ETIMEDOUT){
                   printf("compute %d timed out  |||||||||||||||| \n", tid);
            }else{
                printf("compute %d error we dont know  ?????????????????? \n", tid);
            }
      }
      while(!pre);    
      from = start;
      to = end;
      pre = 0;
      pthread_mutex_unlock(&mtx);
      printf("compute %d %d to %d \n", tid, from, to); 
      time_t born = time(0);  
      for(k=from;k<to;k++){ 
          if (flag > 0 ){
              printf("thread %d is ready to die\n", tid);
              deadnote(k,from, to);
              flag--;
              pthread_exit(&k);
          }
          if (glaf > 0){
              livenote(k, from, to);
              glaf -= 1;
          }
          sum = 1;
          for (i=2;i<=k/2;i++){
               if (!(k%i)) {
                  sum+=i;  
               }  
          }
          if (sum == k && k != 1) {
               printf("%d is perfect\n", k); 
               findperfect(k);
          }
      }
      time_t die = time(0);
      spend = die - born;
      length = to - from;
      printf("this thread took %d seconds\n", spend); 
  }
  return NULL;   
}


int main(int argc, char *argv[]){
    char *host =  argv[1];
    int port  = atoi(argv[2]);
    numthread = atoi(argv[3]);
    pthread_t pdt[numthread];
    pthread_t recv, send, sig;
    threadcount = numthread;
    char tempname[128];
    tempname[127] = '\0';
    gethostname(tempname, 127); 
    localname = malloc(128);
    strcpy(localname, tempname);
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
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);
    block();
    flag = 0;
    glaf = 0;
    pthread_create(&send, NULL, &sendmesg, NULL);
    pthread_create(&recv, NULL, &receive, NULL);
    if(pthread_create(&sig, NULL, &sighandle, NULL) != 0){
      perror("signal thread");
      exit(1);
    }
    int i;  
    pre = 0; // no work to consume
    press = 0;  // 1 means work to do
    for (i = 0;i<numthread; i++){
        
       if(pthread_create(&pdt[i], NULL, &perfect, NULL) != 0){
             perror("compute thread");
	        exit(1);                                        
                  
           }      
}
    pthread_join(recv, NULL);
    pthread_join(send, NULL);
    pthread_join(sig, NULL);
   for (i = 0;i<numthread; i++){
      pthread_join(pdt[i], NULL);
}
	return 0;
}

