#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

typedef enum {FIFO, SHM, MSG} TRANSMISSION_TYPE;

#define buffer_size 16384
const int data_size = 10 << 23;
TRANSMISSION_TYPE type = MSG;
typedef struct {
    long mtype;
    char mtext[buffer_size];
} msg_buff;


int main() {
    char* data_initial = malloc(data_size);
    char* data_recieved = malloc(data_size);
    srand(time(NULL));
    for(int i = 0; i < data_size; i++) {
        data_initial[i] = (char)(i % 256 - 128);
        data_recieved[i] = 0;
    }
    int key_shm = rand(), key_msg = rand();

    int fifo[2], fifo_reverse[2];
    pipe(fifo);
    pipe(fifo_reverse);

    int pid = fork();
    switch(type) {
        case FIFO:;
        if(pid == 0) {
            for(int i = 0; i < data_size; i += buffer_size)
                write(fifo[1], &data_initial[i], buffer_size);
            return 0;
        }
        else {
            for(int i = 0; i < data_size; i += buffer_size)
                read(fifo[0], &data_recieved[i], buffer_size);
            break;
        }

        case MSG:;
        int msgqid = msgget(key_msg, IPC_CREAT | 0600);
        msg_buff m_buffer;
        m_buffer.mtype = 5;
        if(pid == 0) {
            for(int i = 0; i < data_size; i += buffer_size) {
                memcpy(m_buffer.mtext, &data_initial[i], buffer_size);
                msgsnd(msgqid, &m_buffer, buffer_size, 0);
            }
            return 0;
        }
        else {
            for(int i = 0; i < data_size; i += buffer_size) {
                msgrcv(msgqid, &m_buffer, buffer_size, 5, 0);
                memcpy(&data_recieved[i], m_buffer.mtext, buffer_size);
            }
            break;
        }

        case SHM:;
        char *shm_ptr = shmat(shmget(key_shm, buffer_size, IPC_CREAT | 0600), NULL, 0);
        char space = ' ';
        if(pid == 0) {
            for(int i = 0; i < data_size; i += buffer_size) {
                memcpy(shm_ptr, &data_initial[i], buffer_size);
                write(fifo[1], &space, 1);                
                read(fifo_reverse[0], &space, 1);                
           }
            return 0;
        }        
        else {
            for(int i = 0; i < data_size; i += buffer_size) {
                read(fifo[0], &space, 1);                
                memcpy(&data_recieved[i], shm_ptr, buffer_size);
                write(fifo_reverse[1], &space, 1);                
            }
            break;
        }
    }

    for(int i = 0; i < data_size; i++)
        if(data_recieved[i] != data_initial[i]) {
            printf("Transmission error\n");
            return 0;
        }
    printf("Transmission successful\n");
    return 0;
}