#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "ZKP_verify.h"

#define PORT 8080

int server_fd, new_socket;
struct sockaddr_in address;
int addrlen = sizeof(address);

char buffer[1024] = {0};

mpz_t y;


void read_prover_commit(mpz_t c){
    read(new_socket, buffer, sizeof(buffer));
    printf("Received key commit: %s\n", buffer);
    mpz_set_str(c, buffer, 10);
    memset(buffer, '\0', 1024);
    return;
}

void challenge_prover(bool challenge){
    char response[2];
    response[0] = challenge + '0';
    response[1] = '\0';
    send(new_socket, response, strlen(response), 0);
    printf("Send challenge %s to key\n", response);
    memset(response, '\0', 2);
    return;
}

void prover_response(mpz_t response) {
    read(new_socket, buffer, sizeof(buffer));
    printf("Received key response: %s\n", buffer);
    mpz_set_str(response, buffer, 10);
    memset(buffer, '\0', 1024);
    return;
}

int main() {
    mpz_init(y);
    mpz_set_ui(y, 17);
    // 5^7 - 23 * 3396 = 17, x = 7, p = 23, g = 5
    
    char *response = "Hello from server";

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Define the server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    // Accept an incoming connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept");
        exit(EXIT_FAILURE);
    }

    bool res = ZKP_verify(y, read_prover_commit, challenge_prover, prover_response, 10);
    if (res == true){
        printf("accept\n");
    }
    else {
        printf("reject\n");
    }
    // Close the sockets
    close(new_socket);
    close(server_fd);
    return 0;
}
