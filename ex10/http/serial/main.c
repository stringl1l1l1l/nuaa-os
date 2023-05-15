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

    return 0;
}
