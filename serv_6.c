#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 1024
#define MAXSOCK 512

char *escapechar = "exit\n";
int readline(int, char *, int);
int exitCheck(char *, char *, int);

struct Name
{
	char name[20];
	int client_s;
};

struct Name client[MAXSOCK];

int main(int argc, char *argv[])
{
	char rline[MAXLINE], my_msg[MAXLINE];
	char *start = "Welcome to chat program from 0x11!\n";
	int i, j, n;
	int s, client_fd, clilen;
	int nfds;	//max socket num + 1
	fd_set read_fds;	//socknum struct to detect read
	int num_chat = 0;	//client num
	struct sockaddr_in client_addr, server_addr;

	if(argc < 2)
	{
		printf("Input : %s portnum\n", argv[0]);
		return -1;
	}

	printf("Chat server reset...\n");

	//make Early socket
	if((s=socket(PF_INET, SOCK_STREAM, 0))<0)
	{
		printf("Server : Can't open stream socket.\n");
		return -1;
	}

	//server_addr struct setting
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	if(bind(s,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("Server : Can't bind local address.\n");
		return -1;
	}

	//Waiting for clnt's connect request
	listen(s, 5);

	//Max socknum + 1
	nfds = s + 1;
	FD_ZERO(&read_fds);

	while(1)
	{
		//renewal max socknum + 1
		if((num_chat - 1)>=0) nfds = client[num_chat-1].client_s + 1;

		//socketnum which detect read change -> save fd_set struct
		FD_SET(s, &read_fds);
		for(i = 0;i < num_chat;i++) FD_SET(client[i].client_s, &read_fds);

		//call select()
		if(select(nfds, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0) < 0)
		{
			printf("select error\n");
			return -1;
		}
		//processing clnt connection request
		if(FD_ISSET(s, &read_fds))
		{
			clilen = sizeof(client_addr);
			client_fd = accept(s, (struct sockaddr *)&client_addr, &clilen);

			if(client_fd != -1)
			{
				//add list of chat clnt
				client[num_chat].client_s = client_fd;
				num_chat++;
				send(client_fd, start, strlen(start), 0);
				printf("New person!  Total %d people in this chat serv.\n", num_chat);
			}
		}
		//sent msg to everyone. The msg is sent by someone
		for(i=0;i<num_chat;i++)
		{
			if(FD_ISSET(client[i].client_s, &read_fds))
			{
				if((n = recv(client[i].client_S, rline, MAXLINE, 0)) > 0 )
				{
					//input exit -> exit
					if(exitCheck(rline, escapechar, 5) == 1)
					{
						shutdown(client[i].client_s, 2);
						if(i != num_chat - 1) client[i].client_s = client[num_chat-1].client_s;
						num_chat--;
						continue;
					}
					//sent msg to everyone
					for(j = 0; j < num_chat; j++) send(client[j].client_S, rline, n, 0);
					printf("%s", rline);
				}
			}
		}
	}
}


int exitCheck(char *rline, char *escapechar, int len)
{
	int i, max;
	char *tmp;

	max = strlen(rline);
	tmp = rline;
	for(i = 0; i < max; i++)
	{
		if(*tmp == escapechar[0])
		{
			if(strncmp(tmp, escapechar, len) == 0)
				return 1;
		}
		else tmp++;
	}
	return -1;
}

int readline(int fd, char *ptr, int maxlen)
{
	int n,rc;
	char c;
	for(n = 1; n < maxlen; n++)
	{
		if((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if(c=='\n') break;
		}
		else if(rc == 0)
		{
			if(n == 1) return 0;
			else break;
		}
	}
	*ptr = 0;
	return n;
}

