# 🖥️ BCS323-linux-file-monitor

> Linux-based multi-process file monitoring and backup system built in C — featuring IPC via pipes, fork(), shell automation, and a Makefile build system. Built for BCS 323 System Level Programming at Canadian University Dubai.

---

## 📌 Overview

This project was built for **BCS 323 – System Level Programming** at Canadian University Dubai. It implements a fully functional Linux system tool that monitors a directory, detects `.txt` files, logs their metadata, and creates backups — using a multi-process architecture with inter-process communication (IPC) via pipes.

The system is automated with a Bash script and built using a Makefile, following professional development practices including Git version control and GDB debugging.

---

## 🗂️ Project Structure

```
BCS323-linux-file-monitor/
│
├── src/
│   ├── main.c          # Entry point — orchestrates scan, IPC, and backup
│   ├── scanner.c       # Module 1 — directory traversal and .txt detection
│   ├── backup.c        # Module 2 — file copying and activity logging
│   └── monitor.h       # Shared header — structs, macros, function prototypes
│
├── scripts/
│   └── run_system.sh   # Bash automation — compiles and runs the full system
│
├── backup/             # Auto-created — stores backup copies of detected files
├── logs/
│   └── report.txt      # Auto-created — activity log with timestamps and status
├── alerts/             # Reserved for future alerting functionality
│
├── Makefile            # Build system
├── .gitignore
└── README.md
```

---

## ⚙️ How It Works

### System Architecture

```
[run_system.sh]
      │
      ▼
[monitor] ← main.c
      │
      ├── scan_directory()       ← scanner.c
      │     └── detects .txt files, reads name/size/timestamp via stat()
      │
      ├── fork()
      │     ├── Parent: writes file paths → pipe → Child
      │     └── Child:  reads pipe → backup_file() + log_activity()
      │
      └── wait() — parent waits for child to finish
```

### IPC via Pipe
- `pipe()` creates a one-way channel between parent and child
- **Parent process** scans the directory and writes full file paths into the pipe
- **Child process** reads paths from the pipe, performs backup, and logs each file's status to `logs/report.txt`
- `SIGINT` (Ctrl+C) is handled gracefully — a signal handler sets a flag to stop the loop cleanly

---

## 🧩 Modules

### `scanner.c` — Directory Scanner
- Accepts a directory path as a command-line argument
- Uses `opendir()` / `readdir()` to traverse the directory
- Identifies `.txt` files using a custom extension check
- Retrieves file metadata (size, timestamp) via `stat()`
- Stores results in a dynamically allocated array of `FileInfo` structs using `malloc` / `realloc`

### `backup.c` — Backup & Logger
- Creates the `backup/` directory if it doesn't exist using `mkdir()`
- Copies files block-by-block using low-level `open()`, `read()`, `write()` with a 4KB buffer
- Logs each file's backup status (`OK` / `FAILED`) with a timestamp to `logs/report.txt`

### `monitor.h` — Shared Header
Key definitions:
```c
#define MAX_PATH      512
#define MAX_FILES     256
#define TXT_EXTENSION ".txt"
#define BACKUP_DIR    "backup"
#define LOG_FILE      "logs/report.txt"
#define ERR(msg)      (fprintf(stderr, "[ERROR] %s: %s\n", (msg), strerror(errno)))

typedef struct {
    char  name[MAX_PATH];
    char  full_path[MAX_PATH];
    off_t size_bytes;
    char  timestamp[64];
} FileInfo;

typedef struct {
    FileInfo *files;
    int       count;
} ScanResult;
```

---

## 🚀 Getting Started

### Prerequisites
- Linux (tested on Kali Linux)
- `gcc`, `make`, `bash`

### Build & Run

**Option 1 — Automated (recommended):**
```bash
chmod +x scripts/run_system.sh
./scripts/run_system.sh
```
The script compiles the project with `make`, runs the executable, and appends output to `logs/report.txt`.

**Option 2 — Manual:**
```bash
make
./monitor <directory_to_scan>
```

**Example:**
```bash
./monitor /home/user/documents
```

**Clean build:**
```bash
make clean
```

---

## 🔧 Makefile

```makefile
CC     = gcc
FLAGS  = -Wall -g
TARGET = monitor
SRC    = src/main.c src/scanner.c src/backup.c

all:
    $(CC) $(FLAGS) $(SRC) -o $(TARGET)
    @echo "build done"

clean:
    rm -f $(TARGET)
    @echo "cleaned"
```

---

## 📋 Linux CLI Commands Used

| Command | Purpose |
|---|---|
| `find` | Locate `.txt` files within the directory |
| `grep` | Filter log entries by keyword |
| `diff` | Verify backup file integrity vs. originals |
| `chmod` | Set execute permissions on shell scripts |
| `ps` | View running processes |
| `kill` | Terminate processes by PID |
| `cat` | Read and display log file contents |

---

## ✅ Test Cases

| Test | Condition | Expected Result |
|---|---|---|
| Normal scan | Directory with 3 `.txt` files | All 3 detected, backed up, logged |
| Empty directory | No files present | "No .txt files found" message |
| Mixed files | `.txt`, `.pdf`, `.c` files present | Only `.txt` files processed |
| No `.txt` files | Directory with other file types | Scanner skips all files |
| SIGINT handling | Ctrl+C during execution | Graceful shutdown, no zombie processes |
| Backup integrity | `diff` original vs. backup | No differences — identical copies |

---

## 🐛 Debugging

GDB was used to step through the multi-process logic:
```bash
gdb ./monitor
break main
run
next
```
Valgrind was used to verify no memory leaks after `free_scan_result()`.

---

## ⚠️ Challenges & Solutions

| Challenge | Solution |
|---|---|
| Parent finishing before child → zombie processes | Used `wait()` in parent to block until child exits |
| Program hanging due to unclosed pipe ends | Closed unused pipe ends (`pipefd[1]` in child, `pipefd[0]` in parent) |
| Dynamic array overflow for large directories | Used `realloc()` to double capacity as needed |
| "Permission Denied" on backup directory creation | Used `chmod` to fix access permissions |

---

## 👥 Team & Contributions

| Student | ID | Contribution |
|---|---|---|
| **Yasmin Issa** | 20230003798 | Scanner & Backup C modules, IPC pipe logic, fork() architecture |
| **Hassan Mujtaba** | 20220002085 | Shell script (`run_system.sh`), Makefile, build automation |
| **Leanne Jessica Rodrigo** | 20210001983 | Git management, GDB debugging, testing, challenges analysis |

---

## 🏫 Course Information

**BCS 323 – System Level Programming**  
School of Engineering, Applied Sciences, and Technology  
Canadian University Dubai — Spring 2025–26  
Instructors: Dr. Mehak Khurana, Mr. Adel, Mr. Hussein

**CLOs Covered:** CLO-1 · CLO-2 · CLO-3 · CLO-4 · CLO-5 · CLO-6

---

## ⚠️ Notes

- Run on **Linux only** — system calls (`fork`, `pipe`, `stat`, `opendir`) are POSIX-specific
- The `monitor` binary is gitignored — compile from source using `make`
- `logs/` and `backup/` directories are auto-created at runtime
