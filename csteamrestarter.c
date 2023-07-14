#include "csteamrestarter.h"

char *VERSION = "1.0";
char *PROCESSNAME = "steam.exe";
char *PROCESSPATH;
char *PROCESSARG;

int loadConfig() {
    char configPath[MAX_LINE_LENGTH];
    const char *appData = getenv("APPDATA");
    const char *configTmp = "\\SteamRestarter\\Settings.cfg";
    strcpy(configPath, appData);
    strcat(configPath, configTmp);

    char *processPath = getConfig(configPath, "PROCESSPATH");
    char *processArg = getConfig(configPath, "PROCESSARG");

    if (processPath != NULL) {
        PROCESSPATH = strdup(processPath);
        free(processPath);
    }
    if (processArg != NULL) {
        PROCESSARG = strdup(processArg);
        free(processArg);
    }

    return 0;
}

int writeConfig() {
    char *configPath = getenv("APPDATA");
    char *configTmp = "\\SteamRestarter\\Settings.cfg";
    char *filePath = malloc(strlen(configPath) + strlen(configTmp) + 1);
    FILE *fp;

    strcpy(filePath, configPath);
    strcat(filePath, configTmp);

    fp = fopen(filePath, "w+");
    if (fp == NULL) {
        printf("%s\n", translation("Failed to create file!"));
        free(filePath);
        return 1;
    }

    if (PROCESSPATH != NULL)
        fprintf(fp, "PROCESSPATH=%s\n", removeQuotes(PROCESSPATH));
    if (PROCESSARG != NULL)
        fprintf(fp, "PROCESSARG=%s\n", PROCESSARG);

    fclose(fp);
    free(filePath);
    return 0;
}

int startSteam() {
    printf("%s", translation("Starting steam..."));

    char *fullCommand = malloc(strlen(PROCESSPATH) + strlen(" ") + strlen(PROCESSARG) + 1);
    strcpy(fullCommand, PROCESSPATH);
    strcat(fullCommand, " ");
    strcat(fullCommand, PROCESSARG);

    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    BOOL status = CreateProcess(NULL, fullCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (status) {
        printf("\t- %s\n", translation("Process started successfully."));
        free(fullCommand);
        return 0;
    } else {
        printf("\t- %s\n", translation("Failed to start the process."));
        free(fullCommand);
        return 1;
    }
    free(fullCommand);
    return 1;
}

int stopSteam() {
    printf("%s", translation("Terminating steam..."));
    if (isProcessRunning(PROCESSNAME)) {
        terminateProcess(PROCESSNAME);
    } else {
        printf("\t- %s\n", translation("Failed to force terminate steam. Is your steam client still running?"));
    }
}

int restartSteam() {
    stopSteam();
    startSteam();
}

int flushDNS() { system("ipconfig /flushdns"); }

int options() {
    char optionsChoice, tmpProcessPath[256], tmpProcessArg[256];
    while (1) {
        clear_screen();
        printfCenter("Options", "", "\n");
        printLine();
        printf(" [1] %s\n", translation("Steam Path"));
        printf("   > %s\n\n", PROCESSPATH);
        printf(" [2] %s\n", translation("Start parameters"));
        printf("   > %s\n\n", PROCESSARG);
        printf(" [3] %s\n", translation("Save & exit"));
        printf(" [4] %s\n\n", translation("Unsave & exit"));
        optionsChoice = mgetch();
        switch (optionsChoice) {
        case '1':
            printf("%s", translation("New path:"));
            fgets(tmpProcessPath, sizeof(tmpProcessPath), stdin);
            PROCESSPATH = tmpProcessPath;
            break;
        case '2':
            printf("%s", translation("New parameters:"));
            fgets(tmpProcessArg, sizeof(tmpProcessArg), stdin);
            PROCESSARG = tmpProcessArg;
            break;
        case '3':
            writeConfig();
            return 0;
            break;
        case '4':
            return 0;
            break;
        }
    }
}

int about() {
    getScreenSize();
    printfCenter(translation("Steam Restarter"), "", "\n");
    printLine();
    printfCenter(translation("[Version]"), "", "");
    printfCenter(VERSION, "", "\n\n");
    printfCenter(translation("[GitHub]"), "", "");
    printfCenter("https://github.com/SummonHIM/CSteamRestarter", "", "\n\n");
    printfCenter(translation("Copyright 2023 SummonHIM."), "", "");
    printLine();
    return 0;
}

int main() {
    char mainChoice, regPathChoice;
    setlocale(LC_ALL, "");
    while (1) {
        loadConfig();
        DWORD fileAttributes = GetFileAttributes(removeQuotes(PROCESSPATH));
        if (fileAttributes = INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char readProcessPath[256];
            printf("%s\n", translation("Steam client path not found! Please enter the path manually:"));
            fgets(readProcessPath, sizeof(readProcessPath), stdin);
            PROCESSPATH = readProcessPath;
            writeConfig();
            mgetch();
        } else {
            clear_screen();
            getScreenSize();
            printfCenter(translation("Steam Restarter"), "", "\n");
            printLine();
            printfCenter(translation("[1] Force terminate Steam"), "\n", "\n\n");
            printfCenter(translation("[2] Force restart Steam"), "", "\n\n");
            printfCenter(translation("[3] Purges DNS cache"), "", "\n\n");
            printfCenter(translation("[4] Options"), "", "\n\n");
            printfCenter(translation("[5] About"), "", "\n\n");
            printfCenter(translation("[6] Exit"), "", "\n\n");
            mainChoice = mgetch();
            switch (mainChoice) {
            case '1':
                stopSteam();
                printf("%s\n", translation("Press any key to return..."));
                mgetch();
                break;
            case '2':
                restartSteam();
                printf("%s\n", translation("Press any key to return..."));
                mgetch();
                break;
            case '3':
                flushDNS();
                printf("%s\n", translation("Press any key to return..."));
                mgetch();
                break;
            case '4':
                options();
                break;
            case '5':
                clear_screen();
                about();
                printf("%s\n", translation("Press any key to return..."));
                mgetch();
                break;
            case '6':
                clear_screen();
                exit(0);
                break;
            default:
                printf("%s %c.\n", translation("Unknown option"), mainChoice);
                printf("%s\n", translation("Press any key to return..."));
                mgetch();
                break;
            }
        }
    }
}