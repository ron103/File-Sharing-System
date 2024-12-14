Multi-Client File Server Application

Features

•	Upload: Clients can upload files to the server’s repository.
•	Download: Clients can download files from the server.
•	List: View all files stored in the server’s repository.
•	Delete: Remove specific files from the server’s repository.


Compile and Run

1. Compile the Server and Client
make  (which performs below commands)
    gcc -pthread server.c -o server
    gcc client.c -o client
    ./server

2. Start the Server
(Hardcoded to port 8080)
./server

3. Start a Client

In another terminal, run:

./client

You can run multiple clients simultaneously.

Usage

Commands Supported by the Client

1.	UPLOAD:
Upload a file to the server.

UPLOAD /path/to/filename


2.	DOWNLOAD:
Download a file from the server.

DOWNLOAD filename


3.	LIST:
List all files stored on the server.

LIST


4.	DELETE:
Delete a file from the server.

DELETE filename


5.	EXIT:
Disconnect from the server.

EXIT

Example Usage

Enter command (UPLOAD, DOWNLOAD, LIST, DELETE, EXIT): UPLOAD /Users/user/Documents/test.txt
File test.txt uploaded.

Enter command (UPLOAD, DOWNLOAD, LIST, DELETE, EXIT): LIST
Files on server:
test.txt

Enter command (UPLOAD, DOWNLOAD, LIST, DELETE, EXIT): DOWNLOAD test.txt
File test.txt downloaded.

Enter command (UPLOAD, DOWNLOAD, LIST, DELETE, EXIT): DELETE test.txt
File deleted.

Enter command (UPLOAD, DOWNLOAD, LIST, DELETE, EXIT): LIST
Files on server: