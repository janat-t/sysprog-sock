#include "main.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int port = 10000;

/** Checks whether a file exists and it is a normal file. */
int exists_file(char *fname) {
    struct stat info;
    if (stat(fname, &info) == -1)
        return 0;
    return S_ISREG(info.st_mode);
}
/** Checks whether a file is executable. */
int is_file_executable(char *fname) { return access(fname, X_OK) == 0; }

/** Gets the filename part from a path. */
char *get_filename(char *path) {
    for (int i = strlen(path) - 1; i >= 0; i--) {
        if (path[i] == '/')
            return path + (i + 1);
    }
    return path;
}
/** Gets the extension part from a filename. */
char *get_extension(char *fname) {
    fname = get_filename(fname);
    for (int i = strlen(fname) - 1; i >= 0; i--) {
        if (fname[i] == '.')
            return fname + (i + 1);
    }
    return "";
}
/** Gets the MIME content type from an extension. */
char *get_content_type(char *ext) {
    if (strcmp(ext, "html") == 0)
        return "text/html";
    return "text/plain";
}
/** Sends the contents of a stream to another. */
void send_stream(FILE *fout, FILE *f) {
    for (;;) {
        char buf[BUFSIZ];
        int size = fread(buf, 1, BUFSIZ, f);
        if (size == 0)
            return;
        fwrite(buf, size, 1, fout);
    }
}
/** Erase line break characters */
char *chomp(char *line) {
    char *p = strchr(line, '\n');
    if (p)
        *p = '\0';
    p = strchr(line, '\r');
    if (p)
        *p = '\0';
    return line;
}

/** Ignore zombies */
void noop(int sig) {}
void detach_children(void) {
    struct sigaction act;
    act.sa_handler = noop;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART | SA_NOCLDWAIT;
    if (sigaction(SIGCHLD, &act, NULL) == -1)
        PERROR_DIE("sigaction");
}

/** Sends the status line and body. */
void send_status_body(FILE *fout, char *status, char *body) {
    fprintf(fout, "HTTP/1.0 %s\r\n", status);
    fprintf(fout, "Content-Type: text/html\r\n");
    fprintf(fout, "\r\n%s", body);
}
void send_404(FILE *fout) {
    send_status_body(fout, "404 Not Found",
                     "<title>404 Not Found</title><h1>404 Not Found</h1>\n");
}


void session(int fd, FILE *fout, FILE *fin, char *caddr, int cport) {
    char line[4096], method[1024], uri[1024], version[1024];

    // request line
    if (fgets(line, sizeof(line), fin) == NULL)
        return;
    sscanf(line, "%s %s %s", method, uri, version);
    LOG("method = '%s', uri = '%s', version = '%s'", method, uri, version);

    // header
    for (;;) {
        if (fgets(line, sizeof(line), fin) == NULL) {
            LOG("Illegal headers; disconnected.");
            return;
        }
        if (strcmp(line, "\r\n") == 0)
            break;
        LOG("[Header] %s", chomp(line));
    }

    // default behavior; you can rewrite these lines
    fprintf(fout, "HTTP/1.0 200 OK\r\n");
    fprintf(fout, "Content-Type: text/html\r\n");
    fprintf(fout, "\r\n");
    fprintf(fout, "<title>httpd</title>\r\n");
    fprintf(fout, "<p>Client: %s:%d</p>\r\n", caddr, cport);
    fprintf(fout, "<p>Request line: <samp>%s %s %s</samp></p>\r\n", method, uri, version);
    fprintf(fout, "\r\n");
}

void do_session(int fd, char *caddr, int cport) {
    FILE *fin = fdopen(fd, "rb");
    FILE *fout = fdopen(fd, "wb");
    session(fd, fout, fin, caddr, cport);
    fclose(fout);
    fclose(fin);
    close(fd); // unnecessary
    LOG("Connection closed.");
}

void serve() {
    int listfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listfd == -1)
        PERROR_DIE("socket");
    int optval = 1;
    if (setsockopt(listfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        PERROR_DIE("setsockopt");

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
        PERROR_DIE("bind");
    if (listen(listfd, 10) == -1)
        PERROR_DIE("listen");

    // ignore zombies
    detach_children();

    for (;;) {
        struct sockaddr_in caddr;
        unsigned int addrlen = sizeof(caddr);
        int connfd = accept(listfd, (struct sockaddr *)&caddr, &addrlen);
        if (connfd == -1)
            PERROR_DIE("accept");

        do_session(connfd, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
    }
}

void parse_options(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "qp:")) != -1) {
        switch (opt) {
        case 'q': /* -q: quiet */
            l_set_quiet(1);
            break;
        case 'p': /* -p N: port number */
            port = atoi(optarg);
            if (port <= 0)
                DIE("Invalid port number");
            break;
        case '?':
        default:
            fprintf(stderr, "Usage: %s [-q] [-p port]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

/** the entrypoint */
int main(int argc, char **argv) {
    parse_options(argc, argv);
    serve();
    return 0;
}
