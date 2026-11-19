#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <string.h>

#define CMD_SIZE 32768
#define PATH_SIZE 32768

static BOOL append_char(wchar_t *buffer, size_t capacity, size_t *length, wchar_t ch) {
    if (*length >= capacity - 1)
        return FALSE;

    buffer[(*length)++] = ch;
    buffer[*length] = L'\0';

    return TRUE;
}

static BOOL add_argument_capacity( size_t *capacity, const wchar_t *arg) {
    size_t len = wcslen(arg);
    size_t max = (size_t)-1;

    if (*capacity > max - 3)
        return FALSE;

    if (len > (max - *capacity - 3) / 2)
        return FALSE;

    *capacity += len * 2 + 3;

    return TRUE;
}

static BOOL append_quoted_argument( wchar_t *buffer, size_t capacity, size_t *length, const wchar_t *arg) {
    if (*length && !append_char(buffer, capacity, length, L' '))
        return FALSE;

    if (!append_char(buffer, capacity, length, L'"'))
        return FALSE;

    const wchar_t *p = arg;

    while (*p) {
        size_t slashes = 0;

        while (*p == L'\\') {
            slashes++;
            p++;
        }

        if (*p == L'"') {
            for (size_t i = 0; i < slashes * 2 + 1; i++) {
                if (!append_char(buffer, capacity, length, L'\\'))
                    return FALSE;
            }

            if (!append_char(buffer, capacity, length, L'"'))
                return FALSE;

            p++;
        }
        else if (*p == L'\0') {
            for (size_t i = 0; i < slashes * 2; i++) {
                if (!append_char(buffer, capacity, length, L'\\'))
                    return FALSE;
            }

            break;
        }
        else {
            for (size_t i = 0; i < slashes; i++) {
                if (!append_char(buffer, capacity, length, L'\\'))
                    return FALSE;
            }

            if (!append_char(buffer, capacity, length, *p++))
                return FALSE;
        }
    }

    return append_char(buffer, capacity, length, L'"');
}

static wchar_t *build_arguments( const wchar_t *first, int argc, wchar_t *argv[], int first_index, BOOL skip_logs) {
    size_t capacity = 1;

    if (first && !add_argument_capacity(&capacity, first))
        return NULL;

    for (int i = first_index; i < argc; i++) {
        if (skip_logs && _wcsicmp(argv[i], L"-logs") == 0)
            continue;

        if (!add_argument_capacity(&capacity, argv[i]))
            return NULL;
    }

    if (capacity > (size_t)-1 / sizeof(wchar_t))
        return NULL;

    wchar_t *buffer = calloc(capacity, sizeof(wchar_t));

    if (!buffer)
        return NULL;

    size_t length = 0;

    if (first && !append_quoted_argument(
        buffer, capacity, &length, first
    )) {
        free(buffer);
        return NULL;
    }

    for (int i = first_index; i < argc; i++) {
        if (skip_logs && _wcsicmp(argv[i], L"-logs") == 0)
            continue;

        if (!append_quoted_argument(
            buffer, capacity, &length, argv[i]
        )) {
            free(buffer);
            return NULL;
        }
    }

    return buffer;
}

static BOOL has_argument(int argc, wchar_t *argv[], const wchar_t *target)
{
    for (int i = 1; i < argc; i++) {
        if (_wcsicmp(argv[i], target) == 0)
            return TRUE;
    }

    return FALSE;
}

void print_welcome(int argc, wchar_t *argv[])
{
    BOOL epic = FALSE;
    BOOL steam = FALSE;
    BOOL no_battleye = FALSE;

    for (int i = 1; i < argc; i++) {
        if (_wcsicmp(argv[i], L"-epicAppId=gta5") == 0 || _wcsicmp(argv[i], L"-epicAppId=rdr2") == 0 || _wcsicmp(argv[i], L"-eglv2") == 0)
            epic = TRUE;

        if (_wcsicmp(argv[i], L"-steamAppId=gta5") == 0 || _wcsicmp(argv[i], L"-steamAppId=rdr2") == 0)
            steam = TRUE;

        if (_wcsicmp(argv[i], L"-nobattleye") == 0)
            no_battleye = TRUE;
    }

    const wchar_t *store = epic ? L"Epic Games" : steam ? L"Steam" : L"Unknown";

    wprintf(L"================= Play GTA V Linux =================\n\n");
    wprintf(L"Store:           \t %ls\n", store);
    wprintf(L"Battle Eye:      \t %ls\n", no_battleye ? L"Disabled" : L"Enabled");
    wprintf(L"Github:          \t https://github.com/treviasxk\n");
    wprintf(L"Repository:      \t https://github.com/treviasxk/PlayGTAVLinux\n");
    wprintf(L"Version:         \t 2.0.0.0\n");
    wprintf(L"Created by:      \t Trevias Xk\n");
}


