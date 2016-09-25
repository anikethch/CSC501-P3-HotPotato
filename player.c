#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#define LEN  5000

struct info {
   int id;
   int left_id;
   int left_port;
   char left_hostname[LEN];
   int right_id;
   int left_fd;
   int right_fd;
   int master;
} info;
int max(int a,int b) {
    if (a>b) return a;
    else return b;
}

int get_random() {
    int  r;
   /* time_t z;
    srand((unsigned)time(&z));*/
    r=rand()%2;
    return r;
}

int get_hops(char *potato) {
    int i_hops;
    char *hops;
    char *dup_pot;
    dup_pot  = malloc(LEN);
    strcpy(dup_pot,potato);
    hops=malloc(LEN);
    hops = strtok(dup_pot,":");
    i_hops=atoi(hops);
    i_hops--;
    return i_hops;
}

char *modify_potato(char *potato,int id) {
     //printf("potato recieved is %s\n",potato);
     char pot[LEN];
     char s_id[LEN];
     char hops[LEN];
     char trace[LEN];
     int i_hops;
     sprintf(s_id,"%d",id);
     strcpy(hops,"");
     strcpy(trace,"");
     strcpy(hops,strtok(potato,":"));
     //printf("hops is  %s\n",hops);
     i_hops = atoi(hops);
     i_hops--;
     strcpy(hops,strtok(NULL,":"));
     strcpy(trace,hops);
     sprintf(hops,"%d",i_hops);
     strcat(trace,",");
     strcat(trace,s_id);
     strcpy(pot,hops);
     strcat(pot,":");
     strcat(pot,trace);
     return pot;
}

