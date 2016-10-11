#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#define DEFAULT_PORT 23333

#define BOARD_SIZE 19
#define BLACK      1
#define WHITE      2
#define WIN_FLAG   6


char board[BOARD_SIZE][BOARD_SIZE] = {0};
char buffer[MAXBYTE] = {0};
int turn = BLACK;
int row, col;

SOCKET servSock, blackSock, whiteSock;

/*
 * ������
 */
 
BOOL isPort(const int port)
{
	return (port >= 0 && port <= 65535);
}


char *getIp()
{
	PHOSTENT hostinfo;
	char name[255];
	char* ip;
    if(gethostname(name, sizeof(name)) == 0)
    {
        if((hostinfo = gethostbyname(name)) != NULL)
        {
            ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
            return ip;
        }
    }
    return NULL;
} 

void sendTo(SOCKET *sock, const char *message)
{
	send(*sock, message, strlen(message)+sizeof(char), NULL);
}

void retry(SOCKET *sock)
{
	sendTo(sock, "READY\n");
}

BOOL isWin(int x, int y)
{
	int count;
	int i, j;
	
	//�жϺ��ŵ�
	count = 1;
	i = x - 1;
	while (i >= 0 && board[x][y] == board[i][y])
	{
		++count;
		--i;
	}
	
	i = x + 1;
	while (i < BOARD_SIZE && board[x][y] == board[i][y])
	{
		++count;
		++i;
	}
	
	if (count >= WIN_FLAG) return TRUE;
	
	//�������ŵ�
	count = 1;
	j = y - 1;
	while (j >= 0 && board[x][y] == board[x][j])
	{
		++count;
		--j;
	}
	
	j = y + 1;
	while (j < BOARD_SIZE && board[x][y] == board[x][j])
	{
		++count;
		++j;
	}
	
	if (count >= WIN_FLAG) return TRUE;
	
	//������б�ŵ�
	count = 1;
	i = x - 1;
	j = y - 1;
	while (i >= 0 && j >= 0 && board[x][y] == board[i][j])
	{
		++count;
		--i;
		--j;
	}
	
	i = x + 1;
	j = y + 1;
	while (i < BOARD_SIZE && j < BOARD_SIZE && board[x][y] == board[i][j])
	{
		++count;
		++i;
		++j;
	}
	
	if (count >= WIN_FLAG) return TRUE;
	
	//������б�ŵ�
	count = 1;
	i = x - 1;
	j = y + 1;
	while (i >= 0 && j < BOARD_SIZE && board[x][y] == board[i][j])
	{
		++count;
		--i;
		++j;
	}
	
	i = x + 1;
	j = y - 1;
	while (i < BOARD_SIZE && j >= 0 && board[x][y] == board[i][j])
	{
		++count;
		++i;
		--j;
	}
	
	if (count >= WIN_FLAG) return TRUE;
	
	return FALSE;
}

void handle(SOCKET *me, int meFlag, SOCKET *other, int otherFlag)
{
	memset(buffer, 0, sizeof(buffer));
	recv(*me, buffer, MAXBYTE, NULL);
	sscanf(buffer, "%d %d\n", &row, &col);
	
	//�ж������Ƿ�Ϸ� 
	if (board[row][col] != 0)
	{
		retry(me);
		return;
	}
	
	//���� 
	board[row][col] = meFlag;
	
	switch (meFlag)
	{
		case BLACK:
			printf("BLACK step at (%d, %d)\n", row, col);
			break;
		case WHITE:
			printf("WHITE step at (%d, %d)\n", row, col);
			break;
	}
	
	//ת��
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "TURN %d %d\n", row, col);
    sendTo(other, buffer);
    
    //�ж���Ӯ
    if (isWin(row, col))
    {
    	sendTo(me, "WIN\n");
    	sendTo(other, "LOSE\n");
    	return;
	}
	
	//����
	sendTo(other, "READY\n");
	
	turn = otherFlag;
}

void initSock(int port)
{
    //��ʼ�� DLL
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);
    
    //�����׽��� 
    servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    //���׽��� 
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //ÿ���ֽڶ�ʹ��0��� 
    sockAddr.sin_family = PF_INET;  //ʹ��ipv4��ַ 
    sockAddr.sin_addr.s_addr = inet_addr("0.0.0.0");  //��IP��ַ 
    sockAddr.sin_port = htons(port);  //�󶨶˿� 
    bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    
    //�������״̬ 
    listen(servSock, 20);
    
    char *ip = getIp();
    if (NULL != ip)
    {
		printf("Listening on %s:%d\n", ip, port);
	}
	else
	{
    	printf("Listening...\n");
    }
    
    int nSize;
    
    //���պ������� 
    SOCKADDR blackAddr;
    nSize = sizeof(SOCKADDR);
    blackSock = accept(servSock, (SOCKADDR*)&blackAddr, &nSize);
    printf("Client Connected\n");
    
    //���հ������� 
    SOCKADDR whiteAddr;
    nSize = sizeof(SOCKADDR);
    whiteSock = accept(servSock, (SOCKADDR*)&whiteAddr, &nSize);
    printf("Client Connected\n");
    
    //��ʼ������
	sendTo(&blackSock, "READY\n");
}

void closeSock()
{
    //�ر��׽��� 
    closesocket(blackSock);
    closesocket(whiteSock);
    closesocket(servSock);
    
    //��ֹ DLL ��ʹ�� 
    WSACleanup();
}

void work()
{   
    while (TRUE)
    {
    	// �Ƚ����������� 
    	switch (turn)
    	{
    		case BLACK:
    			handle(&blackSock, BLACK, &whiteSock, WHITE);
    			break;
    		case WHITE:
    			handle(&whiteSock, WHITE, &blackSock, BLACK);
    			break;
		}
	}
}

int main(int argc, char *argv[]){
	
	if (2 == argc)
	{
		const char *port = atoi(argv[1]);
		if (isPort(port))
			initSock(port);
		else
			initSock(DEFAULT_PORT);
	}
	else
	{
		initSock(DEFAULT_PORT);
	}
	
	work();
	closeSock();
	
    return 0;
}
