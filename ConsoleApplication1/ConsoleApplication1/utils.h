#pragma once

#include <memory>
#include <string>
#include <vector>
#include <locale>
#include <functional>

#include <WbemCli.h>

struct LogEntry
{
    int64_t ts{};
    int64_t priority{};
    std::string source;
    std::string message;
};

using bstr_ptr = std::unique_ptr<OLECHAR, std::function<void(BSTR)>>;

inline bstr_ptr make_bstr(const wchar_t* str) {
    return {::SysAllocString(str), [](BSTR b) { ::SysFreeString(b); }};
}
inline bstr_ptr make_bstr(const std::wstring& str) {
    return {::SysAllocString(str.c_str()), [](BSTR b) { ::SysFreeString(b); }};
}

inline int64_t convert_high_low(DWORD high, DWORD low) {
    LARGE_INTEGER li{};
    li.LowPart = low;
    li.HighPart = high;
    return static_cast<int64_t>(li.QuadPart);
}

inline int64_t convert_filetime(const FILETIME& t) {
    return convert_high_low(t.dwHighDateTime, t.dwLowDateTime);
}

inline std::string narrow_wstring(const std::wstring& str) {
    const wchar_t* from = str.c_str();
    std::size_t len = str.size();

    std::locale loc("");
    std::vector<char> buffer(len + 1);
    std::use_facet<std::ctype<wchar_t>>(loc).narrow(from, from+len, '_', &buffer[0]);
    return {&buffer[0], &buffer[len]};
}