int wmain(int argc, wchar_t *argv[])
{
    if (argc < 1)
        return 1;

    wchar_t dir[PATH_SIZE];
    wchar_t fake_epic[PATH_SIZE];
    wchar_t run[PATH_SIZE];
    wchar_t executable_name[PATH_SIZE];
    wchar_t *params = NULL;

    DWORD n = GetModuleFileNameW(NULL, dir, PATH_SIZE);

    if (!n || n >= PATH_SIZE)
        return 1;

    wchar_t *last = wcsrchr(dir, L'\\');

    if (!last)
        return 1;


    BOOL show_logs = has_argument(argc, argv, L"-logs");

    if (!show_logs) {
        HWND console = GetConsoleWindow();

        if (console)
            ShowWindow(console, SW_HIDE);
    }

    wcscpy(executable_name, last + 1);
    *last = L'\0';

    if (_wcsicmp(executable_name, L"PlayGTAVLinux.exe") == 0) {
        wcscpy(run, L"PlayGTAV.exe");
    }
    else if (_wcsicmp(executable_name, L"PlayRDR2Linux.exe") == 0) {
        wcscpy(run, L"PlayRDR2.exe");
    }
    else {
        MessageBoxW(
            NULL,
            L"Executable not recognized.",
            L"Game Proxy",
            MB_ICONERROR
        );
        return 1;
    }

    swprintf(fake_epic, PATH_SIZE, L"%ls\\EpicGamesLauncher.exe", dir);
    wchar_t run_path[PATH_SIZE];
    swprintf(run_path, PATH_SIZE,L"%ls\\%ls", dir, run);

    if (GetFileAttributesW(fake_epic) == INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesW(run_path) == INVALID_FILE_ATTRIBUTES) {

        MessageBoxW(
            NULL,
            L"EpicGamesLauncher.exe or executable not found!",
            L"Game Proxy",
            MB_ICONERROR
        );
        return 1;
    }

    params = build_arguments(
        run_path,
        argc,
        argv,
        1,
        TRUE
    );

    if (!params) {
        MessageBoxW(
            NULL,
            L"Failed to build command line arguments.",
            L"Game Proxy",
            MB_ICONERROR
        );

        return 1;
    }

    if (show_logs) {
        print_welcome(argc, argv);

        wprintf(L"\n==================== GAME PROXY ====================\n\n");
        wprintf(L"Original: %ls\n", executable_name);
        wprintf(L"Epic Integration: %ls\n", fake_epic);
        wprintf(L"Executable: %ls\n", run_path);
        wprintf(L"Arguments: %ls\n", params);
        wprintf(L"\n====================================================\n\n");

        fflush(stdout);
    }

    SHELLEXECUTEINFOW info = {0};

    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE;
    info.lpVerb = L"open";
    info.lpFile = fake_epic;
    info.lpParameters = params;
    info.lpDirectory = dir;
    info.nShow = show_logs ? SW_SHOWNORMAL : SW_HIDE;

    if (!ShellExecuteExW(&info)) {
        DWORD error = GetLastError();

        free(params);

        wchar_t message[256];

        swprintf(
            message,
            256,
            L"Failed to start Epic Integration.\nError: %lu",
            error
        );

        MessageBoxW(NULL, message, L"Game Proxy", MB_ICONERROR);

        return 1;
    }

    free(params);

    if (!info.hProcess)
        return 1;

    WaitForSingleObject(info.hProcess, INFINITE);

    DWORD exit_code = 1;

    GetExitCodeProcess(info.hProcess, &exit_code);

    CloseHandle(info.hProcess);

    if (show_logs) {
        wprintf(
            L"\nGame process terminated. Exit code: %lu\n",
            exit_code
        );

        fflush(stdout);
    }

    return (int)exit_code;
}