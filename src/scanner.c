//scanner.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include "monitor.h"

// check if a file name ends with ".txt"
static int has_txt_extension(const char *filename)
{
    size_t flen = strlen(filename);
    size_t elen = strlen(TXT_EXTENSION);

    if (flen <= elen) return 0;

    return strcmp(filename + flen - elen, TXT_EXTENSION) == 0;
}

// fill in FileInfo structure from file data
static void populate_file_info(FileInfo *fi, const char *dir_path, const char *filename)
{
    struct stat st;
    struct tm  *tm_info;

    strncpy(fi->name, filename, MAX_PATH - 1);
    fi->name[MAX_PATH - 1] = '\0';

    snprintf(fi->full_path, MAX_PATH, "%s/%s", dir_path, filename);

    if (stat(fi->full_path, &st) == -1) {
        ERR("stat");
        fi->size_bytes = -1;
        strncpy(fi->timestamp, "unknown", sizeof(fi->timestamp) - 1);
        return;
    }

    fi->size_bytes = st.st_size;

    tm_info = localtime(&st.st_mtime);
    strftime(fi->timestamp, sizeof(fi->timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
}

// scan the directory and return a list of .txt files
ScanResult *scan_directory(const char *dir_path)
{
    DIR *dp = opendir(dir_path);
    if (!dp) {
        ERR("opendir");
        return NULL;
    }

    int capacity = 16;
    ScanResult *result = malloc(sizeof(ScanResult));
    if (!result) {
        closedir(dp);
        return NULL;
    }

    result->count = 0;
    result->files = malloc(capacity * sizeof(FileInfo));
    if (!result->files) {
        free(result);
        closedir(dp);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (entry->d_type != DT_REG && entry->d_type != DT_UNKNOWN) continue;
        if (!has_txt_extension(entry->d_name)) continue;

        if (result->count >= capacity) {
            capacity *= 2;
            FileInfo *tmp = realloc(result->files, capacity * sizeof(FileInfo));
            if (!tmp) break;
            result->files = tmp;
        }

        populate_file_info(&result->files[result->count], dir_path, entry->d_name);
        result->count++;
    }

    closedir(dp);
    return result;
}

// free memory used by scan results
void free_scan_result(ScanResult *result)
{
    if (!result) return;
    free(result->files);
    free(result);
}

// print scan results in human-readable sentences
void print_scan_result(const ScanResult *result)
{
    if (!result || result->count == 0) {
        printf("No .txt files were found in the directory.\n");
        return;
    }

    printf("Found %d .txt file(s) in the directory:\n\n", result->count);

    for (int i = 0; i < result->count; i++) {
        printf("File: %s\n", result->files[i].name);
        printf("  Size: %lld bytes\n", (long long)result->files[i].size_bytes);
        printf("  Last modified at: %s\n\n", result->files[i].timestamp);
    }

    printf("End of scan.\n");
}
