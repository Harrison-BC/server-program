#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/**
 * Prints an error message to the terminal and exits the program.
 *
 * @param msg - the error message to print
 */
void error(const char *msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

/**
 * Function that writes to the client and checks if it was successful.
 *
 * @param clientFd — the client socket
 * @param msg — the error message to print
 */
void writer(int clientFd, const char *msg) {
    char newMsg[2000 + strlen("Server: ")] = "Server: ";
    strcat(newMsg, msg);

    ssize_t bytes = write(clientFd, newMsg, strlen(newMsg));
    
    if (bytes < 0) {
        error("failed writing to client\n");
    }
}

/**
 * Function that creates the socket.
 * 
 * @param portNum - the port to link the server address to
 * @return int - the file descriptor of the server socket
 */
int createSocket(int portNum) {
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockFd < 0) {
        error("Socket could not be created\n");
    }
    
    // create address
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(portNum);
        
    // bind
    int bindReturn = bind(sockFd, (struct sockaddr *) &servAddr, sizeof(servAddr));
    
    if (bindReturn < 0) {
        error("Bind failed\n");
    }

    return sockFd;
}

/**
 * Prepares server for incoming connection requests.
 * 
 * @param sockFd - the server socket file descriptor
 */
void listenForClients(int sockFd) {
    // Listen for incoming connections
    if (listen(sockFd, SOMAXCONN) < 0) {
        error("Error listening for connections");
    }
    printf("Listening for client requests\n");
}

/**
 * Function that listens for an incoming connection and accepts it.
 * 
 * @param sockFd - the server socket file descriptor
 */
int acceptClients(int sockFd) {
    // accept the connection
    struct sockaddr_in clientAddr;
    int clientLen = sizeof(clientLen);
    
    int clientFd = accept(sockFd, (struct sockaddr *) &clientAddr, (socklen_t*) &clientLen);
    
    if (clientFd < 0) {
        error("Could not accept\n");
    }
    
    printf("Connection established\n");
    
    return clientFd;
}