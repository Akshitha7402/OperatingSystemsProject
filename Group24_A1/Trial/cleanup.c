#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "utils.h"
#include <signal.h>

int main() {
    long cid = 999999;
    /*printf("Enter client-id for the cleanup process: \n");
    scanf("%ld", &cid);*/

    char choice;
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

    // Prepare a message to send
    msg.msg_type = 1; // Server expects messages with type 1
    msg.pyld.client_id = cid; // Sending the current client id to the server 
    
    while (1) {
        printf("Do you want the server to terminate? Press Y for Yes and N for No: ");
        scanf(" %c", &choice);

        if (choice == 'Y' || choice == 'y') {
             // Send a termination signal to the main server via the message queue
            msg.pyld.option = -1; // Indication to terminate

            // Send the message
            if (msgsnd(msgqid, &msg, sizeof(msg.pyld), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            // Terminate the cleanup process
            break;
        } else if (choice == 'N' || choice == 'n') {
            // Continue running the cleanup process
            continue;
        } else {
            printf("Invalid input. Please enter Y or N.\n");
        }
    }

    return 0;
}
