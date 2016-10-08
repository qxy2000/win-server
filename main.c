#include <stdio.h>
#include <winsock2.h>

#define BOARD_SIZE 19
#define BLACK      1
#define WHITE      2
#define WIN_FLAG   6


char board[BOARD_SIZE][BOARD_SIZE] = {0};
char buffer[MAXBYTE] = {0};
int turn = BLACK;
int row, col;

SOCKET servSock, blackSock, whiteSock;

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
	board[row][col] = BLACK;
	
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

void initSock()
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
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //��IP��ַ 
    sockAddr.sin_port = htons(23333);  //�󶨶˿� 
    bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    
    //�������״̬ 
    listen(servSock, 20);
    printf("Listening...\n");
    
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

int main(){
	initSock();
	work();
	closeSock();
	
    return 0;
}
