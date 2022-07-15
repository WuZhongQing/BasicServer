#include<iostream>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<signal.h>
#include <strings.h>
#include <string.h>
#include<ctype.h>
#include <sys/wait.h>
#include "Wrap.h"

#define PORT 6666
using namespace std;

void catch_child(int num){ //回收子进程
    while(waitpid(0, NULL, WNOHANG) > 0); // waits only for terminated children
    return ;
}

int main(int argc, char const *argv[])
{
    int lfd, cfd;
    pid_t pid;
    int ret;
    char buf[BUFSIZ];
    struct sockaddr_in serv_addr;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    // memset(&serv_addr, 0, sizeof(serv_addr));//将地址结构清零
    bzero(&serv_addr, sizeof(serv_addr));//同上
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);

    lfd = Socket(AF_INET, SOCK_STREAM, 0);
    Bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    Listen(lfd, 128);
    while(1){
        cfd = Accept(lfd, (struct sockaddr*)&client,&len);
        pid = fork();
        if(pid<0) perr_exit("process create error");
        else if(pid ==0){
            close(lfd);
            break;
        }else{
            //在父进程中回收子进程
            struct sigaction act;
            act.sa_handler=catch_child;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;
            ret = sigaction(SIGCHLD, &act, NULL); //首先注册信号
            if(ret!=0){
                perr_exit("sigaction error");
            }
            close(cfd);
            continue;
        }
    }
    if(pid==0){ //这里是重点，如果跳出来pid==0， 那么就就执行, pid=0代表是子进程
        for(;;){
            ret = Read(cfd, buf, sizeof(buf));
            if(ret==0){
                close(cfd);
                exit(1);
            }
            for(int i=0;i<ret;i++){
                buf[i] = toupper(buf[i]);
                
            }
            write(cfd, buf, ret);
            write(STDOUT_FILENO, buf, ret);
        }

    }

    return 0;
}
