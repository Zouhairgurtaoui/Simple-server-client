#include "chatHeader.h"
#include <signal.h>


struct users
{
    int id;
    int sfd;
    char ip[INET6_ADDRSTRLEN];
    char name[NAME_LEN];
    char time_log[TIME];
    
};
typedef struct client{
   struct client* nextUser;
   struct users user;
}user_t;

pthread_mutex_t userm;
int uid;
user_t *head;

struct addrinfo *insialization();

int creating_binding_listenning_sock(struct addrinfo* );

struct users accepting(user_t*,int );


void addUser(user_t *);

void remUser(int);

void send_all_users(int,char*);

void* handle_user(void*);

int main(){

    
    struct addrinfo *res;
    res=insialization();
    int sfd ;
    sfd=creating_binding_listenning_sock(res); 
    head=NULL;
    uid=0;
    pthread_mutex_init(&userm,NULL);
    pthread_t ult;
    while(1){
        user_t* cli =(user_t*)malloc(sizeof(user_t));
        accepting(cli,sfd);
        addUser(cli);
        pthread_create(&ult,NULL,&handle_user,(void*)cli);
        
        sleep(1);
    }

}


struct addrinfo *insialization(){
    struct addrinfo  *res,hints;
    int stat;

    memset(&hints,0,sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((stat = getaddrinfo(NULL,PORT,&hints,&res)) != 0){
        fprintf(stderr,"getaddrinfo %s",gai_strerror(stat));
        exit(1);
    }

    return res;

}

int creating_binding_listenning_sock(struct addrinfo* p){
 
    int sfd;
    int yes = 1;

    for(;p != NULL;p=p->ai_next){
        if((sfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) < 0){
            perror("creating soccket");
            continue;
        }
        signal(SIGPIPE, SIG_IGN);
        if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1){
            perror("setsocket option");
            continue;
        }
        if(bind(sfd,p->ai_addr,p->ai_addrlen) < 0){
            perror("binding");
            continue;
        }
        if(listen(sfd,BACKLOG) < 0){
            perror("listenning");
            continue;
        }
        break;

    }

    if(!p){
        perror("something went wrong");
        close(sfd);
        exit(1);
    }
    freeaddrinfo(p);
    return sfd;
}
void* get_addr(struct sockaddr* sa){
    if(sa->sa_family == AF_INET)return &((struct sockaddr_in*)sa)->sin_addr;
    else return &((struct sockaddr_in6*)sa)->sin6_addr;
}
struct users accepting(user_t* user,int sfd){

    struct sockaddr_storage userinfo;
    
    socklen_t sin_size=sizeof(userinfo);
    int newsfd;
    
    if((newsfd=accept(sfd,(struct sockaddr*)&userinfo,&sin_size)) < 0){
        perror("accepting");
        close(sfd);
        exit(1);
    }
    uid++;
    user->user.id = uid;
    user->user.sfd = newsfd;
    inet_ntop(((struct sockaddr*)&userinfo)->sa_family,get_addr((struct sockaddr*)&userinfo),user->user.ip,sizeof(user->user.ip));
    time_t t;
    time(&t);
    strncpy(user->user.time_log,ctime(&t),sizeof(user->user.time_log));
    printf("server got connection from %s at %s",user->user.ip,user->user.time_log);
    
    
    return user->user;
}
void addUser(user_t* user){
   	pthread_mutex_lock(&userm);
        user->nextUser=head;
        head=user;
        pthread_mutex_unlock(&userm);
}
void remUser(int UserId){
    pthread_mutex_lock(&userm);
    user_t* temp = head;
    user_t* prev = NULL;
    

    if (temp == NULL) {
        pthread_mutex_unlock(&userm);
        return;
    }

    if (temp->user.id == UserId) {
        head = temp->nextUser;
        free(temp);
        pthread_mutex_unlock(&userm);
        return;
    }

    while (temp != NULL) {
        if (temp->user.id == UserId) {
            prev->nextUser = temp->nextUser;
            free(temp);
            temp = NULL; 
            break;
        }
        prev = temp;
        temp = temp->nextUser;
    }

    pthread_mutex_unlock(&userm);
}



void* handle_user(void* arg){
    user_t* user= (user_t*)arg;
    char buff_out[MAX_BUFFER];
    char buff_in[MAX_BUFFER/2];
    
    char name[NAME_LEN];
    int stat;
    char WelMess[]="<---- Welcome to the chatroom---->\n";
    
    if(send(user->user.sfd,WelMess,35,0) <= 0){
        fprintf(stderr,"error can't send WelMess\n");
    }
    if((stat=recv(user->user.sfd,name,NAME_LEN,0) <= 0)){
         if(send(user->user.sfd,"invalid name try again\n",23,0) <= 0)
        fprintf(stderr,"error can't send message name\n");
    }
    name[NAME_LEN]='\0';
    remLn(name);
    strncpy(user->user.name,name,sizeof(user->user.name));
    printf("user %d with %s connected as %s\n",user->user.id,user->user.ip,user->user.name);

    sprintf(buff_out,"%s joined the chat\n",user->user.name);
    printf("%s ",buff_out);
    send_all_users(user->user.id,buff_out);
    memset(buff_out,0,sizeof buff_out);
    while((stat = recv(user->user.sfd,buff_in,sizeof(buff_in) - 1,0)) > 0 ){
        buff_in[stat]='\0';
        remLn(buff_in);
        if (!strlen(buff_in)) {
            continue;
        }else
        if(!strcmp(buff_in,"exit"))
        {
            break;
        }
        
        sprintf(buff_out,"[%s] : %s\n",user->user.name,buff_in);
        printf("%s",buff_out);
        send_all_users(user->user.id,buff_out);
        memset(buff_out,0,sizeof buff_out);
        
    }
    sprintf(buff_out,"%s has left the chat\n",user->user.name);
    printf("user %d connected as %s with ip %s has left\n",user->user.id,user->user.name,user->user.ip);
    send_all_users(user->user.id,buff_out);
    close(user->user.sfd);
    remUser(user->user.id);
    pthread_detach(pthread_self());

}
void send_all_users(int id,char* message){
    pthread_mutex_lock(&userm);
    user_t *temp = head;
    while(temp != NULL){
        if(temp->user.id != id){
        if(send(temp->user.sfd,message,strlen(message),0) == 0){
            perror("sending message");
            break;
        }
        }
        temp=temp->nextUser;
    }
    pthread_mutex_unlock(&userm);
}
