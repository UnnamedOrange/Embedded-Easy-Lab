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
    int listen_sock; // Socket to listen on.
    int conn_sock;   // Socket to connect to.
} global_variables_t;
static global_variables_t g;

/**
 * @brief Open the listen socket and bind it to the local address.
 */
void open_listen_sock(void)
{
    struct addrinfo* local_addr; // Local address. Should be released using
                                 // freeaddrinfo(...).

    // Get local address.
    {
        struct addrinfo hints = {};      // Hint for getaddrinfo(...).
        hints.ai_family = AF_INET;       // IPv4.
        hints.ai_socktype = SOCK_STREAM; // TCP. (TODO: verify)
        hints.ai_flags = AI_PASSIVE;     // Use local address. (TODO: verify)
        int result;
        if ((result = getaddrinfo(NULL, "5000", &hints, &local_addr)) != 0)
        {
            fprintf(stderr, "Fail to getaddrinfo(...): %s\n",
                    gai_strerror(result));
            exit(1);
        }
    }

    // Open socket.
    g.listen_sock = socket(local_addr->ai_family, local_addr->ai_socktype,
                           local_addr->ai_protocol);
    if (g.listen_sock == -1)
    {
        perror("socket(...)"); // It would output to stderr that
                               // "socket(...): <message>".
        exit(1);
    }

    // Set socket options.
    int optval = 1;
    // SO_REUSEADDR: Allow reuse of local addresses.
    if (setsockopt(g.listen_sock, SOL_SOCKET, SO_REUSEADDR, &optval,
                   sizeof(optval)) == -1)
    {
        perror("setsockopt(...)");
        exit(1);
    }

    // Bind socket.
    if (bind(g.listen_sock, local_addr->ai_addr, local_addr->ai_addrlen) == -1)
    {
        freeaddrinfo(local_addr);
        perror("bind(...)");
        exit(1);
    }

    // Free local address.
    freeaddrinfo(local_addr);

    // Convert the socket to listen mode.
    if (listen(g.listen_sock, 1) == -1) // Argument 2 is the maximum length of
                                        // the queue of pending connections.
    {
        perror("listen(...)");
        exit(1);
    }

    // Set the listen socket to non-blocking.
    fcntl(g.listen_sock, F_SETFL, O_NONBLOCK);
}

/**
 * @brief Close the listen socket.
 */
void close_listen_sock(void)
{
    if (g.listen_sock == 0 || g.listen_sock == -1)
        return;
    close(g.listen_sock);
    memset(&g.listen_sock, 0, sizeof(g.listen_sock));
}

/**
 * @brief Close the connection socket.
 */
void close_conn_sock(void)
{
    if (g.conn_sock == 0 || g.conn_sock == -1)
        return;
    close(g.conn_sock);
    memset(&g.conn_sock, 0, sizeof(g.conn_sock));
}

/**
 * @brief Chat with the client in a dead loop.
 * 1. Set the listen socket to non-blocking mode.
 * 2. Wait for the client to connect. Then use accept(...) to connect.
 * 3. Chat with the client using g.conn_sock.
 */
void chat(void)
{
    do
    {
        fd_set fds_read; // File descriptors set. Use FD_xxx macro to manipulate
                         // it.
        FD_ZERO(&fds_read);

        FD_SET(0, &fds_read);                 // Add stdin to the set.
        if (g.conn_sock == 0)                 // If there is no connection yet.
            FD_SET(g.listen_sock, &fds_read); // Add listen socket to the set.
        else
            FD_SET(g.conn_sock, &fds_read); // Add connection socket to the set.

        struct timeval tv = {};
        tv.tv_usec = 100000; // Wait for at most 100ms.

        int result;
        int max_fd = g.listen_sock > g.conn_sock ? g.listen_sock : g.conn_sock;
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
            // Send to client.
            if (!g.conn_sock)
            {
                printf("No connection yet.\n");
                continue;
            }
            if (send(g.conn_sock, buf, len, 0) == -1)
            {
                perror("send(...)");
                break;
            }
        }
        if (!g.conn_sock &&
            FD_ISSET(g.listen_sock, &fds_read)) // Ready to accept a connection.
        {
            g.conn_sock = accept(g.listen_sock, NULL, NULL);
            if (g.conn_sock == -1)
            {
                perror("accept(...)");
                break;
            }
            close_listen_sock();
            puts("Client connected.");
        }
        else if (g.conn_sock &&
                 FD_ISSET(g.conn_sock, &fds_read)) // Receive message.
        {
            char buf[1024];
            int len = recv(g.conn_sock, buf, sizeof(buf), 0);
            if (len == -1)
            {
                perror("recv(...)");
                break;
            }
            else if (len == 0)
            {
                printf("Client disconnected.\n");
                close_conn_sock();
                open_listen_sock();
                continue;
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
    open_listen_sock();
    chat(); // Main loop.
    close_listen_sock();
    close_conn_sock();
}
