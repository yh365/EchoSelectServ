#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
	WSADATA	wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;
	TIMEVAL timeout;
	fd_set readSet, cpyRead;

	char message[BUF_SIZE];
	int strLen, fdNum, i;
	int clntAdrSize;

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup ERROR");

	hServSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("socket ERROR");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htons(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind ERROR");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen ERROR");

	FD_ZERO(&readSet);
	FD_SET(hServSock, &readSet);

	while (1)
	{
		cpyRead = readSet;
		timeout.tv_sec = 3;
		timeout.tv_usec = 3000;

		if ((fdNum = select(0, &cpyRead, 0, 0, &timeout)) == SOCKET_ERROR)
			break;

		if (fdNum == 0)
		{
			printf("No events..\n");
			continue;
		}

		for (i = 0; i < readSet.fd_count; i++)
		{
			if (FD_ISSET(readSet.fd_array[i], &cpyRead))
			{
				if (readSet.fd_array[i] == hServSock)
				{
					clntAdrSize = sizeof(clntAddr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAdrSize);
					FD_SET(hClntSock, &readSet);

					if (hClntSock == INVALID_SOCKET)
						ErrorHandling("accept ERROR");
					else
					{
						char* str = inet_ntoa(clntAddr.sin_addr);
						printf("Connected client : %d\n", hClntSock);
						printf("Connected client ip : %s\n", str);
					}
				}
				else
				{
					strLen = recv(readSet.fd_array[i], message, BUF_SIZE - 1, 0);

					if (strLen == 0)
					{
						FD_CLR(readSet.fd_array[i], &readSet);
						closesocket(cpyRead.fd_array[i]);
						printf("closed client : %d\n", cpyRead.fd_array[i]);
					}
					else
					{
						send(readSet.fd_array[i], message, strLen, 0);
					}
				}
			}
		}
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
