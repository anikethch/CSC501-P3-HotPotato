#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <time.h>
#define LEN  5000

struct player {
   int id;
   int fd;
   int port;
   struct hostent *host_info;
} player;

int get_random(int players) {
    int  r;
    time_t z;
    srand((unsigned)time(&z));
    r=rand()%players;
    return r;
}

main (int argc, char *argv[]) {

  char buf[LEN],trace[LEN];
  char host[64];
  int n,s, player_sock, rc, len, port, players,hops;
  struct hostent *hp, *ihp;
  struct sockaddr_in server, client;

  if (argc !=4) {
    fprintf(stderr,"Usage is %s <port-number> <number-of-players> <hops>\n",argv[0]);
    exit(1);
  }
  port = atoi(argv[1]);
  players = atoi(argv[2]);
  hops = atoi(argv[3]);
  if (port > 65535) {
     fprintf(stderr,"Please  select port number less than 65535\n");
     exit(1);
  }

  if (players <2) {
     fprintf(stderr,"More than 1 player is required\n");
     exit(1);
  }

  if (hops<0) {
     fprintf(stderr,"Enter non-negative number for hops\n");
     exit(1);
  }

  /* fill in hostent struct for self */
  gethostname(host, sizeof host);
  hp = gethostbyname(host);
  if ( hp == NULL ) {
    fprintf(stderr, "host not found (%s)\n", host);
    exit(1);
  }

  /* use address family INET and STREAMing sockets (TCP) */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if ( s < 0 ) {
    perror("socket:");
    exit(s);
  }
  int enable =1;
  if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(int))<0) {
     perror("Error in setsockopt");
     exit(1);
  }

  /* set up the address and port */
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  memcpy(&server.sin_addr, hp->h_addr_list[0], hp->h_length);
  
  /* bind socket s to address server */
  rc = bind(s, (struct sockaddr *)&server, sizeof(server));
  if ( rc < 0 ) {
    perror("bind:");
    exit(rc);
  }

  rc = listen(s, players);
  if ( rc < 0 ) {
    perror("listen:");
    exit(rc);
  }

  printf("Potato Master on %s\n",host);
  printf("Players = %d\n",players);
  printf("Hops = %d\n",hops);

  /* accept connections */
  struct player player_list[players];
  for (n=0;n<players;n++) {
    len = sizeof(server);
    player_sock = accept(s, (struct sockaddr *)&client, &len);
    if ( player_sock < 0 ) {
      perror("accept:");
      exit(player_sock);
    }
    int i,client_port;
    i = recv(player_sock,&client_port,sizeof(int),0);
    if (i<0) {
       perror("Error in reading from socket\n");
       exit(i);
    }
    
    struct hostent *host_info_temp;
    host_info_temp = malloc(sizeof(struct hostent));
    player_list[n].host_info = malloc(sizeof(struct hostent));
    host_info_temp = gethostbyaddr((char *)&client.sin_addr,sizeof(struct sockaddr),AF_INET);
    if (host_info_temp==NULL) {
       fprintf(stderr,"Host can't be resolved\n");
       exit(1);
    }
    printf("Player %d is on %s\n",n,host_info_temp->h_name);
    //player_list[n].host_info = gethostbyaddr((char *)&client.sin_addr,sizeof(struct sockaddr),AF_INET);
    player_list[n].id = n;
    player_list[n].fd = player_sock;
    player_list[n].port = client_port;
    player_list[n].host_info = host_info_temp;
    //printf("Player %d is on %s\n",n,player_list[n].host_info->h_name);
    i = send(player_sock,&n,sizeof(int),0);
    if (i<0) {
       perror("Error in sending to socket");
       exit(i);
    }
  }

  for (n=0;n<players;n++) {
      int i,j,k;
      char temp[LEN];
      j=n+1;
      if (n==players-1) j=0 ;
       
      if (n==0) k=players-1;
      else k=n-1; 
      
      sprintf(temp,"%d",player_list[j].id);
      strcpy(buf,temp);
      strcat(buf,"::");
      sprintf(temp,"%d",player_list[j].port);
      strcat(buf,temp);
      strcat(buf,"::");
      sprintf(temp,"%d",player_list[k].id);
      strcat(buf,temp);
      strcat(buf,"::");
      strcat(buf,player_list[j].host_info->h_name); 
      strcat(buf,"\0");
      i = send(player_list[n].fd,buf,LEN*sizeof(char),0);
      if (i<0) {
         perror("Error in sending to socket");
         exit(i);
      }
      /*i =  send(player_list[n].fd,&player_list[j].id,sizeof(int),0);
      if (i<0) {
         perror("Error in sending to socket");
         exit(i);
      }

      i = send(player_list[n].fd,&player_list[j].port,sizeof(int),0);
      if (i<0) {
         perror("Error in sending to socket");
         exit(i);
      }
    
      if (n==0) k=players-1;
      else k=n-1; 
      i = send(player_list[n].fd,&player_list[k].id,sizeof(int),0);
      if (i<0) {
         perror("Error in sending to socket");
         exit(i);
      }

      char hostname[64];
      strcpy(hostname,player_list[j].host_info->h_name);
      strcat(hostname,"\0");
      i = send(player_list[n].fd,hostname,sizeof(hostname),0);
      if (i<0) {
         perror("Error in sending to socket");
         exit(i);
      }*/
  }
  int i;
  if (hops!=0) {
  n=get_random(players);
  i =  send(player_list[n].fd,&hops,sizeof(int),0);
  if (i<0) {
      perror("Error in sending to socket");
      exit(i);
  } 
  printf("All players present, sending potato to player %d\n",n);
  } else {
  for (n=0;n<players;n++) {
  i =  send(player_list[n].fd,&hops,sizeof(int),0);
  if (i<0) {
      perror("Error in sending to socket");
      exit(i);
  } 
  }
  close(s);
  exit(0);
  }

  fd_set readfds;
  struct timeval tvl;
  tvl.tv_sec = 30;
  tvl.tv_usec = 0;
  
  int x,maxfds;
  maxfds=0;

  int exit_flag;
  exit_flag =0;
  int trace_count,last_player;
  trace_count =0;
  while(1) {
    if (exit_flag) break;
    FD_ZERO(&readfds);
    for (n=0;n<players;n++) {
      maxfds=max(maxfds,player_list[n].fd);
      FD_SET(player_list[n].fd,&readfds);
    }

    int s;
    char *temp;
    s=select(maxfds+1,&readfds,NULL,NULL,&tvl);

    if (s<0) {
       perror("select");
       exit(s);
    } else if (s==0) {
       fprintf(stderr,"timeout on select");
    } else {
       for (x=0;x<players;x++) {
          if (FD_ISSET(player_list[x].fd,&readfds)&&recv(player_list[x].fd,buf,LEN*sizeof(char),0)>0) {
             last_player=x;
             exit_flag =1;
             break;
          } 
       }     
    } 
  }
     strtok(buf,":");
     strcpy(trace,strtok(NULL,":"));
     printf("Trace of potato:\n");
     printf("%s\n",trace);
     for (x=0;x<players;x++) {
         n=send(player_list[x].fd,"close",5*sizeof(char),0);
         if (n<0) {
            perror("Error in writing to socket");
            exit(n);
         }
     }
  close(s);
  exit(0);
}

int max(int a,int b) {
    if (a>b) return a;
    else return b;
}
