#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "utils.h"
#include <signal.h>

// Signal handler to exit gracefully on Ctrl+C (SIGINT)
void handle_sigint(int signo) {
    printf("\nClient: Received Ctrl+C. Cleaning up and exiting...\n");
    exit(0);
}

int main(void) {
    long cid;
    printf("Enter client-id: \n");
    scanf("%ld", &cid);

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // Set up a signal handler for Ctrl+C (SIGINT)
    signal(SIGINT, handle_sigint);

    int msgqid;
    key_t key;
    struct msg_buffer msg;

    // Connect to the message queue created by the server
    key = ftok("/tmp", MSG_KEY);
    if (key == -1) {
        perror("ftok");
        exit(-1);
    }

    msgqid = msgget(key, 0666);
    if (msgqid == -1) {
        perror("msgget");
        exit(-2);
    }

    printf("Client: Connected to Message Queue with ID %d\n", msgqid);

    // Prepare a message to send
    msg.msg_type = 1; // Server expects messages with type 1
    msg.pyld.client_id = cid; // Sending the current client id to the server 

    while (1) {
        printf("Enter 1 to contact the Ping Server\n");
        printf("Enter 2 to contact the File Search Server\n");
        printf("Enter 3 to contact the File Word Count Server\n");
        printf("Enter 4 if this Client wishes to exit\n");

        printf("Pick one: ");
        fgets(msg.pyld.msg_text, sizeof(msg.pyld.msg_text), stdin);

        int len = strlen(msg.pyld.msg_text);
        if (len > 0 && msg.pyld.msg_text[len - 1] == '\n') {
            // Remove the newline character if present
            msg.pyld.msg_text[len - 1] = '\0';
        }

        if (strcmp(msg.pyld.msg_text, "4") == 0) {
            printf("Exiting gracefully...\n");
            // Perform any necessary cleanup or finalization tasks here
            // Exit the program gracefully
            exit(EXIT_SUCCESS);
        }

        if (strcmp(msg.pyld.msg_text, "1") == 0) {
            msg.pyld.option = 1;
            strcpy(msg.pyld.msg_text, "Client: Hi, Server!");

            if (msgsnd(msgqid, &msg, sizeof(msg.pyld), 0) == -1) {
                perror("Error in msgsnd");
                exit(-3);
            }
        }

        if (strcmp(msg.pyld.msg_text, "2") == 0) {
            // Asking the user to give file name to search for a file
            // Send the command 2 and the filename

            msg.pyld.option = 2;
            printf("Give File name to Search for:\n");
            fgets(msg.pyld.msg_text, sizeof(msg.pyld.msg_text), stdin);

            int len = strlen(msg.pyld.msg_text);
            if (len > 0 && msg.pyld.msg_text[len - 1] == '\n') {
                // Remove the newline character if present
                msg.pyld.msg_text[len - 1] = '\0';
            }

            if (msgsnd(msgqid, &msg, sizeof(msg.pyld), 0) == -1) {
                perror("Error in msgsnd");
                exit(-4);
            }
        }

        if (strcmp(msg.pyld.msg_text, "3") == 0) {
            // Asking the user to give file name to find word count
            // Send the command 3 and the filename

            msg.pyld.option = 3;
            printf("Give File name to find word count for:\n");
            fgets(msg.pyld.msg_text, sizeof(msg.pyld.msg_text), stdin);

            int len = strlen(msg.pyld.msg_text);
            if (len > 0 && msg.pyld.msg_text[len - 1] == '\n') {
                // Remove the newline character if present
                msg.pyld.msg_text[len - 1] = '\0';
            }

            if (msgsnd(msgqid, &msg, sizeof(msg.pyld), 0) == -1) {
                perror("Error in msgsnd");
                exit(-5);
            }
        }

        struct msg_buffer response;
        // Receive the response from the server with the correct message type
        if (msgrcv(msgqid, &response, sizeof(response.pyld), cid, 0) == -1) {
            perror("Error in msgrcv");
            exit(-6);
        } else {
            printf("Client: Received Response: %s\n", response.pyld.msg_text);
        }
    }
    return 0;
}













