#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 9000
#define DATA_FILE "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 1024

static int server_fd = -1;
static int client_fd = -1;
static volatile sig_atomic_t shutdown_requested = 0;

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        shutdown_requested = 1;
    }
}

void cleanup_and_exit(int exit_code) {
    if (client_fd != -1) {
        close(client_fd);
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    
    // Remove the data file
    if (unlink(DATA_FILE) == -1 && errno != ENOENT) {
        syslog(LOG_ERR, "Failed to remove data file: %s", strerror(errno));
    }
    
    closelog();
    exit(exit_code);
}

int setup_signal_handlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Failed to setup SIGINT handler: %s", strerror(errno));
        return -1;
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Failed to setup SIGTERM handler: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

int create_daemon() {
    pid_t pid = fork();
    
    if (pid < 0) {
        syslog(LOG_ERR, "Failed to fork: %s", strerror(errno));
        return -1;
    }
    
    if (pid > 0) {
        // Parent process exits
        exit(0);
    }
    
    // Child process continues
    if (setsid() == -1) {
        syslog(LOG_ERR, "Failed to create new session: %s", strerror(errno));
        return -1;
    }
    
    // Change working directory to root
    if (chdir("/") == -1) {
        syslog(LOG_ERR, "Failed to change directory: %s", strerror(errno));
        return -1;
    }
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Redirect to /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
    
    return 0;
}

int setup_server_socket() {
    struct sockaddr_in address;
    int opt = 1;
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        syslog(LOG_ERR, "Failed to create socket: %s", strerror(errno));
        return -1;
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        syslog(LOG_ERR, "Failed to set socket options: %s", strerror(errno));
        return -1;
    }
    
    // Setup address structure
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
        return -1;
    }
    
    // Listen for connections
    if (listen(server_fd, 1) == -1) {
        syslog(LOG_ERR, "Failed to listen on socket: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

int append_to_file(const char *data, size_t length) {
    FILE *file = fopen(DATA_FILE, "a");
    if (!file) {
        syslog(LOG_ERR, "Failed to open data file: %s", strerror(errno));
        return -1;
    }
    
    if (fwrite(data, 1, length, file) != length) {
        syslog(LOG_ERR, "Failed to write to data file: %s", strerror(errno));
        fclose(file);
        return -1;
    }
    
    fclose(file);
    return 0;
}

int send_file_contents(int client_socket) {
    FILE *file = fopen(DATA_FILE, "r");
    if (!file) {
        if (errno == ENOENT) {
            // File doesn't exist, nothing to send
            return 0;
        }
        syslog(LOG_ERR, "Failed to open data file for reading: %s", strerror(errno));
        return -1;
    }
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        ssize_t bytes_sent = send(client_socket, buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            syslog(LOG_ERR, "Failed to send data to client: %s", strerror(errno));
            fclose(file);
            return -1;
        }
        if ((size_t)bytes_sent != bytes_read) {
            syslog(LOG_ERR, "Partial send to client");
            fclose(file);
            return -1;
        }
    }
    
    if (ferror(file)) {
        syslog(LOG_ERR, "Error reading data file: %s", strerror(errno));
        fclose(file);
        return -1;
    }
    
    fclose(file);
    return 0;
}

int handle_client_connection(int client_socket, struct sockaddr_in *client_addr) {
    char *buffer = NULL;
    size_t buffer_size = 0;
    size_t buffer_used = 0;
    char recv_buffer[BUFFER_SIZE];
    
    // Log connection acceptance
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    syslog(LOG_INFO, "Accepted connection from %s", client_ip);
    
    while (!shutdown_requested) {
        ssize_t bytes_received = recv(client_socket, recv_buffer, sizeof(recv_buffer) - 1, 0);
        
        if (bytes_received == -1) {
            if (errno == EINTR) {
                continue; // Interrupted by signal, try again
            }
            syslog(LOG_ERR, "Failed to receive data: %s", strerror(errno));
            break;
        }
        
        if (bytes_received == 0) {
            // Client closed connection
            break;
        }
        
        recv_buffer[bytes_received] = '\0';
        
        // Expand buffer if needed
        if (buffer_used + bytes_received >= buffer_size) {
            size_t new_size = buffer_size + BUFFER_SIZE;
            char *new_buffer = realloc(buffer, new_size);
            if (!new_buffer) {
                syslog(LOG_ERR, "Failed to allocate memory: %s", strerror(errno));
                break;
            }
            buffer = new_buffer;
            buffer_size = new_size;
        }
        
        // Copy received data to buffer
        memcpy(buffer + buffer_used, recv_buffer, bytes_received);
        buffer_used += bytes_received;
        
        // Check for newline character
        char *newline_pos = memchr(buffer, '\n', buffer_used);
        if (newline_pos) {
            // Found complete packet
            size_t packet_length = newline_pos - buffer + 1;
            
            // Append to file
            if (append_to_file(buffer, packet_length) == -1) {
                break;
            }
            
            // Send file contents back to client
            if (send_file_contents(client_socket) == -1) {
                break;
            }
            
            // Remove processed packet from buffer
            size_t remaining = buffer_used - packet_length;
            if (remaining > 0) {
                memmove(buffer, buffer + packet_length, remaining);
            }
            buffer_used = remaining;
        }
    }
    
    // Log connection closure
    syslog(LOG_INFO, "Closed connection from %s", client_ip);
    
    if (buffer) {
        free(buffer);
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    int daemon_mode = 0;
    
    // Parse command line arguments
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    } else if (argc > 1) {
        fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
        return 1;
    }
    
    // Open syslog
    openlog("aesdsocket", LOG_PID, LOG_USER);
    
    // Setup signal handlers
    if (setup_signal_handlers() == -1) {
        cleanup_and_exit(1);
    }
    
    // Setup server socket
    if (setup_server_socket() == -1) {
        cleanup_and_exit(1);
    }
    
    // Fork daemon if requested
    if (daemon_mode) {
        if (create_daemon() == -1) {
            cleanup_and_exit(1);
        }
    }
    
    // Main server loop
    while (!shutdown_requested) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd == -1) {
            if (errno == EINTR) {
                continue; // Interrupted by signal, try again
            }
            syslog(LOG_ERR, "Failed to accept connection: %s", strerror(errno));
            break;
        }
        
        handle_client_connection(client_fd, &client_addr);
        
        close(client_fd);
        client_fd = -1;
    }
    
    cleanup_and_exit(0);
    return 0;
}