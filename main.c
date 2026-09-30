#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

/* A Basic HTTP 1.0 Server */
/* Samuel-Karim Benhamou */

struct request {
    char *type;
    char *path;
};
bool parse_request(char* buf, struct request *req) {
    char* curr_tok = buf;
    char *delim = " ";
    curr_tok = strtok(curr_tok, delim);
    if(curr_tok == NULL || strncmp(curr_tok, "GET", 3) != 0) {
        return false;
    } else {
        req->type = curr_tok;
        curr_tok = strtok(NULL, delim);
        req->path = curr_tok;
        if(curr_tok == NULL) {
            return false;
        }
        return true;
    }
}

const char *get_mime_type(const char *path) {
    char *ext = strrchr(path, '.');
    if(!ext) {
      return "html/plain";
    } // text, image, application
    if(strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
        return "text/html";
    } else if (strcmp(ext, ".css") == 0) {
        return "text/css";
    } else if (strcmp(ext, ".js") == 0) {
        return "application/js";
    } else if (strcmp(ext, ".png") == 0) {
        return "image/png";
    } else if (strcmp(ext, ".webp") == 0) {
        return "image/webp";
    } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        return "image/jpg";
    }
    return "text/plain";
}
int main() {
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = INADDR_ANY;

    bind(serverfd, (struct sockaddr *) &address, sizeof(address));
    listen(serverfd, 10);
    printf("Listening on port 8080...\n");
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(serverfd, (struct sockaddr *) &client_addr, &client_len);
        if (client_fd < 0) {
            printf("doodoo error\n");
            continue;
        }
        printf("Connection accepted!\n");
        char buf[500];
        ssize_t read_bytes = recv(client_fd, (void *) buf, sizeof(buf) - 1, 0);
        if (read_bytes > 0) {
            printf("%s\n", buf);
            buf[read_bytes] = '\0';
            struct request new_req;
            if(parse_request(buf, &new_req)) {
                printf("Request Type: %s\n", new_req.type);
                printf("Request Path: %s\n\n", new_req.path);
                char file_path[512];
                snprintf(file_path, sizeof(file_path), "./public%s", new_req.path);
                // to-do:
                // send the file to the socket (client_fd)
                // close
                char *path = new_req.path;
                if(strcmp(path, "/") == 0) {
                    snprintf(file_path, sizeof(file_path), "./public/index/html");
                }
                struct stat check_file;
                int success = stat(file_path, &check_file);
                if(success < 0 || S_ISDIR(check_file.st_mode)) {
                  printf("404 file not found");
                  return -1;
                }
                const char *file_type = get_mime_type(file_path); 
                long file_size = check_file.st_size;
                char response[1024];
                snprintf(response, sizeof(response), 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    file_type, file_size
                );
                printf("%s\n", response);
                // phase 5: concurrency (when calling accept, spawn new thread)
            } else {
                printf("Failed to parse HTTP request.\n");
            }
        }
        close(client_fd);
    }
    return 0;
}
