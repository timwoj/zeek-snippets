#include "Processes.h"

#include <cstdio>
#include <iostream>

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

#include "utils.h"

void Processes::GetFromEnum()
{
    DWORD* processes = nullptr;
    DWORD bytes_needed = 0;
    DWORD current_limit = 0;
    do {
        if (current_limit == 0)
            current_limit = 512;
        else
            current_limit *= 2;

        printf("current_limit: %lu", current_limit);
        processes = new DWORD[current_limit];
        if ( ! EnumProcesses(processes, current_limit*sizeof(DWORD), &bytes_needed))
        {
            delete[] processes;
            processes = nullptr;
        }

        printf("bytes_needed: %lu %llu\n", bytes_needed, (bytes_needed / sizeof(DWORD)));
    } while (bytes_needed / sizeof(DWORD) > current_limit);

    if (!processes) {
        return;
    }

    DWORD process_count = bytes_needed / sizeof(DWORD);
    printf("total processes found: %lu\n\n", process_count);
    for (DWORD i = 0; i < process_count; i++) {
        HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);

        CHAR proc_name[MAX_PATH]{};
        HRESULT res = GetModuleFileNameExA(proc, NULL, proc_name, MAX_PATH);
        if (FAILED(res))
        {
            CloseHandle(proc);
            continue;
        }

        PROCESS_MEMORY_COUNTERS memory;
        if ( ! GetProcessMemoryInfo(proc, &memory, sizeof(memory)) )
        {
            CloseHandle(proc);
            continue;
        }

        DWORD prio = GetPriorityClass(proc);
        if (prio == 0)
        {
            CloseHandle(proc);
            continue;
        }

        FILETIME creation_time, exit_time, kernel_time, user_time;
        if (! GetProcessTimes(proc, &creation_time, &exit_time, &kernel_time, &user_time))
        {
            CloseHandle(proc);
            continue;
        }

        std::cout << "name: " << proc_name << std::endl;
        std::cout << "pid: " << static_cast<int64_t>(processes[i]) << std::endl;
        std::cout << "priority: " << static_cast<int64_t>(prio) << std::endl; // TODO convert this something sorta matching nice() levels
        std::cout << "utime: " << convert_filetime(user_time) << std::endl;
        std::cout << "stime: " << convert_filetime(kernel_time) << std::endl;
        std::cout << std::endl;

        CloseHandle(proc);
    }
}

void Processes::GetFromSnapshot()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if ( snapshot == INVALID_HANDLE_VALUE )
        return;

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(PROCESSENTRY32);

    if ( ! Process32First(snapshot, &entry))
    {
        CloseHandle(snapshot);
        return;
    }

    uint32_t process_count = 0;
    do
    {
        process_count++;
        HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);

        CHAR proc_name[MAX_PATH];
        HRESULT res = GetModuleFileNameExA(proc, NULL, proc_name, MAX_PATH);
        if (FAILED(res))
        {
            CloseHandle(proc);
            continue;
        }

        PROCESS_MEMORY_COUNTERS memory;
        if ( ! GetProcessMemoryInfo(proc, &memory, sizeof(memory)) )
        {
            CloseHandle(proc);
            continue;
        }

        DWORD prio = GetPriorityClass(proc);
        if (prio == 0)
        {
            CloseHandle(proc);
            continue;
        }

        FILETIME creation_time, exit_time, kernel_time, user_time;
        if (! GetProcessTimes(proc, &creation_time, &exit_time, &kernel_time, &user_time))
        {
            CloseHandle(proc);
            continue;
        }

        std::cout << "name: " << proc_name << std::endl;
        std::cout << "pid: " << static_cast<int64_t>(entry.th32ProcessID) << std::endl;
        std::cout << "ppid: " << static_cast<int64_t>(entry.th32ParentProcessID) << std::endl;
        std::cout << "priority: " << static_cast<int64_t>(prio) << std::endl; // TODO convert this something sorta matching nice() levels
        std::cout << "utime: " << convert_filetime(user_time) << std::endl;
        std::cout << "stime: " << convert_filetime(kernel_time) << std::endl;
        std::cout << std::endl;

        CloseHandle(proc);

    } while(Process32Next(snapshot, &entry));

    printf("\nfound %u processes", process_count);

    CloseHandle(snapshot);
}
