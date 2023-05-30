#include "std.h"
#include "tcp.h"
#include "http.h"

void usage()
{
    puts("Usage: httpd -p port -h");
    puts("  -p port");
}

int main(int argc, char *argv[])
{
    char *ip_address = "0.0.0.0";
    int port = 80;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            i++;
            port = atoi(argv[i]);
            continue;
        }
    }

    // 打开服务器，持续监听客户端请求
    int server_fd = tcp_listen(ip_address, port);
    while (1) {
        int client_fd = tcp_accept(server_fd);
        http_handler(client_fd);
        close(client_fd);
    }
    return 0;
}
