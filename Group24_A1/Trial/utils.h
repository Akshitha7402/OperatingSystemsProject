#ifndef UTILS_H
#define UTILS_H

#define MSG_KEY 12345
#define MAX_MSG_SIZE 256
#define MAX_CHILDREN 10
#define PIPE_TIMEOUT 1

struct payload
{
    char msg_text[MAX_MSG_SIZE];
    int option;
    long client_id;
};

struct msg_buffer {
    long msg_type;
    struct payload pyld;
};

#endif