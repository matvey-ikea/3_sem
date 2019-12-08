#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

char* data = NULL;
int curr_bit = 0;
int total_size = 0;
int cpid, ppid;

int buff_size = 8;

char* input = "lebedev5.c";
char* output = "lebedev5_copy.c";
    
char mask() {
    return 1 << (curr_bit % 8);
}
void expand(int d) {
    static int data_size = 0;
    if(data_size == 0) {
        data = malloc(1);
        data_size = 1;
    }
    while(d < data_size)
        data = realloc(data, data_size *= 2);
}
void set_zero(int n) {
    expand(curr_bit / 8 + 1);
    data[curr_bit / 8] &= ~mask();
    curr_bit++;
    kill(ppid, SIGUSR1);
}
void set_one(int n) {
    expand(curr_bit / 8 + 1);
    data[curr_bit / 8] |= mask();
    curr_bit++;
    kill(ppid, SIGUSR1);
}
void finish(int n) {
    int fd = open(output, O_CREAT | O_WRONLY, 0600);
    write(fd, data, curr_bit / 8);
    exit(0);
}
void send_bit(int n) {
    if(curr_bit / 8 == total_size)
        return;
    if(data[curr_bit / 8] & mask()) {
        curr_bit++;
        kill(cpid, SIGUSR2);
    }
    else {
        curr_bit++;
        kill(cpid, SIGUSR1);
    }
}
int main() {
    signal(SIGUSR1, set_zero);
    signal(SIGUSR2, set_one);
    signal(SIGINT, finish);

    ppid = getpid();
    cpid = fork();
    
    if(cpid) {
        signal(SIGUSR1, send_bit);
        signal(SIGINT, SIG_DFL);
        int fd = open(input, O_RDONLY);

        expand(buff_size);
        for(int d; d = read(fd, &data[total_size], buff_size); total_size += d)
            expand(total_size + buff_size);
        write(STDOUT_FILENO, data, total_size);
        send_bit(0);
        while(curr_bit / 8 != total_size)
            usleep(1);
        kill(cpid, SIGINT);
    }
    else
        while(1) pause();
}
