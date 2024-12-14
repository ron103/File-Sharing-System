#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void upload_file(int sock, char *filename);
void download_file(int sock, char *filename);
void list_files(int sock);
void delete_file(int sock, char *filename);

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Server address setup
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 addresses from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to the server.\n");

    while (1) {
        printf("\nEnter command (UPLOAD, DOWNLOAD, LIST, DELETE, EXIT): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline

        char command[BUFFER_SIZE], filename[BUFFER_SIZE];
        sscanf(buffer, "%s %s", command, filename);

        if (strcmp(command, "UPLOAD") == 0) {
            send(sock, buffer, strlen(buffer), 0);
            upload_file(sock, filename);
        } else if (strcmp(command, "DOWNLOAD") == 0) {
            send(sock, buffer, strlen(buffer), 0);
            download_file(sock, filename);
        } else if (strcmp(command, "LIST") == 0) {
            send(sock, buffer, strlen(buffer), 0);
            list_files(sock);
        } else if (strcmp(command, "DELETE") == 0) {
            send(sock, buffer, strlen(buffer), 0);
            delete_file(sock, filename);
        } else if (strcmp(command, "EXIT") == 0) {
            break;
        } else {
            printf("Invalid command.\n");
        }
    }

    close(sock);
    return 0;
}

void upload_file(int sock, char *filename) {
    FILE *fp = fopen(filename, "rb");
    char buffer[BUFFER_SIZE];

    if (fp == NULL) {
        perror("File open error");
        return;
    }

    // Get file size
    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    long net_file_size = htonl(file_size);
    send(sock, &net_file_size, sizeof(net_file_size), 0);

    // Send file data
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(sock, buffer, bytes_read, 0);
    }
    fclose(fp);
    printf("File %s uploaded.\n", filename);
}

void download_file(int sock, char *filename) {
    // Receive file size
    long file_size;
    recv(sock, &file_size, sizeof(file_size), 0);
    file_size = ntohl(file_size);

    if (file_size == 0) {
        printf("File not found on server.\n");
        return;
    }

    // Receive file data
    FILE *fp = fopen(filename, "wb");
    char buffer[BUFFER_SIZE];
    long received = 0;
    int bytes_read;

    while (received < file_size) {
        bytes_read = recv(sock, buffer, BUFFER_SIZE, 0);
        fwrite(buffer, 1, bytes_read, fp);
        received += bytes_read;
    }
    fclose(fp);
    printf("File %s downloaded.\n", filename);
}

void list_files(int sock) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    printf("Files on server:\n");
    while ((bytes_read = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (strcmp(buffer, "END_OF_LIST") == 0) {
            break;
        }
        printf("%s", buffer);
    }
}

void delete_file(int sock, char *filename) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    bytes_read = recv(sock, buffer, BUFFER_SIZE, 0);
    buffer[bytes_read] = '\0';
    printf("%s\n", buffer);
}