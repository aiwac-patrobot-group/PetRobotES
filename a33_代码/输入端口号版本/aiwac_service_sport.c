/*
* Author: tangyuan
* Module Date: 
*			version 1 :2019/1/3  
*			version 2 :2019/4/3  适配 #1a. 新协议，A33上只进行数据的传输
* Description: 
* Others:
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>

#define UART_DEV_NAME "/dev/ttyS2"

#define APP_CLIENT_NUM 1000	//可以连的app数
#define SERVPORT 8890	//定义端口号
#define BACKLOG 15	//请求队列中允许的最大请求数
#define MAXDATASIZE 200	//数据长度



void  netInit(void); //进行server网络初始化
void* clientHandle(void *arg);  //接收到后的处理线程

int  uartInit(void);  //串口初始化
void* uartRead(void);  //串口读取线程
int uartMsHandle(char *buf, int bytes); //uart数据分析函数
int socketMsHandle(char *buf, int bytes, int cSFd); //socket数据分析函数



//每个android APP连接时都开一个线程
//app突然中断，该C程序可依然运行
pthread_t clientHandlePth[APP_CLIENT_NUM];  
pthread_t uartReadPth; //串口接收线程

int appClientNum = 0; //当前连接的app数
int client_fd[APP_CLIENT_NUM] ; //socket描述符
int client_fdALL;  //全局socket
int client_fdALLFlag = 0;  //全局socket 可用标签
int sockfd;
struct sockaddr_in server_sockaddr,client_sockaddr;//声明服务器和客户端的socket存储结构


int uartFd;  //串口的设备描述符

char socketBuf[MAXDATASIZE];//socket传输的数据

char uartBuf[MAXDATASIZE];//uart传输的数据
char hState[MAXDATASIZE]; //硬件状态信息
int freeFlag = 0; // 机器人自由的标志，1：自己玩，动  	0；听app的安排
char socketBuf2[MAXDATASIZE]; //socket传输的数据 

//测试使用
int tempPort = 0;
///////////

int main (int argc, char** argv)
{
	int ret = 0 ;
	int sin_size = 0;
	ret = 0;
	
	printf("input port:");
	scanf("%d",&tempPort);
	netInit();  //进行网络的初始化

	ret  = uartInit();  //串口的初始化
	if (ret == 0)
	{
		printf(" uart init fail!\n");
		exit(1);
	}
	
	ret = 0;
	ret = pthread_create(&uartReadPth, NULL, (void *) uartRead, NULL);  //串口接收线程
	if (ret != 0) 
	{
        printf("create thread uartReadPth failed/n");
    }

	printf("\n\nnetInit()   ok\n");
	
	
	
	while (1)
	{
		sin_size = sizeof(client_sockaddr);
		printf("ready to accept \n");
		
		//等待客户端链接
		if((client_fd[appClientNum] = accept(sockfd, (struct sockaddr *) &client_sockaddr,  (socklen_t*)&sin_size)) == -1)   // client_sockaddr[appClientNum]  合适吗
		{
			perror("accept");
			continue;
		}
		printf("accept ok \n");
		
		pthread_create(&clientHandlePth[appClientNum], NULL, (void *) clientHandle, (void*) &client_fd[appClientNum]); //接收到后的处理线程

		appClientNum++;
		printf("appClientNum++  appClientNum:%d\n",appClientNum);

	}
	
	
	pthread_join(uartReadPth, NULL);
	
	while ( appClientNum >=0 )
	{		
		pthread_join(clientHandlePth[appClientNum], NULL);
		appClientNum--;
	}
	
	return 0;

}


void netInit(void)
{
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) 
	{//建立socket链接
		perror("Socket");
		exit(1);
	}

	printf("Socket success!,sockfd=%d\n",sockfd);

	//以sockaddt_in结构体填充socket信息
	server_sockaddr.sin_family = AF_INET;//IPv4
	printf("\ntemperPort:%d\n",tempPort);
	server_sockaddr.sin_port = htons(tempPort);//端口
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;//本主机的任意IP都可以使用
	bzero(&(server_sockaddr.sin_zero),8);//填充0

	if((bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))) == -1) 
	{//bind函数绑定
		perror("bind");
		exit(-1);
	}

	printf("bind success!\n");

	if(listen(sockfd,BACKLOG) == -1) 
	{//监听
		perror("listen");
		exit(1);
	}

	printf("listening ... \n");

}


void* clientHandle(void *arg)
{
	int client_socket = *((int *)arg);
	
	client_fdALL = client_socket;
	int recvbytes = 0;
	char socketBuf[MAXDATASIZE]; //socket传输的数据
	int ret = 0;
	client_fdALLFlag = 1;
	while (1)
	{
		printf(" ready to get ms!!\n");
		printf("\nclient_socket:%d\n",client_socket);
		if((recvbytes = read(client_socket, socketBuf, MAXDATASIZE)) < 1) 
		{
			//接收客户端的请求
			perror("read");
			client_fdALLFlag = 0;
			break;
		}
		
		printf(" get ms from APP , ms:%s, recvbytes:%d,  len:%d\n", socketBuf, recvbytes, strlen(socketBuf));
		
		ret = socketMsHandle(socketBuf, recvbytes, client_socket);
		if (ret == 0)
		{
			printf(" Socket msHandle  error!!\n");
		}
		printf(" Socket msHandle ok!!\n");
		
		memset(&socketBuf, 0, sizeof(socketBuf));
		recvbytes = 0;
	}
	
	return NULL;
	
}


int socketMsHandle(char *buf, int bytes, int cSFd)
{
	int ret = 1;

	printf("Pre send uart from ch559 socketMsHandle    :%s///  bytes:%d   len :%d\n",buf , bytes, strlen(buf));
	
	ret = write(uartFd, buf, bytes);
				
	printf("Next send uart from  ch559 socketMsHandle    :%s///  ret:%d   len :%d\n",buf , ret, strlen(buf));
	if (ret < 0)
	{
		printf("Error,write data to uart form socketMsHandle \n");
	}else
	{
		printf("write(uartFd)  ok \n");
	}		
	
	return ret;
}



int uartInit(void)
{
	int ret = 1;

	
	printf("uartIniting \n");
	uartFd = open(UART_DEV_NAME, O_RDWR | O_NOCTTY);
	if(uartFd  < 0)
	{
		perror("uartFd");
		printf("\n\nopen ttyS2 error \n\n");
		ret = 0;
	}
		
	printf("uartInit()  ok\n");
	return ret;
}


void* uartRead(void)
{
	int uRecvbytes = 0;
	int ret = 0;
		
	while (1)
	{
		printf(" ready read from Uart \n");
		uRecvbytes = read(uartFd, uartBuf, MAXDATASIZE);
		printf(" Uart get  from ch559 ms:%s, len:%d  uRecvbytes:%d!!\n",uartBuf, strlen(uartBuf), uRecvbytes);
		if (uRecvbytes < 0)
		{
			printf(" Uart  read error \n");
		}
		
		
		if ( uartBuf[0] == 0x0a) // 去掉一个  /n
		{
			printf(" Uart  get   n  \n");
			memset(&uartBuf, 0, sizeof(uartBuf));
			uRecvbytes = 0;
			continue;
		}
		
		ret = uartMsHandle(uartBuf, uRecvbytes);
		if (ret == 0)
		{
			printf(" Uart  msHandle  error!!\n");	
		}
					
		memset(&uartBuf, 0, sizeof(uartBuf));
		uRecvbytes = 0;	
	}
	
	
	return NULL;
}


int uartMsHandle(char *buf, int bytes)
{
	int ret = 1;
	int sendBytes =0;
	printf(" Pre uartMsHandle send to JAVA  send ms:%s, len:%d  bytes:%d!!\n",buf, strlen(buf), bytes);
	if (client_fdALLFlag == 0) // 网络有问题
	{
		ret = 0;
		printf("\n net wrong\n");
		return ret;
	}	
	
	//buf[bytes-1] = '\n';
	printf("\nclient_fd[appClientNum-1]:%d\n",client_fdALL);
	sendBytes = write(client_fdALL, buf, bytes);
	printf(" Next uartMsHandle send to 	JAVA  send ms:%s, len:%d  sendBytes:%d!!\n",buf, strlen(buf), sendBytes);
	
	if (sendBytes < 0)
	{
		printf("Error,write data to uart form socketMsHandle \n");
		ret = 0;
	}
	
	return ret;	
}