main (int argc, char *argv[]) {
  if (argc !=3) {
    fprintf(stderr,"Usage is %s <master-machine-name> <port-number>\n",argv[0]);
    exit(1);
  }
  int master_port,port,master_socket,sock,right_sock,len,n,my_id,temp_sock,left_socket,hops,first_hop;
  char host[LEN], str[LEN],temp[LEN],temp_buf[LEN];
  char *buf;
  buf = malloc(LEN);
  struct hostent *server_addr;
  struct sockaddr_in server_sock,peer_sock,left_sock,right;
  struct info player_info;
  struct timeval tvl;
  fd_set readfds,writefds;
  char potato[6];
  char trace[5];
  time_t z;
  srand((unsigned)time(&z));
  strcpy(potato,"potato");
  strcpy(trace,"trace");
  //printf("%s\n",potato);
  master_port = atoi(argv[2]);

  /* fill in hostent struct */
  server_addr= gethostbyname(argv[1]);
  if ( server_addr == NULL ) {
    fprintf(stderr, "host not found (%s)\n", argv[1]);
    exit(1);
  }

  /* use address family INET and STREAMing sockets (TCP) */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if ( sock < 0 ) {
    perror("socket:");
    exit(sock);
  }

  /* set up the address and port */
  server_sock.sin_family = AF_INET;
  server_sock.sin_port = htons(master_port);
  memcpy(&server_sock.sin_addr,server_addr->h_addr_list[0], server_addr->h_length);

  /* connect to socket at above addr and port */
  master_socket= connect(sock, (struct sockaddr *)&server_sock, sizeof(server_sock));
  if ( master_socket < 0 ) {
    perror("connect:");
    exit(master_socket);
  }
 
  player_info.master = sock;

  right_sock = socket(AF_INET, SOCK_STREAM, 0);
  if ( right_sock < 0 ) {
    perror("socket:");
    exit(right_sock);
  }
  
  for (port=1024;port<=65535;port++) {
      peer_sock.sin_family = AF_INET;
      peer_sock.sin_port = htons(port);
      peer_sock.sin_addr.s_addr = htonl(INADDR_ANY);

      n = bind(right_sock,(struct sockaddr *)&peer_sock,sizeof(peer_sock));
      if (n>=0) {
          //printf("port number is %d\n",port);
          break;
      }
  }
  
  int rc;
  rc =  listen(right_sock,1);
  if (rc<0) {
     perror("listen");
     exit(rc);
  } 

  n = send(sock,&port,sizeof(int),0);
  if (n < 0)  {
     perror("error in writing to socket");
     exit(n);
  }  
  
  n = recv(sock,&player_info.id,sizeof(int),0);
  if (n<0) {
     perror("error in reading from socket");
     exit(n);
  }

  printf("Connected as player %d\n",player_info.id);

  /*n = recv(sock,&player_info.left_id,sizeof(int),0);
  if (n<0) {
     perror("error in reading from socket");
     exit(n);
  }*/
  //printf("my left neighbor id is %d\n",player_info.left_id);

 /* n = recv(sock,&player_info.left_port,sizeof(int),0);
  if (n<0) {
     perror("error in reading from socket");
     exit(n);
  }*/
  //printf("my left neighbor port is %d\n",player_info.left_port);

  /*n = recv(sock,&player_info.right_id,sizeof(int),MSG_DONTWAIT);
  if (n<0) {
     perror("error in reading from socket");
     exit(n);
  }*/
  //printf("my right neighbor id is %d\n",player_info.right_id);

  /*n=recv(sock,host,64,0);
  host[n]="\0";
  strcpy(player_info.left_hostname,host);
  
  if (n<0) {
     perror("error in reading from socket");
     exit(n);
  }*/
  //printf("my left neighbor hostname is %s\n",host);
  n = recv(sock,buf,LEN*sizeof(char),0);
  if (n<0) {
     perror("error in reading from socket");
     exit(n);
  }
  strcpy(temp_buf,buf);
  strcpy(temp,strtok(temp_buf,"::")); 
  player_info.left_id = atoi(temp);
  strcpy(temp,strtok(NULL,"::"));
  player_info.left_port=atoi(temp);
  strcpy(temp,strtok(NULL,"::"));
  player_info.right_id=atoi(temp); 
  strcpy(temp,strtok(NULL,"::"));
  strcpy(player_info.left_hostname,temp);

  /*printf("my left neighbor id is %d\n",player_info.left_id);
  printf("my left neighbor port is %d\n",player_info.left_port);
  printf("my right neighbor id is %d\n",player_info.right_id);
  printf("my left neighbor hostname is %s\n",player_info.left_hostname);*/

  player_info.left_fd = 0;
  player_info.right_fd =0;

  int left_peer;
  left_peer = socket(AF_INET,SOCK_STREAM,0);
  if (left_peer<0) {
     perror("socket");
     exit(left_peer);
  }  
  struct hostent *left_peer_info;
  left_peer_info = gethostbyname(player_info.left_hostname);
  if (left_peer_info==NULL) {
     fprintf(stderr,"%s:host not found \n",player_info.left_hostname);
     exit(1);
  } 
  left_sock.sin_family=AF_INET;
  left_sock.sin_port=htons(player_info.left_port);
  memcpy(&left_sock.sin_addr,left_peer_info->h_addr_list[0],left_peer_info->h_length);

  int maxfd,s;
  maxfd = max(sock,right_sock);
  tvl.tv_sec = 20;
  tvl.tv_usec = 0;

  int start;
  start=1;
  first_hop=0;
  while(1) {
    if(player_info.right_fd && player_info.left_fd) break;       
    FD_ZERO(&readfds);
    FD_SET(sock,&readfds);
    FD_SET(right_sock,&readfds);
    s=select(maxfd+1,&readfds,NULL,NULL,&tvl);

    if (s<0) {
       perror("select");
       exit(1);
    } else if (s==0) {
       fprintf(stderr,"timeout\n");
       exit(0);
    } else {
      if (player_info.left_fd>0 && player_info.right_fd>0) 
          break;
        if (start && (FD_ISSET(sock,&readfds)>0)) {
          start=0;
          n = recv(sock,&hops,sizeof(int),0);
          if (n>0) {
              if(hops==0) {
                close(sock);
                exit(0);
              }
              first_hop=1;
              //printf("recieved potato from master\n");
              //printf("hops recieved is %d\n",hops);
              if (player_info.left_fd==0) {
                 int x;
                 x= connect(left_peer, (struct sockaddr *)&left_sock, sizeof(left_sock)); 
                 if (x<0) {
                    perror("connect");
                    exit(x);
                 }
                 player_info.left_fd=left_peer;
                 //printf("connected to my left peer\n");
              }
          } else {
             continue;
          }
      } else {
          if (player_info.right_fd==0) {
             int x,y;
             y=sizeof(peer_sock);
             x = accept(right_sock,(struct sockaddr *)&right,&y);
             if (x<0) {
                perror("accept");
                exit(x);
             }
             player_info.right_fd = x;
             //printf("connected to my right neighbor\n");
          }
          if (player_info.left_fd==0) {
             int x;
             x= connect(left_peer, (struct sockaddr *)&left_sock, sizeof(left_sock));
             if (x<0) {
                 perror("connect");
                 exit(x);
             }
             player_info.left_fd=left_peer;
             //printf("connected to my left peer\n");
          }
       }
    }
  }
 //printf ("i came here\n");
 int  exit_flag;
 exit_flag=0;
 while(1) {
  if (!exit_flag) {
   if (first_hop) {
    //printf("im the first hop\n");
    int x,y;
    hops--;
    first_hop=0;
    if (hops==0) {
       printf("I'm it\n");
       char s_hops[64],pot[64],s_id[64],buf[1024];
       sprintf(s_hops,"%d",hops);
       strcpy(buf,s_hops);
       strcat(buf,":");
       sprintf(s_id,"%d",player_info.id);
       strcat(buf,s_id);
       x = send(player_info.master,buf,LEN*sizeof(char),0);
       if  (x<0) {
           perror("Error in writing to socket");
           exit(x);
       }
       //printf("buf sent is %s\n",buf);
       break;
    } else { 
       char s_hops[64],pot[64],s_id[64];
       sprintf(s_hops,"%d",hops);
       strcpy(pot,s_hops);
       strcat(pot,":");
       sprintf(s_id,"%d",player_info.id);
       strcat(pot,s_id);
       strcpy(buf,pot);
       y=get_random();    
       if (y) {
          printf("Sending potato to %d\n",player_info.right_id);
          x=send(player_info.right_fd,buf,LEN*sizeof(char),0);
          if (x<0) {
             perror("Error in writing to socket");
             exit(x); 
          }
       } else {
          printf("Sending potato to %d\n",player_info.left_id);
          x=send(player_info.left_fd,buf,LEN*sizeof(char),0);
          if (x<0) {
             perror("Error in writing to socket");
             exit(x); 
          }
       }
    }

       //printf("buf sent is %s\n",buf);
  } else {
    //printf("nope im not the first hop\n");
    maxfd = max(player_info.right_fd,player_info.left_fd);
    maxfd = max(maxfd,player_info.master);
    //while(1) {
       int x,y,i,h;
       int recv_p;
       recv_p=0;
       FD_ZERO(&readfds);
       FD_SET(player_info.left_fd,&readfds);
       FD_SET(player_info.right_fd,&readfds); 
       FD_SET(player_info.master,&readfds);
       x= select(maxfd+1,&readfds,NULL,NULL,&tvl);
       if (x<0) {
          perror("Select");
          exit(x);
       } else if (x==0) {
          fprintf(stderr,"timeout in select\n");
          exit(0);
       } else {
          if (FD_ISSET(player_info.master,&readfds)&&recv(player_info.master,buf,LEN*sizeof(char),0)>0) {
               //printf("I'm exiting now\n");
               exit_flag=1;
          } else if (FD_ISSET(player_info.left_fd,&readfds)&&recv(player_info.left_fd,buf,LEN*sizeof(char),0)>0) {
             hops = get_hops(buf);
             recv_p=1;
             //printf("recieved potato from player %d\n",player_info.left_id);
          } else if (FD_ISSET(player_info.right_fd,&readfds)&&recv(player_info.right_fd,buf,LEN*sizeof(char),0)>0) {
             hops = get_hops(buf);
             recv_p=1;
             //printf("recieved potato from player %d\n",player_info.right_id);
          } 
       }
       if (recv_p) {
       if (hops>0) {
           y=get_random();   
           buf = modify_potato(buf,player_info.id);
           if (y) {
               printf("Sending potato to %d\n",player_info.right_id);
               x=send(player_info.right_fd,buf,LEN*sizeof(char),0);
               if (x<0) {
                  perror("Error in writing to socket");
                  exit(y);
               }
   //            break;
           } else {
               printf("Sending potato to %d\n",player_info.left_id);
               x=send(player_info.left_fd,buf,LEN*sizeof(char),0);
               if (x<0) {
                  perror("Error in writing to socket");
                  exit(y);
               }
     //          break;
           }
           //printf("sending potato is %s\n",buf);
       } else {
           printf("I'm it\n");
           buf = modify_potato(buf,player_info.id);
           //printf("sending buf to master %s\n",buf);
           x = send(player_info.master,buf,LEN*sizeof(char),0);
           if (x<0) {
               perror("Error in writing to socket");
               exit(y);
           }
           exit_flag=1;
           break;
       }
     }
    //}
   }
  } else break;
 }
 close(sock);
 close(right_sock);
 close(left_peer);
}
