#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void receiveFile(int sockfd) {
    char buffer[MAX];
    int file_size = 0;
    int bytes_received = 0;

    // Read the file size from the server
    read(sockfd, buffer, sizeof(buffer));
    file_size = atoi(buffer); // Convert string to integer for file size

    printf("File size received from server: %d bytes\n", file_size);

    // Read the file content from the server
    printf("File content received from server:\n");

    while (bytes_received < file_size) {
        bzero(buffer, MAX);
        int bytes_to_read = (file_size - bytes_received) > MAX ? MAX : (file_size - bytes_received);
        int bytes = read(sockfd, buffer, bytes_to_read);
        if (bytes <= 0) {
            printf("Error in reading the file from server.\n");
            break;
        }
        bytes_received += bytes;

        // Print the received content (assuming it's text-based)
        printf("%.*s", bytes, buffer);
    }

    printf("\nFile received from server.\n");
}

void requestFile(int sockfd, const char *filename) {
    // Send the filename to the server
    write(sockfd, filename, strlen(filename) + 1);

    // Receive file from the server
    receiveFile(sockfd);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char filename[MAX];

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else
        printf("Socket Bound\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    } else
        printf("connected to the server..\n");

    while (1) {

        // Request a file from the server
        printf("Enter the filename to request from the server (or type 'exit' to quit): ");
        fgets(filename, MAX, stdin);
        filename[strcspn(filename, "\n")] = '\0'; // Remove newline character if present

        // Check if the user wants to exit
        if (strcmp(filename, "exit") == 0) {
            close(sockfd);
            break;
        }

        // Send the filename to the server and receive the file
        requestFile(sockfd, filename);

        // close the socket
        close(sockfd);
    }

    return 0;
}
