#include "shim_core.h"

#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>

#define SHIM_EXIT_FAILURE 255u

static const wchar_t *skip_first_command_line_token(const wchar_t *command_line) {
    const wchar_t *p = command_line;
    int in_quotes = 0;

    if (p == NULL) {
        return L"";
    }

    while (*p == L' ' || *p == L'\t') {
        ++p;
    }

    while (*p != L'\0') {
        if (*p == L'\"') {
            in_quotes = !in_quotes;
            ++p;
            continue;
        }

        if (!in_quotes && (*p == L' ' || *p == L'\t')) {
            break;
        }

        ++p;
    }

    while (*p == L' ' || *p == L'\t') {
        ++p;
    }

    return p;
}

static wchar_t *build_child_command_line(const wchar_t *target_path, const wchar_t *tail) {
    const size_t target_len = wcslen(target_path);
    const size_t tail_len = wcslen(tail);
    const int has_tail = tail_len > 0;

    // Quotes around target + optional space + tail + NUL.
    const size_t total = 1u + target_len + 1u + (has_tail ? 1u : 0u) + tail_len + 1u;

    if (total > (size_t)(DWORD)-1) {
        return NULL;
    }

    wchar_t *buffer = (wchar_t *)calloc(total, sizeof(wchar_t));
    if (buffer == NULL) {
        return NULL;
    }

    wchar_t *out = buffer;
    *out++ = L'\"';
    memcpy(out, target_path, target_len * sizeof(wchar_t));
    out += target_len;
    *out++ = L'\"';

    if (has_tail) {
        *out++ = L' ';
        memcpy(out, tail, tail_len * sizeof(wchar_t));
        out += tail_len;
    }

    *out = L'\0';
    return buffer;
}

static int configure_job_kill_on_close(HANDLE job) {
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
    ZeroMemory(&info, sizeof(info));
    info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    return SetInformationJobObject(
        job,
        JobObjectExtendedLimitInformation,
        &info,
        sizeof(info)) != 0;
}

int shim_run(const wchar_t *target_path) {
    const wchar_t *command_line = GetCommandLineW();
    const wchar_t *tail = skip_first_command_line_token(command_line);
    wchar_t *child_command_line = build_child_command_line(target_path, tail);

    if (child_command_line == NULL) {
        return (int)SHIM_EXIT_FAILURE;
    }

    HANDLE job = CreateJobObjectW(NULL, NULL);
    if (job == NULL) {
        free(child_command_line);
        return (int)SHIM_EXIT_FAILURE;
    }

    if (!configure_job_kill_on_close(job)) {
        CloseHandle(job);
        free(child_command_line);
        return (int)SHIM_EXIT_FAILURE;
    }

    STARTUPINFOW startup_info;
    PROCESS_INFORMATION process_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    ZeroMemory(&process_info, sizeof(process_info));

    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    BOOL created = CreateProcessW(
        target_path,
        child_command_line,
        NULL,
        NULL,
        TRUE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        &startup_info,
        &process_info);

    free(child_command_line);

    if (!created) {
        CloseHandle(job);
        return (int)SHIM_EXIT_FAILURE;
    }

    if (!AssignProcessToJobObject(job, process_info.hProcess)) {
        TerminateProcess(process_info.hProcess, SHIM_EXIT_FAILURE);
        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
        CloseHandle(job);
        return (int)SHIM_EXIT_FAILURE;
    }

    if (ResumeThread(process_info.hThread) == (DWORD)-1) {
        TerminateProcess(process_info.hProcess, SHIM_EXIT_FAILURE);
        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
        CloseHandle(job);
        return (int)SHIM_EXIT_FAILURE;
    }

    DWORD wait_result = WaitForSingleObject(process_info.hProcess, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        TerminateProcess(process_info.hProcess, SHIM_EXIT_FAILURE);
        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
        CloseHandle(job);
        return (int)SHIM_EXIT_FAILURE;
    }

    DWORD exit_code = SHIM_EXIT_FAILURE;
    if (!GetExitCodeProcess(process_info.hProcess, &exit_code)) {
        exit_code = SHIM_EXIT_FAILURE;
    }

    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
    CloseHandle(job);

    return (int)exit_code;
}
