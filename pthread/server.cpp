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
#define MAXLINE 8092
using namespace std;


struct s_info{
    struct sockaddr_in cliaddr;
    int connfd;
};

void* do_connect(void* arg){
    int n,i;
    struct s_info *ts = (struct s_info*)arg;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN]; //地址
    while(1){
        n = Read(ts->connfd, buf, MAXLINE);
        if(n==0){ // 没有读到
            cout<<"the client "<<ts->connfd<<" closed...\n"<<endl;
            break;
        }
        cout<<"Recived from "<<inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str))<<" at port"<<ntohs((*ts).cliaddr.sin_port)<<endl;//打印客户端信息
        for(int i=0;i<n;i++){
            buf[i] = toupper(buf[i]);
        }
        Write(STDOUT_FILENO, buf, n);//向标准输出打印信息
        Write(ts->connfd, buf, n); //向客户端打印信息
    }
    Close(ts->connfd);
    return (void *)0; //或者pthread_exit(0) 在子线程是一样的。
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr, client_addr;
    bzero(&serv_addr, sizeof(serv_addr)); // 清零内存空间
    socklen_t cliaddr_len;
    int fd, cfd;
    char buf[BUFSIZ];
    int index = 0;
    struct s_info ts[256];//线程函数的参数
    pthread_t tid[MAXLINE];

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    fd = Socket(AF_INET, SOCK_STREAM, 0);//创建套接字

    Bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); //绑定地址
    Listen(fd, 128);
    while(1){
        cliaddr_len = sizeof(client_addr);
        cfd = Accept(fd,(struct sockaddr*)&client_addr, &cliaddr_len); //等待访问，返回cfd
        ts[index].cliaddr = client_addr;
        ts[index].connfd = cfd;    
        pthread_create(&tid[index], NULL, do_connect, (void *)&ts[index]);
        pthread_detach(tid[index]);
        index++;
    }

    return 0;
}
