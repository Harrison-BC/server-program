#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <signal.h>
#include "helperFunctions.c"

#define BUFFER_SIZE 2000

/**
 * Print a file to the client.
 * 
 * @param clientFd - the client socket
 * @param buffer   - the char array to store the file
 * @param fileName - the name of the file to print
 */
static void getTextFile(int clientFd, char *buffer, const char *fileName) {
    memset(buffer, '\0', BUFFER_SIZE);
    
    // display error if no name is specified for the file
    if (fileName[0] == '\0') {
        writer(clientFd, "SERVER 500 Get Error\n");
    } else {
        FILE *filePointer = fopen(fileName, "r");
        
        // if successful, read all lines into the buffer
        if (filePointer) {
            writer(clientFd, "SERVER 200 OK\n\n");
            
            fread(buffer, 1, BUFFER_SIZE, filePointer);
            fclose(filePointer);
            
            writer(clientFd, strcat(buffer, "\n\n"));
        } else {
            writer(clientFd, "SERVER 404 Not Found\n");
        }
    }
}

/**
 * Writes to a text file on the server
 * 
 * @param clientFd - the client socket
 * @param buffer   - the buffer to store the text file
 * @param fileName - the name of the file to write to
 */
static void putTextFile(int clientFd, char *buffer, const char *fileName) {
    FILE *filePointer = fopen(fileName, "w");
    
    // if successful, listen for any lines and copy these into the file
    if (filePointer) {
        int enterCount = 0;

        while (enterCount < 2) {
            memset(buffer, '\0', BUFFER_SIZE);

            ssize_t bytes = read(clientFd, buffer, BUFFER_SIZE);
            
            if (bytes <= 0) {
                close(clientFd);
                error("Error receiving message");
            }
            
            // if enter pressed, increment the count
            enterCount = buffer[0] == '\n' ? enterCount + 1 : 0;
            
            // print to the file and flush
            fprintf(filePointer, "%s", buffer);
            fflush(filePointer);
        }
        
        // close file and print success message
        fclose(filePointer);
        writer(clientFd, "SERVER 201 Created\n");
    } else {
        writer(clientFd, "SERVER 501 Put Error\n");
    }
}

/**
 * Function that reads from writes to the client.
 * Checks for specific commands and handles these accordingly.
 * 
 * @param clientFd - the client socket
 */
static void readWrite(int clientFd) {
    writer(clientFd, "Successfuly connected to the server...\n");
    char buffer[BUFFER_SIZE];
    char command[20];
    char fileName[25];

    // always check for new input
    while (1) {
        memset(command, '\0', sizeof(command));
        memset(fileName, '\0', sizeof(fileName));
        memset(buffer, '\0', sizeof(buffer));
        
        // get incoming message and store in buffer
        ssize_t bytes = read(clientFd, buffer, BUFFER_SIZE);
        
        if (bytes <= 0) {
            close(clientFd);
            error("Error receiving message");
        }

        printf("Received message: %s", buffer);

        // populate the command and fileName strings
        sscanf(buffer, "%s %s", command, fileName);
        
        // check what the command is
        if (strcasecmp(command, "bye") == 0) {
            printf("Closing connection with client\n");
            break;
        } else if (strcasecmp(command, "get") == 0) {
            getTextFile(clientFd, buffer, fileName);
        } else if (strcasecmp(command, "put") == 0) {
            putTextFile(clientFd, buffer, fileName);
        }
    }
}

/**
 * The main function that calls the creation of the socket, listening for a connection,
 * and then calls the readWrite function.
 * 
 * @param argc - number of arguments
 * @param argv - arguments
 * @return int - 0 if successful
 */
int main(int argc, char *argv[]) {
    int portNum;

    if (argc != 2) return EXIT_FAILURE;
    
    sscanf(argv[1], "%d", &portNum);
    if (portNum < 1024 || portNum > 49151) {
        return EXIT_FAILURE;
    }
    
    int sockFd = createSocket(portNum);
    listenForClients(sockFd);
    
    // always accept new connections
    while (1) {
        pid_t processId;
        int clientFd = acceptClients(sockFd);
        
        if ((processId = fork()) == 0) {
            close(sockFd);          // close socket as it is handled by the parent process
            readWrite(clientFd);
            break;
        } else if (processId > 0) {
            close(clientFd);        // close client as it is handled by the child process
        } else {
            error("Fork failed");
        }
    }

    return EXIT_SUCCESS;
}
