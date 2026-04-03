

#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define COPY_BUF_SIZE 4096

static int ensure_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 0;
        fprintf(stderr, "%s exists but is not a directory\n", path);
        return -1;
    }
    if (mkdir(path, 0755) == -1) return -1;
    return 0;
}

static int copy_file(const char *src, const char *dst) {
    int fd_in = open(src, O_RDONLY);
    if (fd_in == -1) return -1;

    int fd_out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) { close(fd_in); return -1; }

    char buf[COPY_BUF_SIZE];
    ssize_t n_read;
    while ((n_read = read(fd_in, buf, COPY_BUF_SIZE)) > 0) {
        if (write(fd_out, buf, (size_t)n_read) != n_read) {
            close(fd_in); close(fd_out); return -1;
        }
    }

    close(fd_in);
    close(fd_out);
    return (n_read == -1) ? -1 : 0;
}

void log_activity(const FileInfo *fi, const char *status) {
    if (ensure_dir(LOG_DIR) == -1) return;
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (!log_fp) return;

    time_t now;
    time(&now);
    char now_str[64];
    strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(log_fp, "[%s] FILE=%-30s SIZE=%lld B MODIFIED=%s STATUS=%s\n",
            now_str, fi->name, (long long)fi->size_bytes, fi->timestamp, status);

    fclose(log_fp);
}

int backup_file(const FileInfo *fi, const char *backup_dir) {
    if (ensure_dir(backup_dir) == -1) return -1;

    char dst_path[MAX_PATH * 2];
    snprintf(dst_path, MAX_PATH * 2, "%s/%s", backup_dir, fi->name);

    printf("Copying %s → %s\n", fi->name, dst_path);

    if (copy_file(fi->full_path, dst_path) == -1) {
        log_activity(fi, "FAILED");
        fprintf(stderr, "Failed to backup: %s\n", fi->name);
        return -1;
    }

    log_activity(fi, "OK");
    printf("Backup successful: %s\n", fi->name);
    return 0;
}

int backup_all(const ScanResult *result, const char *backup_dir) {
    if (!result || result->count == 0) {
        printf("No files to back up.\n");
        return 0;
    }

    int success = 0;
    for (int i = 0; i < result->count; i++) {
        if (backup_file(&result->files[i], backup_dir) == 0)
            success++;
    }
    printf("%d / %d file(s) backed up successfully.\n", success, result->count);
    return success;
}
