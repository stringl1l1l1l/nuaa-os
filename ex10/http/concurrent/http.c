#include "http.h"
#include "std.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* web_root = "www";

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2
#define MAX_REQ_SIZE 512
#define MAX_STR_LEN 100
#define MAX_LINE_LEN 512
#define MAX_ARG_CNT 10
#define MAX_FILE_SIZE 1024

/**
 * @brief 解析带参数的URL，并将参数放入env中
 * 带参数的URL：/app/show_env?name=tom&age=10
 * @param envv 解析出的参数的存储位置
 * @param url 带解析的URL
 */
void parse_url(char* res, char** envv, char* url)
{
    char* p = strrchr(url, '?');
    if (p == NULL) {
        strcpy(res, url);
        return;
    }
    *p = 0; // 将原始URL的参数部分截断
    strcpy(res, url); // 将不带参数的url保存到res
    *p = '?'; // 恢复原始url
    
    // p + 1 开始是参数字符串
    char *tmp = malloc(MAX_REQ_SIZE);
    strcpy(tmp, "QUERY_STRING=");
    strcat(tmp, p + 1);
    envv[0] = tmp;
}

/**
 * @brief 	该函数创建一个管道，在子进程中重定向标准输出到管道写端，将命令的执行结果写入管道；
 *			在父进程中将管道内容读出。
 * @param buffer 要接收输出字符串的buffer指针
 * @param path 脚本文件路径
 */
void read_pipe(char* buffer, char* path, char **envv)
{
    int fd[2] = { -1, -1 };
    int red = -1, wrt = -1;
    char* argv[2] = { NULL, NULL };

    argv[0] = path;

    pipe(fd);
    red = fd[0];
    wrt = fd[1];
    int pid = fork();
    if (pid == 0) {
        // 关闭子进程的管道读端
        close(red);
        // 重定向标准输出到管道写端
        dup2(wrt, STD_OUT);
        // 及时关闭无用的管道写口
        close(wrt);
        // 分割命令字符串并执行
        execve(argv[0], argv, envv);
    }
    // 父进程等待子进程结束，此时子进程已将执行结果写入管道
    wait(NULL);
    // 重定向标准输入到管道读端
    dup2(red, STD_IN);
    // 关闭父进程管道读写端
    close(red);
    close(wrt);
    // 将管道中的数据通过管道读端读到buffer中
    int cnt = -1;
    do {
        cnt = read(STD_IN, buffer, MAX_LINE_LEN * sizeof(char));
        write(STD_OUT, buffer, cnt * sizeof(char));
    } while (cnt > 0);
}

/**
 * @brief 向socket文件中写入一个大小为size的chunk
 * http_write_chunk(fw, "hello", 5)
 * 共发送 3+5+2 个字符
 * 5\r\nHELLO\r\n
 * @param fw    socket的文件结构体
 * @param chunk 一个chunk
 * @param size
 */
void http_write_chunk(FILE* fw, void* chunk, int size)
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
void http_end(FILE* fw)
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
void http_prints(FILE* fw, void* string)
{
    int size = strlen(string);
    http_write_chunk(fw, string, size);
}

/**
 * @brief 向指定文件中写入http chunk格式的格式化字符串
 *
 */
