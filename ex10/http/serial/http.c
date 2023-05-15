#include "std.h"
#include "http.h"

char *web_root = "www";

// http_write_chunk(fw, "hello", 5)
// 共发送 3+5+2 个字符
// 5\r\nHELLO\r\n
void http_write_chunk(FILE *fw, void *chunk, int size)
{
    fprintf(fw, "%x\r\n", size);
    fwrite(chunk, size, 1, fw);
    fprintf(fw, "\r\n");
}

// 共发送 3+0+2 个字符
// 0\r\n\r\n
void http_end(FILE *fw)
{
    http_write_chunk(fw, NULL, 0);
    fflush(fw);
}

void http_prints(FILE *fw, void *string)
{
    int size = strlen(string);
    http_write_chunk(fw, string, size);
}

void http_printf(FILE *fw, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char chunk[100];
    int size = vsprintf(chunk, format, ap);
    va_end(ap);

    http_write_chunk(fw, chunk, size);
}

void http_send_headers(FILE *fp, char *content_type)
{
    fprintf(fp, "HTTP/1.1 200 OK\r\n");
    fprintf(fp, "Server: tiny httpd\r\n");
    fprintf(fp, "Content-type: %s\r\n", content_type);
    fprintf(fp, "Transfer-Encoding: chunked\r\n");
    fprintf(fp, "\r\n");
}

// GET /index.html HTTP/1.1\r\n
char *http_parse_req(FILE *fr, char *req, int req_size)
{
    fgets(req, req_size, fr); 
    puts(req);
    return NULL;
}

void http_handler(int fd)
{
    FILE *fr = fdopen(fd, "r");
    FILE *fw = fdopen(fd, "w");

    char req[1024];
    char *http_path;
    http_path = http_parse_req(fr, req, sizeof(req));

#if 1
    http_send_headers(fw, "text/html");
#else
    http_send_headers(fw, "text/plain");
#endif

    http_prints(fw, "<h1>HELLO</h1>");
    http_end(fw);
}
