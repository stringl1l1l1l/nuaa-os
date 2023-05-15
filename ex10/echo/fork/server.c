#include "echo.h"

int create_server(char *ip_address, int port)
{
    struct sockaddr_in server;
    int fd;
    int option;

    // create a socket, return its fd
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        error("socket");

    // allow reuse socket
    option = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
        error("setsockopt");

    server.sin_family = AF_INET;// use IPv4
    server.sin_addr.s_addr = inet_addr(ip_address);// Dot-decimal ip to uint32_t ip
    server.sin_port = htons(port);// byte sequence transfer
    if (bind(fd, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) < 0)
        error("bind");
    listen(fd, 8);// listening on specified socket addr, allow max 8 requests
    return fd;
}

void echo_handler(int fd)
{
    char buff[128];
    // recv(client, buff, sizeof(buff), 0);
    int count = read(fd, buff, sizeof(buff));
    buff[count] = 0;
    printf("server received: %s\n", buff);

    int i;
    for (i = 0; i < count; i++)
        buff[i] = toupper(buff[i]); // transefer letter characters to uppercase
    write(fd, buff, count);
    close(fd);
}

void run_echo_server(char *ip_address, int port)
{
    int server_fd = create_server(ip_address, port);
    while (1) {
        struct sockaddr_in client;
        socklen_t length = sizeof(struct sockaddr_in);
        int client_fd;
        // server will create a client socket when accepting a request
        // this function will be blocked until there is a request
        client_fd = accept(server_fd, (struct sockaddr *)&client, &length);
        if (client_fd < 0)
            error("accept");
        printf("accept client\n");

        echo_handler(client_fd);
    }
}

int main(int argc, char *argv[])
{
    char *ip_address = "127.0.0.1";
    int port = 1234;
    run_echo_server(ip_address, port);
    return 0;
}
