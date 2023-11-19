#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include <pthread.h> // For POSIX threads
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

// Function designed for handling each client request
void *handle_request(void *arg) {
    int connfd = *((int *)arg);
    char filename[MAX];
    char buffer[MAX];
    int n;

    // Read the file name sent by the client
    read(connfd, filename, sizeof(filename));
    filename[strcspn(filename, "\n")] = '\0'; // Remove newline character if present

    printf("File requested by client: %s\n", filename);

    FILE *fileptr;
    fileptr = fopen(filename, "r");
    if (fileptr == NULL) {
        printf("File not found or unable to open the file.\n");
        write(connfd, "File not found or unable to open the file.", sizeof("File not found or unable to open the file."));
        close(connfd);
        pthread_exit(NULL);
    }

    // Calculate the file size
    fseek(fileptr, 0L, SEEK_END);
    int file_size = ftell(fileptr);
    rewind(fileptr);

    // Send the file size to the client followed by a flag
    char size_msg[MAX];
    snprintf(size_msg, sizeof(size_msg), "%d$", file_size);
    write(connfd, size_msg, sizeof(size_msg));

    // Send the file contents to the client
    while ((n = fread(buffer, sizeof(char), sizeof(buffer), fileptr)) > 0) {
        if (write(connfd, buffer, n) < 0) {
            printf("Error sending file.\n");
            break;
        }
    }

    fclose(fileptr); // Close the file
    close(connfd);   // Close the connection

    printf("File sent to client.\n");
    pthread_exit(NULL);
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    socklen_t len;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else
        printf("Server listening..\n");

    len = sizeof(cli);

    while (1) {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        } else
            printf("server accept the client...\n");

        // Create a thread to handle the client request
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_request, &connfd) != 0) {
            printf("Failed to create thread for handling request.\n");
            close(connfd);
        }
    }

    // close the socket
    close(sockfd);

    return 0;
}
