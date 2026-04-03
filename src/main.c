#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "monitor.h"

// flag so sigint can stop the loop
int running = 1;

void handle_sigint(int sig) {
    printf("\ncaught signal %d, shutting down...\n", sig);
    running = 0;
}

int main(int argc, char *argv[]) {
    const char *dir = (argc > 1) ? argv[1] : "project";

    printf("Checking if system monitoring is active...\n");
    printf("Looking at folder: %s\n", dir);

    // register sigint handler
    signal(SIGINT, handle_sigint);

    ScanResult *result = scan_directory(dir);
    if (!result) {
        printf("Error: Could not scan directory.\n");
        return 1;
    }

    print_scan_result(result);

    // create pipe for ipc between parent and child
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        printf("Error: pipe failed.\n");
        free_scan_result(result);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        printf("Error: fork failed.\n");
        free_scan_result(result);
        return 1;
    }

    if (pid == 0) {
        // child process - reads paths from pipe and does backup + log
        close(pipefd[1]);

        printf("Child process started (pid: %d)\n", getpid());

        char path[MAX_PATH];
        while (read(pipefd[0], path, MAX_PATH) > 0) {
            for (int i = 0; i < result->count; i++) {
                if (strcmp(result->files[i].full_path, path) == 0) {
                    backup_file(&result->files[i], BACKUP_DIR);
                    log_activity(&result->files[i], "OK");
                    break;
                }
            }
        }

        close(pipefd[0]);
        printf("Child process done.\n");
        free_scan_result(result);
        exit(0);

    } else {
        // parent process - sends file paths to child through pipe
        close(pipefd[0]);

        printf("Parent process (pid: %d) sending files to child...\n", getpid());

        for (int i = 0; i < result->count && running; i++) {
            write(pipefd[1], result->files[i].full_path, MAX_PATH);
        }

        close(pipefd[1]);
        wait(NULL);

        printf("All done. Check backup/ and logs/report.txt\n");
    }

    free_scan_result(result);
    return 0;
}
