/*
Name       : Meet Patel
Student ID : 110123981
Section    : 2
*/
#include <stdio.h> // Include standard input/output library
#include <stdlib.h> // Include standard library
#include <string.h> // Include string manipulation functions
#include <unistd.h> // Include POSIX operating system API
#include <sys/socket.h> // Include socket-related functions and structures
#include <netinet/in.h> // Include internet address family structures
#include <arpa/inet.h> // Include definitions for internet operations
#include <fcntl.h> // Include file control options
#include <asm-generic/socket.h> // Include generic socket definitions
#include <dirent.h> // Include directory entry structure
#include <sys/stat.h> // Include data returned by the stat() system call
#include <errno.h> // Include error number definitions
#include <time.h> // Include time and date functions
#include <pwd.h> // Include password structure
#include <grp.h> // Include group structure
#include <sys/types.h> // Include data types used in system calls
#include <sys/wait.h> // Include declarations for waiting
#include <limits.h> // Include implementation-specific limits for system variables
#include <errno.h> // Include error number definitions (repeated)

#define m_pat_M_PORT 4002 // Define a macro for the server port number (4001)
#define m_pat_MX_CLTS 10 // Define a macro for the maximum number of clients (10)
#define m_pat_BUF_SZ 1024 // Define a macro for the buffer size (1024 bytes)

// Function to search for file information
void m_pat_src_fl_inf(int m_pat_clt_sck, const char *m_pat_fl_nmp, const char *m_pat_fol) {
    char m_pat_src_cmd[m_pat_BUF_SZ]; // Buffer to store the find command
    char m_pat_fl_inf[m_pat_BUF_SZ]; // Buffer to store file information
    FILE *m_pat_fp; // File pointer

    // Creating the find command to search for the file and get its details
    snprintf(m_pat_src_cmd, m_pat_BUF_SZ, "find %s -name \"%s\" -printf '%%f: %%s bytes, %%t, %%M\\n' -quit", m_pat_fol, m_pat_fl_nmp);

    // Executing the find command and reading the output
    m_pat_fp = popen(m_pat_src_cmd, "r"); // Opening a pipe to execute the command
    if (m_pat_fp == NULL) { // Checking if execution failed
        perror("Error!!!!! Command Execution Failed"); // Printing error message
        return;
    }

    memset(m_pat_fl_inf, 0, m_pat_BUF_SZ); // Clearing the file information buffer
    char m_pat_ln[m_pat_BUF_SZ]; // Buffer to read lines

    // Reading output line by line
    if (fgets(m_pat_ln, sizeof(m_pat_ln), m_pat_fp) != NULL) { // Checking if reading successful
        strcat(m_pat_fl_inf, m_pat_ln); // Appending file info to the result buffer
        printf("%s", m_pat_ln); // Printing the line on the server side
        send(m_pat_clt_sck, m_pat_fl_inf, strlen(m_pat_fl_inf), 0); // Sending file info to the client
    } else {
        char *m_pat_err_msg = "getfn - File not found\n"; // Error message for file not found
        send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0); // Sending error message to the client
    }
    pclose(m_pat_fp); // Closeing the file pointer
}

// Function to search and compress files within a specified size range
void m_pat_src_cmp_fl(int m_pat_clt_sck, long m_pat_sz_1, long m_pat_sz_2, const char *m_pat_fol) {
    char m_pat_src_cmd[m_pat_BUF_SZ]; // Buffer for the find and compress command
    char m_pat_tar_cmd[m_pat_BUF_SZ]; // Buffer for tar command

    setenv("m_pat_curr_fol", "/home/meet/Desktop/ASP/Project", 1); // Setting environment variable for the current folder

    // Getting the user's home folder
    const char *m_pat_pre_wrk_fol = getenv("m_pat_curr_fol");
    if (m_pat_pre_wrk_fol == NULL) { // Checking if getting the folder failed
        perror("getenv");
        return;
    }

    // Creating the find command to search for files within the size range and compress them into temp.tar.gz
    snprintf(m_pat_src_cmd, m_pat_BUF_SZ, "find %s ! -name \"temp.tar.gz\" -type f -size +%ldc -size -%ldc -print | tar -czvf temp.tar.gz -T - | sed 's/\\///'", m_pat_pre_wrk_fol, m_pat_sz_1, m_pat_sz_2);

    // Running the find command to compress files within the size range into temp.tar.gz
    system(m_pat_src_cmd); // Executeing the command

    // Checking if the archive temp.tar.gz was created
    FILE *m_pat_arcp = fopen("temp.tar.gz", "rb");
    if (m_pat_arcp) { // Checking if the file was successfully opened
        fseek(m_pat_arcp, 0, SEEK_END); // Moving file pointer to end
        long m_pat_fl_sz = ftell(m_pat_arcp); // Getting the file size
        fclose(m_pat_arcp); // Closing the file

        // If the archive has content, send it to the client
        if (m_pat_fl_sz > 0) {
            system("tar tf temp.tar.gz"); // Display the contents of temp.tar.gz
            send(m_pat_clt_sck, "temp.tar.gz", strlen("temp.tar.gz"), 0); // Sending the archive name to the client
            return;
        }
    }

    // If no files were found or added to the archive, send an error message
    char *m_pat_err_msg = "getfz - No files found in the specified size range.";
    send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0);
}

