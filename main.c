#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>

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
                printf("Request Path: %s\n", new_req.path);
                // to-do:
                // phase 4: serve files (use stat() to check that the file exists, else 404 file not found)
                // inspect file type and set appropriate mime header (text/html, text/css, etc.)
                // build http response
                // send the file to the socket (client_fd)
                // close
                //
                // phase 5: concurrency (when calling accept, spawn new thread)
            } else {
                printf("Failed to parse HTTP request.\n");
            }
        }
        close(client_fd);
    }
    return 0;
}
