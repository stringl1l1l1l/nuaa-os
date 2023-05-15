#include "echo.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int create_client(char *ip_address, int port)
{
    struct sockaddr_in server;
    int fd;
    // use ipv4 
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        error("socket");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip_address);
    server.sin_port = htons(port);
    if (connect(fd, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) < 0)
        error("bind");
    return fd;
}

int main(int argc, char *argv[])
{
    /*if (argc != 2) {
        puts("Usage: client msg\n");
        return 0;
    }*/

    char msg[] = "1234abcd";
    int client = create_client("127.0.0.1", 1234);

    for(int i = 0; i < 5; i++) {
      pid_t pid = fork();

      if(pid == 0) {
        // send(client, msg, strlen(msg), 0);
        write(client, msg + i, 1);
        printf("client send: %s\n", msg);

        // recv(client, buff, sizeof(buff), 0);
        char buff[128];
        int count = read(client, buff, sizeof(buff));
        buff[count] = 0;
        printf("client receive: %s\n", buff);
    }
  }
    return 0;
}