// Function to remove newline character from a string
void m_pat_rm_nw_chr(char *m_pat_strp) {
    int m_pat_len = strlen(m_pat_strp); // Getting the length of the string
    for (int i = 0; i < m_pat_len; i++) { // Iterating through the string
        if (m_pat_strp[i] == '\n') { // Checking for newline character
            m_pat_strp[i] = '\0'; // Replacing newline with null terminator
            return; // Stop after removing the first newline character
        }
    }
}

// Function to search and compress files with specified extensions
void m_pat_src_cmp_ext(int m_pat_clt_sck, char **m_pat_exts, int m_pat_no_exts, const char *m_pat_fol) {
    char m_pat_src_cmd[m_pat_BUF_SZ]; // Buffer for the find and compress command

    // Checking if temp.tar.gz exists and remove it if so
    if (access("temp.tar.gz", F_OK) != -1) {
        remove("temp.tar.gz"); // Removing the existing archive
    }

    // Constructing the find command to search for files with specified extensions
    snprintf(m_pat_src_cmd, m_pat_BUF_SZ, "find \"%s\" -type f ", m_pat_fol); // Initial find command

    for (int i = 0; i < m_pat_no_exts; i++) { // Looping through each extension
        if (i == 0) {
            strncat(m_pat_src_cmd, "\\( ", m_pat_BUF_SZ - strlen(m_pat_src_cmd) - 1); // Start grouping extensions
        } else {
            strncat(m_pat_src_cmd, " -o ", m_pat_BUF_SZ - strlen(m_pat_src_cmd) - 1); // OR between extensions
        }
        char m_pat_ext_src[m_pat_BUF_SZ]; // Buffer for current extension
        snprintf(m_pat_ext_src, m_pat_BUF_SZ, "-iname '*.%s'", m_pat_exts[i]); // Preparing extension pattern
        strncat(m_pat_src_cmd, m_pat_ext_src, m_pat_BUF_SZ - strlen(m_pat_src_cmd) - 1); // Add extension to the command
    }
    m_pat_rm_nw_chr(m_pat_src_cmd); // Removing trailing newline if any
    strncat(m_pat_src_cmd, "' \\) -print0 | xargs -0 tar -czf temp.tar.gz", m_pat_BUF_SZ - strlen(m_pat_src_cmd) - 1); // Add command to compress files

    // Running the concatenated command
    int m_pat_rslt = system(m_pat_src_cmd); // Executing the command

    if (m_pat_rslt == 0) { // Checking if command execution was successful
        // If successful, sending the file to the client
        FILE *m_pat_arcp = fopen("temp.tar.gz", "rb"); // Openning the created archive
        if (m_pat_arcp) { // Checking if the file was successfully opened
            fseek(m_pat_arcp, 0, SEEK_END); // Moving file pointer to end
            long m_pat_fl_sz = ftell(m_pat_arcp); // Getting the file size
            fclose(m_pat_arcp); // Closing the file

            // If the archive has content, send it to the client
            if (m_pat_fl_sz > 0) {
                send(m_pat_clt_sck, "temp.tar.gz", strlen("temp.tar.gz"), 0); // Sending the archive name to the client
                return;
            }
        }
    }

    // If no files were found or added to the archive or if there was an error, send an error message
    char *m_pat_err_msg = "getft - No file found for the specified extension(s)";
    send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0); // Send error message to the client
}

