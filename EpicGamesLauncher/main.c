#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <stdio.h>

#define CP_UNIXCP     65010 /* Wine extension */

static BOOL append_char(wchar_t *buffer, size_t capacity, size_t *length, wchar_t ch) {
    if (*length >= capacity - 1)
        return FALSE;

    buffer[(*length)++] = ch;
    buffer[*length] = L'\0';

    return TRUE;
}

WCHAR* convert_to_win32(const WCHAR *unixW) {
    typedef WCHAR* (CDECL *wine_get_dos_file_name_PFN)(const char* path);

    HMODULE k32 = LoadLibraryW(L"kernel32");
    wine_get_dos_file_name_PFN pwine_get_dos_file_name =
        (wine_get_dos_file_name_PFN)GetProcAddress(k32, "wine_get_dos_file_name");

    if (!pwine_get_dos_file_name) return NULL;

    int r = WideCharToMultiByte(CP_UNIXCP, 0, unixW, -1, NULL, 0, NULL, NULL);

    if (!r) return NULL;

    char *unixA = malloc(r);

    if (!unixA) return NULL;

    r = WideCharToMultiByte(CP_UNIXCP, 0, unixW, -1, unixA, r, NULL, NULL);

    if (!r) return NULL;

    WCHAR *ret = pwine_get_dos_file_name(unixA);
    free(unixA);

    return ret;
}


static void check_epic_api_path(int argc, WCHAR **argv)
{
    const WCHAR prefix[] = L"-eglv2apipath=";
    const size_t prefix_len = wcslen(prefix);

    for (int i = 1; i < argc; i++) {
        if (_wcsnicmp(argv[i], prefix, prefix_len) != 0)
            continue;

        const WCHAR *value = argv[i] + prefix_len;
        size_t len = wcslen(value);

        BOOL quoted = (
            len >= 2 &&
            value[0] == L'"' &&
            value[len - 1] == L'"'
        );

        size_t offset = quoted ? 1 : 0;
        size_t path_len = quoted ? len - 2 : len;

        if (!path_len) {
            wprintf(L"EGLV2: Empty API path.\n");
            return;
        }

        WCHAR *path = calloc(path_len + 1, sizeof(WCHAR));

        if (!path)
            return;

        memcpy(
            path,
            value + offset,
            path_len * sizeof(WCHAR)
        );

        BOOL unix_path = path[0] == L'/';

        WCHAR *converted = unix_path
            ? convert_to_win32(path)
            : _wcsdup(path);

        free(path);

        if (!converted) {
            wprintf(L"EGLV2: Failed to convert API path.\n");
            return;
        }

        wprintf(L"EGLV2 API path: [%ls]\n", converted);

        DWORD attr = GetFileAttributesW(converted);

        if (attr == INVALID_FILE_ATTRIBUTES) {
            wprintf(
                L"EGLV2: File not accessible. Error: %lu\n",
                GetLastError()
            );
        }
        else if (attr & FILE_ATTRIBUTE_DIRECTORY) {
            wprintf(L"EGLV2: Path points to a directory!\n");
        }
        else {
            wprintf(L"EGLV2: API file exists and is accessible!\n");
        }

        if (unix_path)
            HeapFree(GetProcessHeap(), 0, converted);
        else
            free(converted);

        return;
    }

    wprintf(L"EGLV2: Argument not found!\n");
}

static BOOL add_argument_capacity(size_t *capacity, const wchar_t *arg) {
    size_t len = wcslen(arg);
    size_t max = (size_t)-1;

    if (*capacity > max - 3)
        return FALSE;

    if (len > (max - *capacity - 3) / 2)
        return FALSE;

    *capacity += len * 2 + 3;

    return TRUE;
}

static BOOL append_quoted_argument(wchar_t *buffer, size_t capacity, size_t *length, const wchar_t *arg) {
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
        if (skip_logs && _wcsicmp(argv[i], L"--logs") == 0)
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
        if (skip_logs && _wcsicmp(argv[i], L"--logs") == 0)
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

int wmain(int argc, WCHAR **argv) {
    if (argc < 2) return -1;

    check_epic_api_path(argc, argv);

    const WCHAR* exe = argv[1];
    WCHAR* temp = NULL;

    int forward_argc = argc + 2;

    WCHAR **forward_argv = calloc(
        forward_argc,
        sizeof(WCHAR *)
    );

    if (!forward_argv)
        return 1;

    for (int i = 0; i < argc; i++)
        forward_argv[i] = argv[i];

    forward_argv[argc] = L"-useEpic";
    forward_argv[argc + 1] = L"-fromRGL";

    WCHAR *args = build_arguments(
        NULL,
        forward_argc,
        forward_argv,
        2,
        FALSE
    );

    free(forward_argv);

    SHELLEXECUTEINFOW info = {0};

    if (!args)
        return 1;

    /* handle a unix path */
    if (exe[0] == L'/' || (exe[0] == L'"' && exe[1] == L'/')) {
        temp = convert_to_win32(exe);
        if (!temp) return -1;
        exe = temp;
    }

    wprintf(L"Wrapper: Executing: %ls %ls\n", exe, args);

    wprintf(L"Wrapper: Received arguments:\n");

    for (int i = 0; i < argc; i++) {
        wprintf(L"argv[%d] = [%ls]\n", i, argv[i]);
    }

    /* guess and set working directory */
    {
        WCHAR *exe_cpy = calloc(wcslen(exe)+1, sizeof(*exe));
        wcscpy(exe_cpy, exe);
        WCHAR *last = wcsrchr(exe_cpy, '\\');
        if (last)
        {
            *last = L'\0';
            printf("Wrapper: Working dir: %ls\n", exe_cpy);
            SetCurrentDirectoryW(exe_cpy);
        }
        free(exe_cpy);
    }

    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE;
    info.nShow = SW_SHOWNORMAL;
    info.lpFile = exe;
    info.lpParameters = args;

    if (!ShellExecuteExW(&info)) {
        DWORD error = GetLastError();

        wprintf(
            L"Wrapper: Failed to execute process. Error: %lu\n",
            error
        );

        free(args);

        if (temp)
            HeapFree(GetProcessHeap(), 0, temp);

        return 1;
    }

    free(args);

    if (!info.hProcess) {
        if (temp)
            HeapFree(GetProcessHeap(), 0, temp);

        return 1;
    }

    wprintf(L"Wrapper: Waiting for process termination...\n");
    fflush(stdout);

    DWORD exit_code = 1;

    DWORD wait_result = WaitForSingleObject(
        info.hProcess,
        INFINITE
    );

    if (wait_result == WAIT_OBJECT_0) {
        if (!GetExitCodeProcess(info.hProcess, &exit_code)) {
            exit_code = 1;
        }
    }
    else {
        wprintf(
            L"Wrapper: WaitForSingleObject failed. Error: %lu\n",
            GetLastError()
        );
    }

    CloseHandle(info.hProcess);

    wprintf(
        L"Wrapper: Process terminated! Exit code: %lu\n",
        exit_code
    );

    if (temp)
        HeapFree(GetProcessHeap(), 0, temp);

    return (int)exit_code;
}
