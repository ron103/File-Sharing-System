#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define FILE_REPO "./repository/"

void *handle_client(void *client_socket);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create the repository directory if it doesn't exist
    mkdir(FILE_REPO, 0777);

    // Socket creation
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind to the specified PORT
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for connections...\n");

    while (1) {
        // Accept incoming connections
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0) {
            perror("Accept");
            continue;
        }

        printf("New connection accepted.\n");

        // Create a new thread for each client
        pthread_t thread_id;
        int *pclient = malloc(sizeof(int));
        *pclient = new_socket;
        if (pthread_create(&thread_id, NULL, handle_client, pclient) != 0) {
            perror("Thread creation failed");
            free(pclient);
        }
    }

    close(server_fd);
    return 0;
}

void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        char command[BUFFER_SIZE], filename[BUFFER_SIZE];
        sscanf(buffer, "%s %s", command, filename);

        if (strcmp(command, "UPLOAD") == 0) {
            // Extract the base name from the file path
            char *base_filename = strrchr(filename, '/');
            if (base_filename) {
                base_filename++; // Skip the '/'
            } else {
                base_filename = filename; // Already just the name
            }

            // Receive file size
            long file_size;
            recv(sock, &file_size, sizeof(file_size), 0);
            file_size = ntohl(file_size);

            // Receive file data
            char filepath[BUFFER_SIZE];
            snprintf(filepath, BUFFER_SIZE, "%s%s", FILE_REPO, base_filename);
            FILE *fp = fopen(filepath, "wb");
            if (fp == NULL) {
                perror("File open error");
                continue;
            }

            long received = 0;
            while (received < file_size) {
                bytes_read = recv(sock, buffer, BUFFER_SIZE, 0);
                fwrite(buffer, 1, bytes_read, fp);
                received += bytes_read;
            }
            fclose(fp);
            printf("File %s uploaded.\n", base_filename);
        } else if (strcmp(command, "DOWNLOAD") == 0) {
            // Construct the file path
            char filepath[BUFFER_SIZE];
            snprintf(filepath, BUFFER_SIZE, "%s%s", FILE_REPO, filename);

            // Open the file for reading
            FILE *fp = fopen(filepath, "rb");
            if (fp == NULL) {
                perror("File not found");
                long file_size = 0;
                file_size = htonl(file_size);
                send(sock, &file_size, sizeof(file_size), 0);
                continue;
            }

            // Determine file size
            fseek(fp, 0L, SEEK_END);
            long file_size = ftell(fp);
            rewind(fp);

            // Send the file size
            long net_file_size = htonl(file_size);
            send(sock, &net_file_size, sizeof(net_file_size), 0);

            // Send the file data
            while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
                send(sock, buffer, bytes_read, 0);
            }
            fclose(fp);
            printf("File %s downloaded.\n", filename);
        } else if (strcmp(command, "LIST") == 0) {
            // List files in the repository
            FILE *fp = popen("ls " FILE_REPO, "r");
            if (fp == NULL) {
                perror("popen");
                continue;
            }
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                send(sock, buffer, strlen(buffer), 0);
            }
            pclose(fp);
            send(sock, "END_OF_LIST", strlen("END_OF_LIST"), 0);
        } else if (strcmp(command, "DELETE") == 0) {
            // Delete the specified file
            char filepath[BUFFER_SIZE];
            snprintf(filepath, BUFFER_SIZE, "%s%s", FILE_REPO, filename);
            if (remove(filepath) == 0) {
                send(sock, "File deleted", strlen("File deleted"), 0);
                printf("File %s deleted.\n", filename);
            } else {
                send(sock, "File not found", strlen("File not found"), 0);
            }
        } else {
            send(sock, "Invalid command", strlen("Invalid command"), 0);
        }
    }

    printf("Connection closed.\n");
    close(sock);
    return NULL;
}