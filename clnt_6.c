#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1024
#define MAXSOCK 512

char *escapechar = "exit\n";
int readline(int, char *, int);

int s;
struct Name{
	char n[20];
	int len;
} name;

void filtering(char *, char *, char *, char *);

int main(int argc, char *argv[])
{
	char line[MAXLINE], sendline[MAXLINE+1];
	int n, pid, size;
	struct sockaddr_in server_addr;
	int nfds;
	fd_set read_fds;

	int cnt = 0;
	char msg_name[30] = "name: ";
	
	if(argc < 4)
	{
		printf("correct input : %s host ip address portnum username \n", argv[0]);
		return -1;
	}

	sprintf(msg_name, "%s%s", msg_name, argv[3]);
	//reset chat user name struct
	sprintf(name.n, "[%s]", argv[3]);
	name.len = strlen(name.n);

	//make socket
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Client : Can't open stream socket.\n");
		return -1;
	}

	//Initialize server_addr which is socket address of chat server
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	//request connection
	if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("Client : Can't connet to server.\n");
		return -1;
	}
	else printf("Welcome to chat server!\n");

	nfds = s + 1;
	FD_ZERO(&read_fds);

	while(1)
	{
		FD_SET(0, &read_fds);
		FD_SET(s, &read_fds);
		//select
		if(select(nfds, &read_fds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0 ) < 0 )
		{
			printf("select error\n");
			return -1;
		}
		//msg from server
		if(FD_ISSET(s, &read_fds))
		{
			char recvline[MAXLINE];
			int size;
			if( (size = recv(s, recvline, MAXLINE, 0) ) > 0 )
			{
				recvline[size] = '\0';
				printf("%s \n", recvline);
			}
		}
		
		if(cnt == 0)
		{
			send(s, msg_name, strlen(msg_name), 0);
			cnt++;
		}

		//input from keyboard
		if(FD_ISSET(0, &read_fds))
		{
			if(readline(0, sendline, MAXLINE) > 0)
			{
				if ((ptr = strstr(sendline, "FUCK")) != NULL)
					filtering(ptr, sendline, "FUCK", "XXXX");
				else if ((ptr = strstr(sendline, "fuck")) != NULL)
					filtering(ptr, sendline, "fuck", "xxxx");
				else if ((ptr = strstr(sendline, "WTF")) != NULL)
					filtering(ptr, sendline, "WTF", "XXX");
				else if ((ptr = strstr(sendline, "wtf")) != NULL)
					filtering(ptr, sendline, "wtf", "xxx");
				else if ((ptr = strstr(sendline, "SIBAL")) != NULL)
					filtering(ptr, sendline, "SIBAL", "XXXXX");
				else if ((ptr = strstr(sendline, "sibal")) != NULL)
					filtering(ptr, sendline, "sibal", "xxxxx");
				
				size = strlen(sendline);
				sprintf(line, "%s %s", name.n, sendline);
				if(send(s, line, size + name.len, 0) != (size+name.len))
					printf("Error : Written error on socket.\n");
				if(size == 5 && strncmp(sendline, escapechar, 5) == 0)
				{
					printf("Good bye.\n");
					close(s);
					return -1;
				}
			}
		}
	}
}

int readline(int fd, char *ptr, int maxlen)
{
	int n, rc;
	char c;
	
	for(n = 1; n < maxlen; n++)
	{
		if((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if( c == '\n') break;
		}
		else if (rc == 0)
		{
			if(n == 1) return 0;
			else break;
		}
	}
	*ptr = 0;
	return(n);
}

void filtering(char *ptr, char *line, char *original, char *filter)
{
	char result[MAXLINE];
	char *start;
	
	memset(result, 0, sizeof(result));
	
	if(ptr != line)
		strncpy(result, line, strlen(line) - strlen(ptr));
	
	strcat(result, filter);
	
	start = ptr + strlen(original);
	strcat(result, start);
	
	strcpy(line, result);
}
