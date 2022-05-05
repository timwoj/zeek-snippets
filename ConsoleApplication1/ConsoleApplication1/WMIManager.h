#pragma once

#include <string>
#include <vector>
#include <WbemCli.h>

#include "utils.h"

struct AccountInfo {
    std::string name;
    std::string full_name;
    std::string sid;
    std::string home_directory;
    bool is_admin = false;
    bool is_system_acct = false;
};

class WMIManager {
public:
    WMIManager();
    ~WMIManager();

    [[nodiscard]] std::string GetOSVersion() const;
    [[nodiscard]] std::vector<AccountInfo> GetUserData() const;
    [[nodiscard]] std::vector<LogEntry> GetLogs() const;

private:

    [[nodiscard]] IEnumWbemClassObject* GetQueryEnumerator(const std::wstring& query) const;

    IWbemLocator* locator = nullptr;
    IWbemServices* cimv2_service = nullptr;
    IWbemServices* default_service = nullptr;

    bstr_ptr cimv2_root = nullptr;
    bstr_ptr default_root = nullptr;
    bstr_ptr wql = nullptr;
    bstr_ptr stdregprov = nullptr;
};
