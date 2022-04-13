#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include<sys/un.h>

#define PORT 502

#define Def_Controller_Sensor_1 			0x01
#define Def_Controller_Sensor_2				0x02

#define Def_Controller_TriggerReady			0x11
#define Def_Controller_TriggerAck			0x12
#define Def_Controller_ResultsAvailable		0x13

#define Def_Controller_Motor_1 				0x31
#define Def_Controller_Motor_2				0x32

#define Def_Controller_TriggerEnable		0x41
#define Def_Controller_BufferResultsEnable	0x42
#define Def_Controller_Trigger 				0x43
#define Def_Controller_ResultsAck 			0x44







int clientsocket[2];
struct sockaddr_in serveraddr;
char sendbuf[12];
int recvck;
char RecBuff[12];
int ErrorCount = 0;
struct timeval start,end;  
long timeuse;
const char *filename="/tmp/server.sock";
struct sockaddr_un un;
int fd, new_fd;




/*****************************************************************************
 * 
 * 	建立連線 
 * 		控制器	ip : 192.168.1.75
 * 		鏡頭	ip : 192.168.1.76
 * 
******************************************************************************/
int createsocket()
{	
	for(int i=0;i<1;i++)
	{
		if((clientsocket[i] = socket(AF_INET,SOCK_STREAM,0)) < 0)
		{
			perror("socket error");
			return -1;
		}
		else  printf("socket OK\n");

		
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(PORT);


			serveraddr.sin_addr.s_addr = inet_addr("10.63.1.31");

		if(connect(clientsocket[i],(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
		{
            printf("is me\n");
			perror("connect");
			exit(1);
		}
		else
		{
			printf("connect to server\n");
			
		}
	}
	return 1;

	
}






/*****************************************************************************
 * 
 * 	Modbus TCP 寫入資料
 * 
 * 		參數 ip 		:	0 (控制器)	、  1 (鏡頭)
 * 		參數 Function	:	0x05 (寫入0x) 、  0x06 (寫入4x)
 * 		參數 Address	: 	要讀取的位址
 * 		參數 Data		:	要讀取的資料
 * 
******************************************************************************/
void WriteData(unsigned ip,unsigned char Function,unsigned int Address,unsigned int Data)
{
	sendbuf[0] = 0xac;
	sendbuf[1] = 0x70;
	sendbuf[2] = 0x00;
	sendbuf[3] = 0x00;
	sendbuf[4] = 0x00;
	sendbuf[5] = 0x06;
	sendbuf[6] = 0x01;
	sendbuf[7] = Function;
	sendbuf[8] = (Address >> 8);
	sendbuf[9] = (Address & 0x00ff);
	switch(Function)
	{
		// Write bit(0x)
		case 0x05:
			if(Data == 0)
				sendbuf[10] = 0x00;
			else
				sendbuf[10] = 0xFF;

			sendbuf[11] = 0x00;
			break;
		// Write Word(4x)
		case 0x06:
			sendbuf[10] = (Data >> 8);
			sendbuf[11] = (Data & 0x00ff);
			break;
	}


	send(clientsocket[ip],sendbuf,sizeof(sendbuf),0);
	while((recvck = recv(clientsocket[ip],RecBuff,sizeof(RecBuff),0)) == 0);
	ErrorCount = 0;
	for(int i=0;i<12;i++)
	{
		//printf("%d %d %d\n",i,sendbuf[i],RecBuff[i]);
		if(RecBuff[i] != sendbuf[i])
			ErrorCount++;
	}
	if(ErrorCount == 0)
	{
		//printf("寫入成功\n");
	}
	else
	{
		printf("封包錯誤數量：%d\n",ErrorCount);
	}
	//printf("--------------------------------------\n");
}



/*****************************************************************************
 * 
 * 	Modbus TCP 讀取資料
 * 
 * 		參數 ip 		:	0 (控制器)	、  1 (鏡頭)
 * 		參數 Function	:	0x02 (讀取1x) 、  0x04 (讀取3x)
 * 		參數 Address	: 	要讀取的位址
 * 		參數 Count		:	要讀取的數量
 * 
******************************************************************************/
char* ReadData(unsigned char ip,unsigned char Function,unsigned int Address,unsigned int Count)
{
	sendbuf[0] = 0xac;
	sendbuf[1] = 0x70;
	sendbuf[2] = 0x00;
	sendbuf[3] = 0x00;
	sendbuf[4] = 0x00;
	sendbuf[5] = 0x06;
	sendbuf[6] = 0x01;
	sendbuf[7] = Function;
	sendbuf[8] = (Address >> 8);
	sendbuf[9] = (Address & 0x00ff);
	sendbuf[10] = (Count >> 8);
	sendbuf[11] = (Count & 0x00ff);


	unsigned int DataLength;

	if(Function == 0x01 || Function == 0x02)
		DataLength = (Count - 1) / 8 + 1;
	if(Function == 0x03 || Function == 0x04)
		DataLength = Count * 2;

	send(clientsocket[ip],sendbuf,sizeof(sendbuf),0);

    printf("開始接收\n");

	//-----------------------------------------------------
	// 讀取回傳資料

	char ReturnBuff[DataLength + 9];
	while((recv(clientsocket[ip],ReturnBuff,sizeof(ReturnBuff),0)) == 0){
        printf("test\n");
    }
    
	ErrorCount = 0;
	for(int i=0;i<8;i++)
	{
		if(i == 5)
			continue;
		if(ReturnBuff[i] != sendbuf[i])
			ErrorCount++;
	}
	if(ReturnBuff[8] != DataLength)
		ErrorCount++;


	static char returnData[256];
	for(int i = 0; i < DataLength;i++)
	{
        printf("%c\n",ReturnBuff[i + 9]);
		returnData[i] = ReturnBuff[i + 9];
	}

	return returnData;
}

void closed(void)
{
	WriteData(0,0x05,0,0);	// 馬達 1
	WriteData(0,0x05,1,0);	// 馬達 2

}

int CheckTimeout(int Command, int WaitFlag, float SetTime,char* print);
void ControllerUse(int Command, int Status);

int main()
{
	//atexit (closed);


	char Now[20];

	char *ReturnData;

	int StandAlongMode = 0;
	int ConnectFlag = 0;
	
	int passFlag = 0;
	int Step = -1;
	char recv_buffer[1000];
	char send_buffer[1000];
    int recvck = 0;
	int SendLength = 0;
	int Index = 1;
	int TimeoutFlag = 0;
	struct timeval timeout = {0,5000}; 

	
    createsocket();
    printf("ddd\n");
	printf("這個: %c\n",*(ReadData(0,0x00,3,1)) );
    printf("aaa\n");
    scanf("%d",&Index);
	// while(1)
	// {
        

        
    //     // WriteData(0,0x05,1,0);

		
			

			
    // }

	return 0;
}


			
		


void ControllerUse(int Command, int Status)
{
	switch (Command)
	{
		case Def_Controller_Motor_1:
			WriteData(0,0x05,0,Status);
				/* code */
			break;
		case Def_Controller_Motor_2:
			WriteData(0,0x05,1,Status);
			/* code */
			break;
		case Def_Controller_TriggerEnable:

			break;

		case Def_Controller_BufferResultsEnable:

			break;

		case Def_Controller_Trigger:
	
			break;
		case Def_Controller_ResultsAck:
	
			break;
	}
}

// gettimeofday(&start, NULL );  

// 			while(1)
// 			{
// 				gettimeofday(&end, NULL );  
// 				timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;  
// 				if(timeuse >= 0.3)
// 				{
// 					StopMotor2();
// 					close(clientsocket);
// 				}
// 				printf("解碼前time=%f\n",timeuse /1000000.0);


// 			}