// Function to search and compress files created before a specified date
void m_pat_src_cmp_dt_bf(int m_pat_clt_sck, const char *m_pat_dt_str) {
    char m_pat_src_cmd[m_pat_BUF_SZ]; // Buffer for the find and compress command
    char m_pat_tar_cmd[m_pat_BUF_SZ]; // Buffer for tar command

    setenv("m_pat_curr_fol", "/home/meet/Desktop/ASP/Assignment4", 1); // Setting environment variable for the current folder

    // Getting the user's home folder
    const char *m_pat_pre_wrk_fol = getenv("m_pat_curr_fol");
    if (m_pat_pre_wrk_fol == NULL) { // Checking if getting the folder failed
        perror("getenv");
        return;
    }

    // Constructing the find command to search for files created before or on the given date and pass them to tar
    snprintf(m_pat_src_cmd, m_pat_BUF_SZ, "find %s -type f ! -newermt \"%s 23:59:59\" -print | tar -cvf temp.tar.gz -T -", m_pat_pre_wrk_fol, m_pat_dt_str);

    // Running the find command to compress files created before the given date into temp.tar.gz
    system(m_pat_src_cmd); // Execute the command

    // Checking if the archive temp.tar.gz was created
    FILE *m_pat_arcp = fopen("temp.tar.gz", "rb");
    if (m_pat_arcp) { // Checking if the file was successfully opened
        fseek(m_pat_arcp, 0, SEEK_END); // Moving file pointer to end
        long m_pat_fl_sz = ftell(m_pat_arcp); // Getting the file size
        fclose(m_pat_arcp); // Closing the file

        // If the archive has content, send it to the client
        if (m_pat_fl_sz > 0) {
            send(m_pat_clt_sck, "temp.tar.gz", strlen("temp.tar.gz"), 0); // Send the archive name to the client
            return;
        }
    }

    // If no files were found or added to the archive, send an error message
    char *m_pat_err_msg = "getfdb - No file found before the specified date";
    send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0); // Sending error message to the client
}

// Function to search and compress files created on or after a specified date
void m_pat_src_cmp_dt_af(int m_pat_clt_sck, char *m_pat_dt_str) {
    char m_pat_src_cmd[m_pat_BUF_SZ]; // Buffer for the find command
    char m_pat_tar_cmd[m_pat_BUF_SZ]; // Buffer for the tar command

    setenv("m_pat_curr_fol", "/home/meet/Desktop/ASP/Assignment3", 1); // Setting environment variable for the current folder

    // Getting the user's home folder
    const char *m_pat_pre_wrk_fol = getenv("m_pat_curr_fol");
    if (m_pat_pre_wrk_fol == NULL) { // Checking if getting the folder failed
        perror("getenv");
        return;
    }

    // Checking if temp.tar.gz exists and remove it if so
    if (access("temp.tar.gz", F_OK) != -1) {
        remove("temp.tar.gz"); // Removing the existing archive
    }

    // Creating the find command to search for files created on the given date or after and compress them into temp.tar.gz
    snprintf(m_pat_src_cmd, m_pat_BUF_SZ, "find %s -type f \\( -newermt '%s' -o -newermt '%s + 1 day' \\) -exec tar rf temp.tar {} +", m_pat_pre_wrk_fol, m_pat_dt_str, m_pat_dt_str);

    // Running the find command to compress files created on or after the given date into temp.tar
    system(m_pat_src_cmd); // Executing the command

    // Compressing the temp.tar into temp.tar.gz
    snprintf(m_pat_tar_cmd, m_pat_BUF_SZ, "gzip -f temp.tar"); // Compressing the created tar file
    system(m_pat_tar_cmd); // Executing the compression command

    // Checking if the archive temp.tar.gz was created
    FILE *m_pat_arcp = fopen("temp.tar.gz", "rb");
    if (m_pat_arcp) { // Checking if the file was successfully opened
        fseek(m_pat_arcp, 0, SEEK_END); // Moving file pointer to end
        long m_pat_fl_sz = ftell(m_pat_arcp); // Getting the file size
        fclose(m_pat_arcp); // Closing the file

        // If the archive has content, send it to the client
        if (m_pat_fl_sz > 0) {
            send(m_pat_clt_sck, "temp.tar.gz", strlen("temp.tar.gz"), 0); // Sending the archive name to the client
            return;
        }
    }

    // If no files were found or added to the archive, send an error message
    char *m_pat_err_msg = "getfda - No file found before the specified date";
    send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0); // Sending error message to the client
}

