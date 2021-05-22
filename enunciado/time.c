#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    struct timeval current_time;
    struct timeval end_process_time;

    gettimeofday(&current_time, NULL);
    int pid = fork();
    if (pid == 0) {
        char *command_args[argc];

        for (int i=0; i<argc-1; i++){
            command_args[i] = argv[i+1];
        }    

        command_args[argc-1] = NULL;
        execvp(command_args[0], command_args);  // run
    }
    else if (pid < 0) {
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    } else {
        wait(NULL);
        gettimeofday(&end_process_time, NULL);
        int sec;
        int msec;
        sec = end_process_time.tv_sec - current_time.tv_sec;
        msec= end_process_time.tv_usec - current_time.tv_usec;
        printf(" Seconds: %d Microseconds : %d \n ", sec,msec);
    }
    return 0;
}