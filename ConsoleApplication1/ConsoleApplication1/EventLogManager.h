#pragma once

#include <vector>
#include <map>
#include <optional>
#include <string>
#include <wtypes.h>

#include "utils.h"

class EventLogManager
{
public:
    EventLogManager();
    ~EventLogManager();

    std::vector<LogEntry> GetLogEntries();

private:
    
    std::optional<LogEntry> processRecord(char* buffer, PEVENTLOGRECORD record);

    DWORD last_record_read = 0;
    HANDLE event_log = nullptr;
    std::map<std::wstring, HMODULE> dll_cache;
};