// Function to process client requests
void pclientrequest(int m_pat_clt_sck) {
    // Buffer to store received data
    char m_pat_buf[m_pat_BUF_SZ];
    // Variable to store the number of bytes read
    int m_pat_val_rd;

    while (1) {
        // Clear the buffer
        memset(m_pat_buf, 0, m_pat_BUF_SZ);
        // Reading data from client socket
        m_pat_val_rd = read(m_pat_clt_sck, m_pat_buf, m_pat_BUF_SZ - 1);
        // Checking if data reading encountered an error or connection closed
        if (m_pat_val_rd <= 0) {
            printf("=================> Connection closed or error reading data\n");
            break;
        }

        if ((strncmp(m_pat_buf, "getfn", 5)) == 0) {
            // Extracting file name from the request and fetch user's home directory to retrieve file information
            char *m_pat_fl_nmp = strtok(m_pat_buf + 6, "\n");
            // Sending the file information or an error message back to the client
            if (m_pat_fl_nmp != NULL) {
                struct passwd *pw = getpwuid(getuid());
                if (pw) {
                    char *m_pat_pre_wrk_fol = pw->pw_dir; // User's home directory
                    // Continue to receive new requests
                    m_pat_src_fl_inf(m_pat_clt_sck, m_pat_fl_nmp, m_pat_pre_wrk_fol);
                } else {
                    char *m_pat_err_msg = "Error!!!! Failed to get user's home directory";
                    send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0);
                }
            } else { // Handle invalid file requests
                char *m_pat_err_msg = "File request is not valid";
                send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0);
            }
            continue;
        } else if (strncmp(m_pat_buf, "getfz", 5) == 0) {
            // Extract size range from the request and fetch the current directory
            long m_pat_sz_1, m_pat_sz_2;
            // Search and compress files within the specified size range
            if (sscanf(m_pat_buf + 6, "%ld %ld", &m_pat_sz_1, &m_pat_sz_2) == 2 && m_pat_sz_1 <= m_pat_sz_2 && m_pat_sz_1 >= 0 && m_pat_sz_2 >= 0) {
                char *m_pat_rt_dir = getenv("PWD");
                // Send compressed file or an error message back to the client
                // Continue to receive new requests
                m_pat_src_cmp_fl(m_pat_clt_sck, m_pat_sz_1, m_pat_sz_2, m_pat_rt_dir);
            } else { // Handle invalid size range
                char *m_pat_err_msg = "Size range is not valid";
                send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0);
            }
            continue;
        } else if (strncmp(m_pat_buf, "getft", 5) == 0) {
            // Extract file extensions from the request and fetch the current directory
            char *m_pat_exts[3]; // Assuming maximum of 3 m_pat_exts
            int m_pat_no_exts = 0;
            char *m_pat_tk = strtok(m_pat_buf + 6, " "); // Skip "getft" and space
            // Search and compress files based on specified extensions
            while (m_pat_tk != NULL && m_pat_no_exts < 3) {
                m_pat_exts[m_pat_no_exts] = m_pat_tk;
                //printf("============> %s\n", m_pat_tk);
                m_pat_no_exts++;
                m_pat_tk = strtok(NULL, " ");
            } // Send compressed file or an error message back to the client
            if (m_pat_no_exts>3) {
                // Handle invalid extension count
                char *m_pat_err_msg = "More than 3 extensions are not allowed";
                send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0);
            }
            // Continue to receive new requests
            setenv("m_pat_curr_fol", "/home/meet/Desktop/ASP/Project", 1);
            // Get the user's home directory
            const char *m_pat_pre_wrk_fol = getenv("m_pat_curr_fol");
            if (m_pat_pre_wrk_fol == NULL) {
                perror("getenv");
                return;
            }
            // Call function to search and compress by extension
            m_pat_src_cmp_ext(m_pat_clt_sck, m_pat_exts, m_pat_no_exts, m_pat_pre_wrk_fol);
            continue;
        } else if (strncmp(m_pat_buf, "getfdb", 6) == 0) {
            // Extract date from the request and fetch the current directory
            char *m_pat_dt_str = strtok(m_pat_buf + 7, "\n");
            if (m_pat_dt_str != NULL) {
                char *m_pat_pwd = getenv("PWD");
                // Search and compress files created before the specified date
                m_pat_src_cmp_dt_bf(m_pat_clt_sck, m_pat_dt_str);   // Continue to receive new requests
            } else {
                // Send compressed file or an error message back to the client
                char *m_pat_err_msg = "Invalid date request.";
                // Handle invalid date request
                send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0);
            }
            continue;
        } else if (strncmp(m_pat_buf, "getfda", 6) == 0) {
            // Extract date from the request and fetch the current directory
            char *m_pat_dt_str = strtok(m_pat_buf + 7, "\n");
            if (m_pat_dt_str != NULL) {
                // Search and compress files created on or after the specified date
                char *m_pat_pwd = getenv("PWD");
                // Send compressed file or an error message back to the client
                m_pat_src_cmp_dt_af(m_pat_clt_sck, m_pat_dt_str);   // Continue to receive new requests
            } else {
                // Handle invalid date request
                char *m_pat_err_msg = "Invalid date request.";
                send(m_pat_clt_sck, m_pat_err_msg, strlen(m_pat_err_msg), 0);
            }
            continue;
        }
        send(m_pat_clt_sck, m_pat_buf, strlen(m_pat_buf), 0);
        if (strcmp(m_pat_buf, "quitc\n") == 0) {
            break;
        }
    }
    close(m_pat_clt_sck);
}

