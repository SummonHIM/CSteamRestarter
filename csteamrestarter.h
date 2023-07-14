#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_LENGTH 256
#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#include <tlhelp32.h>
#define CLEAR_SCREEN "cls"
#pragma comment(lib, "kernel32.lib")
#elif __linux__
#include <dirent.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define CLEAR_SCREEN "clear"
#endif

int WINDOW_COL, WINDOW_ROW;

int getScreenSize() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    WINDOW_COL = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    WINDOW_ROW = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#elif __linux__
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    WINDOW_COL = ws.ws_col;
    WINDOW_ROW = ws.ws_row;
#else
#error "Unsupported operating system."
#endif
}

void printfCenter(char *s, const char *fs, const char *es) { printf("%s%*s%*s%s", fs, (WINDOW_COL / 2) + strlen(s) / 2, s, (WINDOW_COL / 2) - strlen(s) / 2, "", es); }

void printLine() {
    for (int i = 0; i < WINDOW_COL; i++) {
        printf("-");
    }
    printf("\n");
}

void clear_screen() { system(CLEAR_SCREEN); }

char mgetch(void) {
    char ch;
#ifdef _WIN32
    ch = getch();
#elif __linux__
    struct termios oldattr, newattr;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
#else
#error "Unsupported operating system."
#endif
    return ch;
}

char *getConfig(const char *filename, const char *key) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }

        char *k = strtok(line, "=");
        char *v = strtok(NULL, "=");

#ifdef _WIN32
        if (v != NULL) {
            v[strcspn(v, "\r\n")] = '\0';
        }
#endif

        if (k != NULL && v != NULL && strcmp(k, key) == 0) {
            fclose(file);
            return strdup(v);
        }
    }

    fclose(file);
    return NULL;
}

char *translation(const char *str) { return getConfig("locales\\zh_CN.lang", str); }

int isProcessRunning(const char *processName) {
#ifdef _WIN32
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        return 0;
    }

    do {
        if (strcmp(entry.szExeFile, processName) == 0) {
            CloseHandle(snapshot);
            return 1;
        }
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return 0;
#elif __linux__
    DIR *dir = opendir("/proc");
    if (!dir) {
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char path[256];
            snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);

            FILE *file = fopen(path, "r");
            if (file) {
                char cmdline[256];
                if (fgets(cmdline, sizeof(cmdline), file) != NULL) {
                    cmdline[strlen(cmdline) - 1] = '\0';

                    if (strcmp(cmdline, processName) == 0) {
                        closedir(dir);
                        fclose(file);
                        return 1;
                    }
                }
                fclose(file);
            }
        }
    }

    closedir(dir);
    return 0;
#else
#error "Unsupported operating system."
#endif
}

void terminateProcess(const char *processName) {
#ifdef _WIN32
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD pid = 0;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("\t- %s\n", translation("Error: Failed to create process snapshot."));
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnapshot, &pe32)) {
        printf("\t- %s\n", translation("Error: Failed to get process information."));
        CloseHandle(hSnapshot);
        return;
    }

    while (Process32Next(hSnapshot, &pe32)) {
        if (_stricmp(pe32.szExeFile, processName) == 0) {
            pid = pe32.th32ProcessID;
            break;
        }
    }

    if (pid != 0) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess != NULL) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
            printf("\t- %s\n", translation("Process terminated successfully."));
        } else {
            printf("\t- %s\n", translation("Error: Failed to open process."));
        }
    } else {
        printf("\t- %s\n", translation("Error: Process not found."));
    }

    CloseHandle(hSnapshot);
#else
#error "Unsupported operating system."
#endif
}

char *removeQuotes(char *str) {
    int len = strlen(str);
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
    return str;
}