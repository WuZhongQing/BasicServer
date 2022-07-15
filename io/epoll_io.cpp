#include "Wrap.h"
#include<iostream>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<errno.h>
#include<sys/epoll.h>

#define MAXLINE 8192
#define PORT 8888
#define OPEN_MAX 5000

using namespace std;

int main(int argc, char const *argv[])
{
    int i, listenfd, connfd, sockfd;
    int n, num=0;
    ssize_t nready, efd, res;
    char buf[MAXLINE], str[INET_ADDRSTRLEN];
    socklen_t clilen;

    struct sockaddr_in cliaddr, servaddr; //客户端，服务器端地址
    struct epoll_event tep, ep[OPEN_MAX]; // tep:epoll_ctl参数， ep[]:epoll_wait参数

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);//创建listen 监听描述符
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,&opt, sizeof(opt));//设置端口复用
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));//绑定地址
    Listen(listenfd, 20);

    efd = epoll_create(OPEN_MAX);//创建红黑树的root
    if(efd == -1) perr_exit("epoll_reate error");
    tep.events = EPOLLIN;
    tep.data.fd = listenfd; //指定lfd的监听事件为 EPOLLIN
    res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
    if(res==-1) perr_exit("epoll_ctl error");
    //epoll 
    for(;;){
        nready = epoll_wait(efd, ep, OPEN_MAX, -1);
        if(nready==-1) perr_exit("epoll_wait error");

        for(int i=0;i<nready;i++){
            if(!(ep[i].events & EPOLLIN)) continue;
            if(ep[i].data.fd==listenfd){ // connect
                clilen = sizeof(clilen);
                connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
                cout<<"connected."<<endl;
                num++;
                tep.events = EPOLLIN;
                tep.data.fd = connfd;
                res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
                if(res==-1) perr_exit("epoll_ctl error");

            }else{
                sockfd = ep[i].data.fd;//
                n = Read(sockfd, buf, MAXLINE);// read from client
                if(n==0){ // disconnect
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, nullptr);
                    if(res==-1) perr_exit("epoll_ctl error");
                    Close(sockfd);
                    cout<<"closed"<<endl;
                }else if(n<0){
                    perr_exit("read error");
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, nullptr);
                    Close(sockfd);
                }else{
                    for(int j=0;j<n;j++){
                        buf[j] = toupper(buf[j]);
                    }
                    Write(sockfd, buf,n);
                    Write(STDOUT_FILENO, buf,n);
                }
                

            }
        }

    }
    Close(listenfd);
    Close(efd);
    return 0;
}
