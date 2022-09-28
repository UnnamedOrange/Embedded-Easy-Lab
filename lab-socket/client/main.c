#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <error.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct
{
    const char* ip_input; // IP address.
    int sock;             // The only socket in client.
} global_variables_t;
static global_variables_t g;

/**
 * @brief Parse the args and save the IP address to g.ip_input.
 */
void parse_arg(int argn, char* argv[])
{
    if (argn != 2)
    {
        printf("Usage: %s <IP address>\n", argv[0]);
        exit(0);
    }
    g.ip_input = argv[1];
}

/**
 * @brief Close the socket.
 */
void close_sock(void)
{
    if (g.sock == 0 || g.sock == -1)
        return;
    close(g.sock);
    memset(&g.sock, 0, sizeof(g.sock));
}

/**
 * @brief Open the socket and connect to the server.
 */
void open_sock_and_connect(void)
{
    struct addrinfo* local_addr; // Local address. Should be released using
                                 // freeaddrinfo(...).

    // Get local address.
    {
        struct addrinfo hints; // Hint for getaddrinfo(...).
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;       // IPv4.
        hints.ai_socktype = SOCK_STREAM; // TCP. (TODO: verify)
        // hints.ai_flags = AI_PASSIVE;     // Use local address. (TODO: verify)
        int result;
        if ((result = getaddrinfo(g.ip_input, "5000", &hints, &local_addr)) !=
            0)
        {
            fprintf(stderr, "Fail to getaddrinfo(...): %s\n",
                    gai_strerror(result));
            exit(1);
        }
    }

    // Create socket.
    g.sock = socket(local_addr->ai_family, local_addr->ai_socktype,
                    local_addr->ai_protocol);
    if (g.sock == -1)
    {
        perror("socket(...)"); // It would output to stderr that
                               // "socket(...): <message>".
        exit(1);
    }

    // Connect to the server.
    if (connect(g.sock, local_addr->ai_addr, local_addr->ai_addrlen) == -1)
    {
        perror("connect");
        exit(1);
    }

    // Free local address.
    freeaddrinfo(local_addr);

    // Set the socket to non-blocking.
    fcntl(g.sock, F_SETFL, O_NONBLOCK);
}

void chat(void)
{
    do
    {
        fd_set fds_read; // File descriptors set. Use FD_xxx macro to manipulate
                         // it.
        FD_ZERO(&fds_read);

        FD_SET(0, &fds_read);      // Add stdin to the set.
        FD_SET(g.sock, &fds_read); // Add  socket to the set.

        struct timeval tv;
        memset(&tv, 0, sizeof(tv));
        tv.tv_usec = 100000; // Wait for at most 100ms.

        int result;
        int max_fd = g.sock;
        result =
            select(max_fd + 1, &fds_read, NULL, NULL, // NULL are non-used sets.
                   &tv);
        // Don't rely on the value of tv now!

        if (result == -1)
        {
            perror("select(...)");
            break;
        }

        if (FD_ISSET(0, &fds_read)) // stdin is ready.
        {
            char buf[1024];
            int len = read(0, buf, sizeof(buf));
            if (len == -1)
            {
                perror("read(...)");
                break;
            }
            else if (len == 0)
            {
                printf("EOF on stdin\n");
                break;
            }
            // Send to server.
            if (send(g.sock, buf, len, 0) == -1)
            {
                perror("send(...)");
                break;
            }
        }
        if (FD_ISSET(g.sock, &fds_read)) // Receive message.
        {
            char buf[1024];
            int len = recv(g.sock, buf, sizeof(buf), 0);
            if (len == -1)
            {
                perror("recv(...)");
                break;
            }
            else if (len == 0)
            {
                printf("Server disconnected.\n");
                break;
            }
            // Send to stdout.
            if (write(1, buf, len) == -1)
            {
                perror("write(...)");
                break;
            }
        }
    } while (true);
}

int main(int argc, char* argv[])
{
    parse_arg(argc, argv);
    open_sock_and_connect();
    chat(); // Main loop.
    close_sock();
}
