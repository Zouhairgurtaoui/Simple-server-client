#include "chatHeader.h"
#include <libgen.h>



void *receiving(void* args){
    int connfd=*(int*)args;
    char buff_in[MAX_BUFFER/2];
    int nbuff,n;
    while((nbuff=recv(connfd,buff_in,sizeof(buff_in)-1,0)) > 0){
        buff_in[nbuff]='\0';
        printf("%s",buff_in);
    }
   
}

int main(int argc,char* argv[]){


    if(argc < 2){
        fprintf(stderr,"ERROR: usage <./%s> <IP>\n",argv[0]);
        return 1;
    }
    struct addrinfo hint,*res;
    int stat,connfd;
    memset(&hint,0,sizeof hint);
    hint.ai_socktype=SOCK_STREAM;
    hint.ai_family=AF_UNSPEC;
    if((stat = getaddrinfo(argv[1],PORT,&hint,&res)) != 0){
        fprintf(stderr,"getaddrinfo %s",gai_strerror(stat));
        return 1;
    }
    if((connfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol)) < 0){
        perror("creating socket");
        return 2;
    }
    if(connect(connfd,res->ai_addr,res->ai_addrlen) == -1){
        perror("connecting to the server");
        return 2;
    }
    freeaddrinfo(res);

    char buff_out[MAX_BUFFER];
    char name[NAME_LEN];
    if((stat=recv(connfd,buff_out,sizeof(buff_out) - 1,0))<0){
            perror("receiiving");
        }
    buff_out[stat]='\0';
    puts(buff_out);
    memset(buff_out,0,sizeof buff_out);
    printf("\nEnter your Name\n");
    scanf("%s",&name);
    remLn(name);
    if(send(connfd,name,NAME_LEN,0) < 0){
        perror("sending name");
        return 3;
    }
    pthread_t clt;
    
    while(fgets(buff_out,MAX_BUFFER,stdin)){
        pthread_create(&clt,NULL,&receiving,(void*)&connfd);
        buff_out[strlen(buff_out) - 1]='\0';
        if(!strcmp(buff_out,"exit")){
            if(send(connfd,buff_out,sizeof(buff_out),0)<0){
                perror("sending");
            }
            break;
        }
        if(send(connfd,buff_out,sizeof(buff_out),0) < 0){
                perror("sending");
            }
        
    }
    close(connfd);


}
