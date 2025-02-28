#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ZKP_proof.h"
#define PORT 8080

int sock = 0;
struct sockaddr_in serv_addr;
char *message = "Hello from client";
char buffer[1024] = {0};

mpz_t x;

void commit_to_verifier(mpz_t C){
    mpz_get_str(buffer, 10, C);
    send(sock, buffer, strlen(buffer), 0);
    printf("Commit %s to host\n", buffer);
    memset(buffer, '\0', 1024);
}

void read_challenge(int *challenge){
    read(sock, buffer, sizeof(buffer));
    printf("Host challenges: %s\n", buffer);
    switch (buffer[0])
    {
    case '0':
        *challenge = 0;
        break;
    case '1':
        *challenge = 1;
        break;
    
    default:
        *challenge = 2;
        break;
    }
    memset(buffer, '\0', 1024);
    return;
}

void respond_to_challenge(mpz_t Response){
    mpz_get_str(buffer, 10, Response);
    send(sock, buffer, strlen(buffer), 0);
    printf("Response challenge with %s to host\n", buffer);
    memset(buffer, '\0', 1024);
}

int main() {
    mpz_init(x);
    mpz_set_ui(x, 7);

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    ZKP_proof(x, commit_to_verifier, read_challenge, respond_to_challenge);

    // Close the socket
    close(sock);
    return 0;
}
