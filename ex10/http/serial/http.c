#include "std.h"
#include "http.h"
#include <string.h>

char *web_root = "www";
#define MAX_REQ_SIZE 512

/**
 * @brief 向socket文件中写入一个大小为size的chunk
 * http_write_chunk(fw, "hello", 5)
 * 共发送 3+5+2 个字符
 * 5\r\nHELLO\r\n
 * @param fw    socket的文件结构体
 * @param chunk 一个chunk
 * @param size
 */
void http_write_chunk(FILE *fw, void *chunk, int size)
{
    fprintf(fw, "%x\r\n", size);
    fwrite(chunk, size, 1, fw);
    fprintf(fw, "\r\n");
}

/**
 * @brief 向指定文件中写入http chunk的终止符号
 * 共发送 3+0+2 个字符
 * 0\r\n\r\n
 */
void http_end(FILE *fw)
{
    http_write_chunk(fw, NULL, 0);
    fflush(fw);
}

/**
 * @brief @brief 向指定文件中写入http chunk格式的字符串
 *
 * @param fw
 * @param string
 */
void http_prints(FILE *fw, void *string)
{
    int size = strlen(string);
    http_write_chunk(fw, string, size);
}

/**
 * @brief 向指定文件中写入http chunk格式的格式化字符串
 *
 */
void http_printf(FILE *fw, const char *format, ...)
{
    va_list ap;// 可变参数列表
    va_start(ap, format);
    char chunk[MAX_REQ_SIZE];
    int size = vsprintf(chunk, format, ap);
    va_end(ap);

    http_write_chunk(fw, chunk, size);
}

/**
 * @brief 向socket写入响应头
 *
 * @param fp 指定的socket文件结构体
 * @param content_type 响应内容的类型，"text/html"表明这是html格式文件，"text/plain"表示是纯文本文件
 */
void http_send_headers(FILE *fp, char *content_type)
{
    fprintf(fp, "HTTP/1.1 200 OK\r\n");
    fprintf(fp, "Server: tiny httpd\r\n");
    fprintf(fp, "Content-type: %s\r\n", content_type);
    fprintf(fp, "Transfer-Encoding: chunked\r\n");
    fprintf(fp, "\r\n");
}

/**
 * @brief 解析客户端发来的请求报文 [GET /index.html HTTP/1.1\r\n]
 *
 * @param fr 客户端sokect文件结构体
 * @param req 保存解析出的字符串
 * @param req_size 请求报文字符串大小
 * @return char* 解析出的路径字符串
 */
char *http_parse_req(FILE *fr, char *req, int req_size)
{
    fgets(req, req_size, fr);

    char *path = (char*)malloc(sizeof(char) * req_size);
    // strcpy(path, web_root);
    memset(path, 0, req_size);
    // 由于strtok会修改原字符串，因此开辟临时变量存一下
    char *tmp = (char*)malloc(req_size);
    strcpy(tmp, req);
    strtok(tmp, " ");
    strcat(path, strtok(NULL, " ")); // 获取按空格分隔的第二个字符串，也就是相对路径, 并追加到根路径后

    free(tmp);

    return path;
}


/**
 * @brief 客户端请求获取普通文档时的处理函数, 将文档内容写入socket
 *
 * @param fw
 * @param path
 */
void file_handler(FILE* fw, char *path) {

    int pathLen = strlen(path);
    char type[15] = "text/plain";
    if(pathLen > 5) {
        char tail[6];
        strcpy(tail, path + strlen(path) - 5);
        if(strcmp(tail, ".html") == 0)
            strcpy(type, "text/html");
    }


    char * path_with_root = (char *) malloc(MAX_REQ_SIZE);
    strcpy(path_with_root, web_root);
    strcat(path_with_root, path);

    FILE *file = fopen(path_with_root, "r");
    if(file == NULL)
    {
        perror(path_with_root);
        return;
    }

    http_send_headers(fw, type);

    char line[MAX_REQ_SIZE];
    while (fgets(line, sizeof(line), file)) {
        http_prints(fw, line);
    }

    http_end(fw);
    fclose(file);
}

/**
 * @brief 客户端请求获取目录时的处理函数
 *
 * @param fw sokect文件结构体
 * @param path 请求的路径
 */
void dir_handler(FILE* fw, char *path)
{
    char * path_with_root = (char *) malloc(MAX_REQ_SIZE);
    strcpy(path_with_root, web_root);
    strcat(path_with_root, path);

    DIR *dir = opendir(path_with_root);
    if(dir == NULL)
    {
        perror(path_with_root);
        return;
    }

    http_send_headers(fw, "text/html");
    http_prints(fw, "<html>");
    http_prints(fw, "<body>");
    http_printf(fw, "<h2>%s</h2>", path);
    http_prints(fw, "<ol>");
    struct dirent *entry;
    // readdir用于读取目录所有项，成功读取则返回下一个入口点，否则返回NULL
    while (entry = readdir(dir)) {
        if (strcmp(entry->d_name, ".") == 0)
            continue;

        if (strcmp(entry->d_name, "..") == 0)
            continue;

        if (entry->d_type == DT_DIR) {
            char newPath[256];
            strcpy(newPath, path);
            if(path[strlen(path) - 1] != '/')
                strcat(newPath, "/");
            strcat(newPath, entry->d_name);
            http_printf(fw, "<a href=\"%s\">/%s</a><br>", newPath, entry->d_name);
            // http_printf(fw, "<a href=\"/%s\">/%s</a><br>", entry->d_name, entry->d_name);
        }

        if (entry->d_type == DT_REG) {
            char newPath[256];
            strcpy(newPath, path);
            if(path[strlen(path) - 1] != '/')
                strcat(newPath, "/");
            strcat(newPath, entry->d_name);
            http_printf(fw, "<a href=\"%s\">/%s</a><br>", newPath, entry->d_name);
        }
    }
  
    http_prints(fw, "<ol>");
    http_prints(fw, "<body>");
    http_prints(fw, "<html>");
    http_end(fw);
    
    closedir(dir);
}

void http_handler(int fd)
{
    FILE *fr = fdopen(fd, "r");
    FILE *fw = fdopen(fd, "w");

    char req[MAX_REQ_SIZE];
    char *http_path;
    http_path = http_parse_req(fr, req, sizeof(req));
    puts(http_path);

    // 若请求根路径，返回index.html
    if(strcmp(http_path, "/") == 0) {
        strcat(http_path, "index.html");
    }
    
    // sizeof 对一个堆内存中的字符指针作用时，只返回一个指针的大小
    char * path_with_root = (char *) malloc(MAX_REQ_SIZE);
    strcpy(path_with_root, web_root);
    strcat(path_with_root, http_path);

    struct stat info;
    if(stat(path_with_root, &info) == -1) {
        file_handler(fw, "/404.html");
    }
    else if (S_ISDIR(info.st_mode)) {
        dir_handler(fw, http_path);
    } else if(S_ISREG(info.st_mode)) {
        file_handler(fw, http_path);
    }
    free(http_path);
}
