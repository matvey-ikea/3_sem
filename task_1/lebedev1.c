#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char in[2097152];
char* argv[200000];

int main() {
    while(1) {
        int pid = fork();
        if(pid == 0) {
            fgets(in, 2097152, stdin);
            argv[0] = strtok(in, " \n");
            for (int i = 1; argv[i - 1] != NULL; i++)
                argv[i] = strtok(NULL, " \n");
            execvp(argv[0], argv);
            printf("Exec failed!\n");
            break;
        }
        else {
            int status;
            waitpid(pid, &status, 0);
            printf("Exit status: %d\n", WEXITSTATUS(status));
        }
    }
}