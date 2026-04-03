#ifndef MONITOR_H
#define MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define MAX_PATH        512
#define MAX_FILES       256
#define TXT_EXTENSION   ".txt"
#define BACKUP_DIR      "backup"
#define LOG_DIR         "logs"
#define LOG_FILE        "logs/report.txt"
#define ALERTS_DIR      "alerts"

#define MSG_FILE        'F'
#define MSG_DONE        'D'

#define ERR(msg)  (fprintf(stderr, "[ERROR] %s: %s\n", (msg), strerror(errno)))

typedef struct {
    char   name[MAX_PATH];
    char   full_path[MAX_PATH];
    off_t  size_bytes;
    char   timestamp[64];
} FileInfo;

typedef struct {
    FileInfo *files;
    int       count;
} ScanResult;

ScanResult *scan_directory(const char *dir_path);
void        free_scan_result(ScanResult *result);
void        print_scan_result(const ScanResult *result);

int  backup_file(const FileInfo *fi, const char *backup_dir);
int  backup_all(const ScanResult *result, const char *backup_dir);
void log_activity(const FileInfo *fi, const char *status);

#endif
