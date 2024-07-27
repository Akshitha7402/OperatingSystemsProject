#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h> // Include signal handling
#include "utils.h"   // Include the utils header
#include <string.h>

int msgqid;
int num_children = 0;

// Signal handler to remove the message queue and exit gracefully
void cleanup_and_exit(void) {

    // Parent waits for child terminations
    while (num_children > 0) 
    {
        int child_status;
        pid_t terminated_child_pid;
        terminated_child_pid = wait(&child_status);
        if(terminated_child_pid == -1) 
        {
            printf("\nNo active children to wait for.\n");
            break;
        }
        if (terminated_child_pid > 0) 
        {
            printf("Child %d terminated\n", terminated_child_pid);
            num_children--;
        }
    }

    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(3);
    }
    exit(0);
}

int main(void) 
{
    struct msg_buffer msg;
    key_t key;

    // Create a message queue (or get an existing one)
    key = ftok("/tmp", MSG_KEY);
    if (key == -1) {
        perror("ftok");
        exit(-1);
    }

    msgqid = msgget(key, 0666 | IPC_CREAT);
    if (msgqid == -1) {
        perror("msgget");
        exit(-2);
    }

    printf("Server: Message Queue Created with ID %d\n", msgqid);
    // cleanup_and_exit();

    // Set up a signal handler to remove the message queue on Ctrl+C (SIGINT)
    //signal(SIGINT, cleanup_and_exit);

    // Continuously listen for messages
    while (1) 
    {
        while(num_children<MAX_CHILDREN)
        {
                // Receive a message
                if (msgrcv(msgqid, &msg, sizeof(msg.pyld), 1, 0) == -1) {
                    perror("msgrcv");
                    exit(-3);
                } 
                else 
                {

                    if(msg.pyld.option == -1) {
                        cleanup_and_exit();
                    }

                    // Create a pipe for communication with the child process
                    int pipe_fd[2];
                    if (pipe(pipe_fd) == -1) {
                        perror("pipe");
                        exit(-4);
                    }

                    // Create a child process
                    pid_t pid = fork();
                    if (pid == -1) {
                        perror("fork");
                        exit(-5);
                    }
                    if (pid == 0) {   // Child  process

                        num_children++;//counting number of running children

                        close(pipe_fd[1]); // Close the write end of the pipe


                        struct msg_buffer input_msg;//defining input message recieved from parent process about the client process to be executed
                        if(read(pipe_fd[0],&input_msg,sizeof(input_msg))==-1)
                        {
                            perror("read");
                        }
                        close(pipe_fd[0]);
                        printf("Server's Child received msg from client id : %ld\n",input_msg.pyld.client_id);
                        printf("Server's Child: Received Command: %s\n", input_msg.pyld.msg_text);



                        struct msg_buffer response_msg;// defining response to be sent to client
                        response_msg.msg_type = input_msg.pyld.client_id;
                        response_msg.pyld.client_id=input_msg.pyld.client_id;
                        response_msg.pyld.option=input_msg.pyld.option;


                        char response_text[MAX_MSG_SIZE];
                        ssize_t n;


                        int pipe_child[2];
                        if(pipe(pipe_child)== -1)
                        {
                            perror("pipe");
                            exit(-6);
                        }

                        
                        if (input_msg.pyld.option == 1){
                            strcpy(response_msg.pyld.msg_text, "Server: Hello Client");
                        }
                        else if(input_msg.pyld.option == 2){

                                    // Create a grandchild process

                                    pid_t pid_1= fork();
                                    if (pid_1== -1) {
                                        perror("fork");
                                        exit(-7);
                                    }
                                    int status_grandchild;

                                    if(pid_1 == 0)
                                    {
                                        close(pipe_child[0]);
                                        dup2(pipe_child[1], STDOUT_FILENO);
                                        close(pipe_child[1]);


                                        char command[256];
                                        snprintf(command, sizeof(command),"find . -name %s",input_msg.pyld.msg_text);
                                        if (execlp("sh","sh","-c" , command, NULL) == -1) {
                                            perror("execlp");
                                            exit(-8);
                                        }

                                    }
                                    else
                                    {   
                                        waitpid(pid_1,&status_grandchild,0); // wait for grandchild to complete

                                        close(pipe_child[1]);
                                        dup2(pipe_child[0], STDIN_FILENO);
                                        close(pipe_child[0]);


                                        while ((n = read(STDIN_FILENO, response_text, sizeof(response_text))) > 0) 
                                        {
                                            response_text[n] = '\0'; // Null-terminate the received data
                                        }
                                        if(response_text[0]=='\0')
                                        {
                                            strncpy(response_msg.pyld.msg_text, "The File Doesn't exist",sizeof(response_msg.pyld.msg_text));

                                        }
                                        else
                                        {
                                            strncpy(response_msg.pyld.msg_text, response_text, sizeof(response_msg.pyld.msg_text));
                                        }
                                    }
                        }
                        else if(input_msg.pyld.option==3){
                                    

                                    // Create a grandchild process
                                    int pid_2= fork();
                                    if (pid_2== -1) {
                                        perror("fork");
                                        exit(-9);
                                    }
                                    int status_grandchild;


                                    if(pid_2 == 0)
                                    {

                                        close(pipe_child[0]);
                                        dup2(pipe_child[1], STDOUT_FILENO);
                                        close(pipe_child[1]);

                                        char command[256];
                                        snprintf(command, sizeof(command),"wc -cw %s",input_msg.pyld.msg_text);
                                        if (execlp("sh", "sh", "-c", command, NULL) == -1) {
                                            perror("execlp");
                                            exit(-10);
                                        }
                                    }
                                    else
                                    {
                                        waitpid(pid_2,&status_grandchild,0); // wait for grandchild to complete

                                        close(pipe_child[1]);
                                        dup2(pipe_child[0], STDIN_FILENO);
                                        close(pipe_child[0]);

                                        while ((n = read(STDIN_FILENO, response_text, sizeof(response_text))) > 0) 
                                        {
                                            response_text[n] = '\0'; // Null-terminate the received data
                                        }
                                        strncpy(response_msg.pyld.msg_text, response_text, sizeof(response_msg.pyld.msg_text));
                                    }
                        }
                        // Send the response to the client through the message queue
                        if (msgsnd(msgqid, &response_msg, sizeof(response_msg.pyld), 0) == -1) {
                            perror("msgsnd");
                            exit(-11);
                        }
                    }
                    else
                    {  
                        close(pipe_fd[0]);
                        if(write(pipe_fd[1],&msg,sizeof(msg))==-1)
                            {
                                perror("write");
                            }
                        close(pipe_fd[1]);                               
                    }
            }
        }
        // Parent waits for child terminations
        while (num_children > 0) 
        {
            int child_status;
            pid_t terminated_child_pid;
            terminated_child_pid = wait(&child_status);
            if (terminated_child_pid > 0) 
            {
                printf("Child %d terminated\n", terminated_child_pid);
                num_children--;
            }
        }
    }
    return 0;
}
