#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define SERVER_IP   "10.208.141.230"   // server IP
#define SERVER_PORT 8082 //port number

int main(int argc,char *argv[])
{
    int sock;
    struct sockaddr_in server;
    const char *msg = "PLACE_FINGER";
    ssize_t sent;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    /* Server configuration */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server.sin_addr) <= 0) {
        perror("Invalid server IP");
        close(sock);
        return 1;
    }

    /* Connect */
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        close(sock);
        return 1;
    }

    printf("Connected to server\n");

    /* Send message */
    if(strcmp(argv[1],"2")==0)
    {
	    sent = send(sock, "NEW", strlen("NEW"), 0);
    }
    else
    sent = send(sock, msg, strlen(msg), 0);
    if (sent < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

    printf("Sent: %s (%ld bytes)\n", msg, sent);

    /* Close */
    close(sock);
    return 0;
}