void http_printf(FILE* fw, const char* format, ...)
{
    va_list ap; // 可变参数列表
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
void http_send_headers(FILE* fp, char* content_type)
{
    fprintf(fp, "HTTP/1.1 200 OK\r\n");
    fprintf(fp, "Server: tiny httpd\r\n");
    fprintf(fp, "Content-type: %s;charset=utf-8\r\n", content_type);
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
char* http_parse_req(FILE* fr, char* req, int req_size)
{
    fgets(req, req_size, fr);

    char* path = (char*)malloc(sizeof(char) * req_size);
    // strcpy(path, web_root);
    memset(path, 0, req_size);
    // 由于strtok会修改原字符串，因此开辟临时变量存一下
    char* tmp = (char*)malloc(req_size);
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
void file_handler(FILE* fw, char* path)
{

    int pathLen = strlen(path);
    char type[15] = "text/plain";
    if (strstr(path, ".html") != NULL)
        strcpy(type, "text/html");

    char path_with_root[MAX_REQ_SIZE];
    strcpy(path_with_root, web_root);
    strcat(path_with_root, path);

    FILE* file = fopen(path_with_root, "r");
    if (file == NULL) {
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
void dir_handler(FILE* fw, char* path)
{
    char path_with_root[MAX_REQ_SIZE];
    strcpy(path_with_root, web_root);
    strcat(path_with_root, path);

    DIR* dir = opendir(path_with_root);
    if (dir == NULL) {
        perror(path_with_root);
        return;
    }

    http_send_headers(fw, "text/html");
    http_prints(fw, "<html>");
    http_prints(fw, "<body>");
    http_printf(fw, "<h2>%s</h2>", path);
    http_prints(fw, "<ol>");

    struct dirent* entry;
    // readdir用于读取目录所有项，成功读取则返回下一个入口点，否则返回NULL
    while (entry = readdir(dir)) {
        if (strcmp(entry->d_name, ".") == 0)
            continue;

        if (strcmp(entry->d_name, "..") == 0)
            continue;

        if (entry->d_type == DT_DIR) {
            char newPath[256];
            strcpy(newPath, path);
            if (path[strlen(path) - 1] != '/')
                strcat(newPath, "/");
            strcat(newPath, entry->d_name);
            http_printf(fw, "<a href=\"%s\">/%s</a><br>", newPath, entry->d_name);
        }

        if (entry->d_type == DT_REG) {
            char newPath[256];
            strcpy(newPath, path);
            if (path[strlen(path) - 1] != '/')
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

/**
 * @brief 客户端请求动态页面时的处理函数
 *
 * @param fw 同上
 * @param path 待执行脚本文件的绝对路径
 */
void dynamic_handler(FILE* fw, char* path, char **envv)
{
    char buffer[MAX_FILE_SIZE] = { 0 };
    read_pipe(buffer, path, envv);
    http_send_headers(fw, "text/html");
    http_prints(fw, buffer);
    http_end(fw);
}

void http_handler(int fd)
{
    FILE* fr = fdopen(fd, "r");
    FILE* fw = fdopen(fd, "w");

    char req[MAX_REQ_SIZE];
    char* envv[2] = { NULL, NULL };
    char* http_path;
    http_path = http_parse_req(fr, req, sizeof(req));

    // 先将参数解析为envv，并将参数从URL中去除
    char *http_path_without_parm = (char *)malloc(MAX_REQ_SIZE);
    memset(http_path_without_parm, 0, MAX_REQ_SIZE);
    parse_url(http_path_without_parm, envv, http_path);

    // 若请求根路径，返回index.html
    if (strcmp(http_path_without_parm, "/") == 0) {
        strcat(http_path_without_parm, "index.html");
    }
    puts(http_path_without_parm);
    // Attention：sizeof 对一个堆内存中的字符指针作用时，只返回一个指针的大小
    char path_with_root[MAX_REQ_SIZE] = {0};
    strcpy(path_with_root, web_root);
    strcat(path_with_root, http_path_without_parm);

    // 若请求的url不存在，返回404界面
    struct stat info;
    if (stat(path_with_root, &info) == -1) {
        file_handler(fw, "/404.html");
        return;
    }
    
    // 获取请求url的起始字符串，查看是否是/app/...
    char head[6] = { 0 };
    strncpy(head, http_path, 5);
    // 动态页面
    if (strcmp(head, "/app/") == 0) {
        dynamic_handler(fw, path_with_root, envv);
        return;
    }
    
    // 静态页面
    if (S_ISREG(info.st_mode)) {
        file_handler(fw, http_path_without_parm);
        return;
    }
    if (S_ISDIR(info.st_mode)) {
        dir_handler(fw, http_path_without_parm);
        return;
    }
    
    free(http_path_without_parm);
    free(http_path);
    if(envv[0] != NULL)
        free(envv[0]);
}
