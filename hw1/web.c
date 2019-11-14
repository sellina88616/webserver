#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

//content of the main page
char webpage[] = 
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Penny's Web Server</title>\r\n"
"<style>body { background-color: #66CDAA }</style></head>\r\n"
"<body><center><h1>Web Server hw1</h1><br>\r\n"
"<img src=\"/test.jpg\"></img></center></body></html>\r\n";

//principal function
int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr, client_addr; 
	socklen_t sin_len = sizeof(client_addr);
	int fd_server, fd_client;
	char buf[2048];
	int fdimg;
	int on = 1;
        
	/*We use socket as a file descriptor, with the domain that will be AF_INET to use the protocols
    	Internet ARPA, then the type, where we will use the variable SOCK_STREAM flow socket, and finally
    	the proto-only that will be 0*/
	fd_server = socket(AF_INET, SOCK_STREAM, 0); 
        
	if (fd_server < 0) {
		perror("socket");
		exit(1);	
	}
        
	/* We set the socket options, we pass our socket, then SOL_SOCKET first layer
    	Socket used for socket-independent options, SO_REUSEADDR specifies the validation rules
	of the addresses supplied to bind () is a Boolean value, then we pass the value at = 1, and the
	sock size */
	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	
	//Address Family
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(18000);
	
	if (bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		perror("bind");
		close(fd_server);
		exit(1);
	}

	if (listen(fd_server, 10) == -1){
		perror("listen");
		close(fd_server);
		exit(1);
	}	
	
	//Main loop that runs on each connection
	while(1) {
		//With accept incoming connections
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
		if (fd_client == -1){
			perror("Connection fail...\n");
			continue;
		}	
		printf("Got client connection.\n");

		//We raise child process
		if (!fork()) {
			close(fd_server);
			memset(buf, 0, 2048);
			read(fd_client, buf, 2047);
			//if the get asks us for the favicon.ico we will send it to him
			if(!strncmp(buf, "GET /favicon.ico", 16)){
				fdimg = open("favicon.ico", O_RDONLY);
				sendfile(fd_client, fdimg, NULL, 4000);
				close(fdimg);
			}
			//When you ask us for the image, we send it to you so that it can be displayed
			else if(!strncmp(buf, "GET /test.jpg", 13)) {
				fdimg = open("test.jpg", O_RDONLY);
				sendfile(fd_client, fdimg, NULL, 420000);
				close(fdimg);
			}
			//For when the GET is for /
			else{
				write(fd_client, webpage, sizeof(webpage) - 1);
			}
			close(fd_client);
			printf("Closing...\n");
			exit(0);
		}
		close(fd_client);
	}	
	return 0;
}
