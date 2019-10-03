#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

typedef struct {
  int txd[2]; 
  int rxd[2]; 
} dpipe_t;

int forker(dpipe_t* p, int* in, int* out) {
    pipe(p->txd);
    pipe(p->rxd);
    int pid = fork();
    if(pid) {
        close(p->txd[0]);
        close(p->rxd[1]);
        *in = p->rxd[0];
        *out = p->txd[1];
    }
    else {
        close(p->rxd[0]);
        close(p->txd[1]);
        *in = p->txd[0];
        *out = p->rxd[1];
    }
    return pid;
}

int main() {
    dpipe_t p;
    int in, out;

    int pid = forker(&p, &in, &out);
    fcntl(in, F_SETFL, O_NONBLOCK);
    fcntl(out, F_SETFL, O_NONBLOCK);

    char name[7];
    if(pid) strcpy(name, "Parent");
    else strcpy(name, "Child");

    int ppid = getppid();
    srand(getpid());
    int ncycles = rand() % 10 + 1;
    for(int i = 0; i < ncycles; i++) {
        int num = rand();
        printf("%s sent number %d\n", name, num);
        dprintf(out, "%d", num);
        
        char res[15];
        int len = 0;
        while((len = read(in,res,14)) <= 0) {
            if(pid == 0 && getppid() == ppid) continue;
            if(pid != 0 && waitpid(pid, NULL, WNOHANG) == 0) continue;
            goto end;
        }
        res[len] = 0;
        printf("%s recieved number %s\n", name, res);
        sleep(1);
    }
end:
    printf("%s terminated\n", name);
    fflush(stdout);
}