// Main function to create a server and handle client connections
int main() {
    int m_pat_S_scfd, m_pat_C_nsc; // Server and client socket file descriptors
    struct sockaddr_in m_pat_scadd; // Server address structure
    int m_pat_opt = 1; // Socket option value
    int m_pat_scadd_len = sizeof(m_pat_scadd); // Length of server address structure
    int m_pat_con_clts = 0; // Track the number of connected clients

    // Creating a socket
    if ((m_pat_S_scfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error!!!! Socket not created");
        exit(EXIT_FAILURE);
    }

    // Setting socket options to reuse address and port
    if (setsockopt(m_pat_S_scfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &m_pat_opt, sizeof(m_pat_opt))) {
        perror("Error!!!! setsockopt not working");
        exit(EXIT_FAILURE);
    }

    // Setting up server address
    m_pat_scadd.sin_family = AF_INET; // IPv4
    m_pat_scadd.sin_addr.s_addr = INADDR_ANY; // Accepting connections from any available interface
    m_pat_scadd.sin_port = htons(m_pat_M_PORT); // Setting the server port

    // Binding the socket to the server address
    if (bind(m_pat_S_scfd, (struct sockaddr *)&m_pat_scadd, sizeof(m_pat_scadd)) < 0) {
        perror("Error!!!! Binding not working");
        exit(EXIT_FAILURE);
    }

    // Listening for incoming connections
    if (listen(m_pat_S_scfd, m_pat_MX_CLTS) < 0) {
        perror("Error!!!!! Listen not working");
        exit(EXIT_FAILURE);
    }

    printf("Mirror started on PORT %d\n", m_pat_M_PORT);
    printf("Waiting for the client to be connected...\n");

    // Accepting and handling incoming client connections
    while (1) {
        // Accepting client connection
        if ((m_pat_C_nsc = accept(m_pat_S_scfd, (struct sockaddr *)&m_pat_scadd, (socklen_t*)&m_pat_scadd_len)) < 0) {
            perror("Error!!!! Accept not working");
            exit(EXIT_FAILURE);
        }

        printf("Connection established with a client!\n");

        // Forking a child process to handle client requests
        int pid = fork();
        if (pid == 0) {
            close(m_pat_S_scfd); // Closing server socket in child process
            pclientrequest(m_pat_C_nsc); // Processing client requests
            printf("Client disconnected!\n");
            exit(0); // Exiting child process
        } else if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        close(m_pat_C_nsc); // Closing client socket in parent process
    }
    return 0;
}