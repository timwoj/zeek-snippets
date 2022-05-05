#include "EventLogManager.h"

#include <sstream>

EventLogManager::EventLogManager() : event_log(OpenEventLog(NULL, L"System"))
{
}

EventLogManager::~EventLogManager()
{
    CloseEventLog(event_log);

    printf("\n%lld entries in dll cache at shutdown", dll_cache.size());
    for ( auto& [key, library] : dll_cache)
        FreeLibrary(library);
}

std::vector<LogEntry> EventLogManager::GetLogEntries()
{
    if ( ! event_log )
        return {};

    DWORD status = ERROR_SUCCESS;
    DWORD bytes_to_read = 0x10000;
    DWORD bytes_needed;
    DWORD bytes_read = 0;

    DWORD read_flag = EVENTLOG_FORWARDS_READ;
    read_flag |= last_record_read == 0 ? EVENTLOG_SEQUENTIAL_READ : EVENTLOG_SEEK_READ;

    auto buffer = (wchar_t*)malloc(bytes_to_read);
    if ( ! buffer )
        return {};

    std::vector<LogEntry> results;

    int num_entries = 0;
    while (status == ERROR_SUCCESS) {

        if ( ! ReadEventLog(event_log, read_flag, last_record_read, buffer, bytes_to_read, &bytes_read, &bytes_needed)) {
            status = GetLastError();
            if ( status == ERROR_INSUFFICIENT_BUFFER ) {
                auto temp = (wchar_t*)realloc(buffer, bytes_needed);
                if (! temp)
                    break;

                buffer = temp;
                bytes_to_read = bytes_needed;
            }
            else if ( status != ERROR_HANDLE_EOF )
            {
                // TODO: this should probably do some error handling or something
                break;
            }
        }
        else {
            char* current = reinterpret_cast<char*>(buffer);
            char* end = current + bytes_read;

            while ( current < end )
            {
                auto record = reinterpret_cast<PEVENTLOGRECORD>(current);
                if (auto result = processRecord(current, record))
                {
                    results.push_back(result.value());
                    break;
                }
                last_record_read = record->RecordNumber;
                current += record->Length;
            }
        }

        if ( ! results.empty() )
            break;
    }

    free(buffer);
    return results;
}

#define KEY_SIZE 8192

std::optional<LogEntry> EventLogManager::processRecord(char* buffer, PEVENTLOGRECORD record)
{
    LogEntry entry{};
    entry.ts = static_cast<int64_t>(record->TimeGenerated);
    entry.priority = static_cast<int64_t>(record->EventType);

    std::wstring source = reinterpret_cast<wchar_t*>(buffer + sizeof(*record));
    if (source != L"DCOM")
        return {};
    entry.source = narrow_wstring(source);
    
    std::wstring key_name = L"SYSTEM\\CurrentControlSet\\Services\\Eventlog\\System\\" + source;

    HKEY key_handle;
    std::unique_ptr<wchar_t[]> message_file(new wchar_t[KEY_SIZE]);

    DWORD res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_name.c_str(), 0, KEY_READ, &key_handle);
    if (FAILED(res))
    {
        RegCloseKey(key_handle);
        return entry;
    }
    else
    {
        DWORD key_size = KEY_SIZE;
        DWORD key_type;
        res = RegQueryValueEx(key_handle, L"EventMessageFile", NULL, &key_type, (LPBYTE)message_file.get(), &key_size);
        RegCloseKey(key_handle);

        if ( res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND )
            return entry;
    }

    std::unique_ptr<wchar_t[]> formatted(new wchar_t[KEY_SIZE]);
    res = ExpandEnvironmentStrings(message_file.get(), formatted.get(), KEY_SIZE);
    if (res == 0)
        // TODO: this should probably output some sort of error using GetLastError()
        return {};

    // Break up the strings from the record into an array of strings so we can
    // pass them as a whole to FormatMessage().
    std::unique_ptr<wchar_t*[]> all_strings(new wchar_t*[record->NumStrings]);
    auto* curr_string = reinterpret_cast<wchar_t*>(buffer + record->StringOffset);

    for (int i = 0; i < record->NumStrings; i++)
    {
        all_strings.get()[i] = curr_string;
        if ( i == record->NumStrings - 1)
            curr_string += wcslen(curr_string) + 1;
    }

    std::wistringstream stream(formatted.get());
    std::wstring filename;
    while (std::getline(stream, filename, L';'))
    {
        // Check if we already have this module in the cache and load it if we don't.
        HMODULE dll;
        auto it = dll_cache.find(filename);
        if (it != dll_cache.end())
            dll = it->second;
        else
        {
            dll = LoadLibraryEx(filename.data(), NULL, LOAD_LIBRARY_AS_DATAFILE);
            if (dll)
                dll_cache.insert({filename, dll});
            else
            {
                // TODO: error message?
                return entry;
            }
        }

        wchar_t* actual_message = nullptr;
        res = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER, dll, record->EventID, NULL, 
            (LPWSTR)&actual_message, 0, (va_list*)all_strings.get());
        if (res == 0 && GetLastError() != ERROR_MR_MID_NOT_FOUND)
        {
            // TODO: there's probably better error handling that could be done here.
            printf("Format message failed: %lu\n", GetLastError());
            return entry;
        }

        if ( actual_message )
        {
            entry.message = narrow_wstring(std::wstring(actual_message));
            LocalFree(actual_message);
        }
        else
        {
        // TODO: DCOM (and others) are missing this EventMessageFile registry entry and so therefore
        // TODO: won't get their strings formatted correctly. We can either fix this, or we can
        // TODO: output something more generic in those cases (like a collection of the strings we did get)
        }
    }
    
    return entry;
}