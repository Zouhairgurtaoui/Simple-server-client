#pragma once


#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>


#define MAX_BUFFER 1024
#define PORT "3490"
#define BACKLOG 10
#define NAME_LEN 50
#define TIME 60

void remLn(char* s){
   while (*s != '\0') {
        if (*s == '\r' || *s == '\n') {
            *s = '\0';
        }
        s++;
    }
  
}



