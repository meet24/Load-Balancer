/*
Name       : Meet Patel
Student ID : 110123981
Section    : 2
*/
#include <stdio.h> // Standard input-output library
#include <stdlib.h> // Standard library
#include <string.h> // String manipulation functions
#include <unistd.h> // Symbolic constants and types (POSIX operating system API)
#include <sys/socket.h> // Socket programming functions and structures
#include <netinet/in.h> // Internet address family structures and constants
#include <arpa/inet.h> // Functions for manipulating numeric IP addresses
#include <linux/limits.h> // Constants defining maximum values for various path-related variables
#include <errno.h> // Error handling macros and functions

#define m_pat_S_PORT 4001 // Define server port number
#define m_pat_M_PORT 4002 // Define another port number
#define m_pat_BUF_SZ 1024 // Define buffer size for data handling
#define CLIENT_COUNT_FILE "client_count.txt" // Define the file name for client count

// Function to receive and process response from the server or mirror
void m_pat_res(int m_pat_clt_sck) {
    char m_pat_buf[m_pat_BUF_SZ]; // Buffer to store received data
    memset(m_pat_buf, 0, m_pat_BUF_SZ); // Clear the buffer

    // Reading data from the client socket
    int m_pat_val_rd = read(m_pat_clt_sck, m_pat_buf, m_pat_BUF_SZ - 1);
    if (m_pat_val_rd <= 0) { // Checking for connection closure or error in data reading
        printf("Connection closed or error reading data.\n");
        close(m_pat_clt_sck); // Closing client socket
        exit(EXIT_FAILURE); // Exiting the program with failure status
    } else {
        printf("\nReceived information:\n%s\n", m_pat_buf); // Printing received information

        // Processing response if needed, e.g., handle "temp.tar.gz" file
        if (strstr(m_pat_buf, "temp.tar.gz") != NULL) {
            system("mkdir -p ~/f23project"); // Creating a directory if not existing
            system("cp temp.tar.gz ~/f23project/"); // Copying the received file to the directory
        }
        if (strcmp(m_pat_buf, "quitc\n") == 0) { // Checking for quit command from the server
            exit(EXIT_FAILURE); // Exiting the program with failure status
        }
    }
}

// Main function to handle load balancing
int main() {
    struct sockaddr_in m_pat_S_addr, m_pat_M_addr; // Server and Mirror addresses
    int m_pat_cur_clt = 0; // Current client count

    // Reading client count from file
    FILE *m_pat_fl = fopen(CLIENT_COUNT_FILE, "r");
    if (m_pat_fl != NULL) {
        fscanf(m_pat_fl, "%d", &m_pat_cur_clt); // Reading the count
        fclose(m_pat_fl);
    }

    m_pat_cur_clt++; // Incrementing the client count

    // Updating client count in the file
    m_pat_fl = fopen(CLIENT_COUNT_FILE, "w");
    if (m_pat_fl != NULL) {
        fprintf(m_pat_fl, "%d", m_pat_cur_clt); // Writing the updated count
        fclose(m_pat_fl);
    } else {
        printf("Error!!!!! File not opened\n"); // Error if file couldn't be opened
        return -1;
    }

    int m_pat_clt_sck = 0; // Client socket
    int m_pat_con_ser; // Connection flag for server or mirror

    // Deciding connection to server or mirror based on client count
    if (m_pat_cur_clt <= 8) {
        m_pat_con_ser = (m_pat_cur_clt - 1) / 4 % 2 == 0 ? 1 : 0; // Connect to server
    } else {
        m_pat_con_ser = (m_pat_cur_clt - 8) % 2 != 0 ? 1 : 0; // Connect to mirror
    }

    // Connecting to server or mirror based on the flag
    if (m_pat_con_ser) {
        // Connect to the server
        // Creating a socket
        if ((m_pat_clt_sck = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error!!! Socket not created");
            return -1;
        }

        // Server address configuration
        m_pat_S_addr.sin_family = AF_INET;
        m_pat_S_addr.sin_port = htons(m_pat_S_PORT);

        // Convert IPv4 addresses from text to binary form
        if (inet_pton(AF_INET, "127.0.0.1", &m_pat_S_addr.sin_addr) <= 0) {
            perror("Error!!! Invalid IP address/ Address not supported");
            return -1;
        }

        // Connect to the server
        if (connect(m_pat_clt_sck, (struct sockaddr *)&m_pat_S_addr, sizeof(m_pat_S_addr)) < 0) {
            perror("Connection Failed");
            return -1;
        }

        printf("Connected to the server! Client: %d\n", m_pat_cur_clt); // Connection successful message
    } else {
        // Connect to the mirror
        // Creating a socket
        if ((m_pat_clt_sck = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error!!!! Socket not created");
            return -1;
        }

        // Mirror address configuration
        m_pat_M_addr.sin_family = AF_INET;
        m_pat_M_addr.sin_port = htons(m_pat_M_PORT);

        // Convert IPv4 addresses from text to binary form
        if (inet_pton(AF_INET, "127.0.0.1", &m_pat_M_addr.sin_addr) <= 0) {
            perror("Error!!! Invalid IP address/ Address not supported");
            return -1;
        }

        // Connect to the mirror
        if (connect(m_pat_clt_sck, (struct sockaddr *)&m_pat_M_addr, sizeof(m_pat_M_addr)) < 0) {
            perror("Connection Failed");
            return -1;
        }

        printf("Connected to the mirror! Client: %d\n", m_pat_cur_clt); // Connection successful message
    }

    // Command handling loop
    while (1) {
        char m_pat_buf[m_pat_BUF_SZ]; // Buffer to store user input
        memset(m_pat_buf, 0, m_pat_BUF_SZ); // Clear the buffer
        printf("\nSyntax of Commands:\n 1. getfn FILENAME\n 2. getfz SIZE1 SIZE2\n 3. getft EXT1 EXT2 EXT3\n 4. getfdb YYYY-MM-DD\n 5. getfda YYYY-MM-DD\n 6. quitc\n");
        printf("\nEnter Command: ");
        fgets(m_pat_buf, m_pat_BUF_SZ - 1, stdin); // Reading user input
        send(m_pat_clt_sck, m_pat_buf, strlen(m_pat_buf), 0); // Sending the command to server/mirror
        m_pat_res(m_pat_clt_sck); // Processing the response
    }

    close(m_pat_clt_sck); // Close client socket

    return 0;
}
