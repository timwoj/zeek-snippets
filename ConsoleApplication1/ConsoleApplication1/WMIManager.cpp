#include "WMIManager.h"

#include <iostream>

WMIManager::WMIManager() {
    HRESULT res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if ( FAILED(res) )
    {
        printf("Failed to initialize COM\n");
        return;
    }

    res = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                               EOAC_NONE, 0);
    if ( FAILED(res) )
    {
        printf("Failed to initialize security level\n");
        return;
    }

    res = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&locator);
    if ( FAILED(res) )
    {
        printf("Failed to create COM locator\n");
        return;
    }

    cimv2_root = make_bstr(L"root\\CIMV2");
    res = locator->ConnectServer(cimv2_root.get(), NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &cimv2_service);
    if ( FAILED(res) )
    {
        printf("Failed to connect root\\CIMV2\n");
        locator->Release();
    }

    default_root = make_bstr(L"root\\default");
    res = locator->ConnectServer(default_root.get(), NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &default_service);
    if ( FAILED(res) ) {
        printf("Failed to connect root\\default\n");
        cimv2_service->Release();
        locator->Release();
    }

    wql = make_bstr(L"WQL");
    stdregprov = make_bstr(L"StdRegProv");
}

WMIManager::~WMIManager() {
    if ( cimv2_service )
        cimv2_service->Release();
    if ( default_service )
        default_service->Release();
    if ( locator )
        locator->Release();
}

IEnumWbemClassObject* WMIManager::GetQueryEnumerator(const std::wstring& query) const {

    if ( ! wql || ! cimv2_service )
        return nullptr;

    auto b_query = make_bstr(query);
    if ( ! b_query )
        return nullptr;

    IEnumWbemClassObject* enumerator = nullptr;
    HRESULT res = cimv2_service->ExecQuery(wql.get(), b_query.get(), WBEM_FLAG_FORWARD_ONLY, NULL, &enumerator);
    if ( FAILED(res) )
    {
        IErrorInfo* error;
        (void)GetErrorInfo(0, &error);
        BSTR bSTRErrDesc = SysAllocString(L"__CLASS");
        error->GetHelpFile(&bSTRErrDesc);
        std::wcout << "Error with fetch: [" << res << "]" << "[" << bSTRErrDesc << "]" << std::endl;

        return nullptr;
    }

    return enumerator;
}

std::string WMIManager::GetOSVersion() const {
    
    IEnumWbemClassObject* enumerator = GetQueryEnumerator(L"SELECT * from Win32_OperatingSystem");
    if ( ! enumerator )
        return "";

    std::wstring version;

    HRESULT res;
    IWbemClassObject* obj = nullptr;
    int num_elems = 0;
    while ( (res = enumerator->Next(WBEM_INFINITE, 1, &obj, (ULONG*)&num_elems)) != WBEM_S_FALSE ) {
        if ( FAILED(res) )
            break;

        VARIANT var;
        VariantInit(&var);
        if ( SUCCEEDED(obj->Get(L"Caption", 0, &var, NULL, NULL)) && var.vt == VT_BSTR )
            version += var.bstrVal;

        if (! version.empty())
            version += L" ";

        VariantInit(&var);
        if ( SUCCEEDED(obj->Get(L"Version", 0, &var, NULL, NULL)) && var.vt == VT_BSTR )
            version += var.bstrVal;

        obj->Release();
    }

    enumerator->Release();

    return narrow_wstring(version);
}

std::vector<AccountInfo> WMIManager::GetUserData() const
{
    IEnumWbemClassObject* enumerator = GetQueryEnumerator(L"SELECT Caption, FullName, SID from Win32_UserAccount");
    if ( ! enumerator )
        return {};

    std::vector<AccountInfo> out;

    HRESULT res;
    IWbemClassObject* obj = nullptr;
    int num_elems = 0;
    while ( (res = enumerator->Next(WBEM_INFINITE, 1, &obj, (ULONG*)&num_elems)) != WBEM_S_FALSE ) {
        if ( FAILED(res) )
            break;

        AccountInfo info;

        VARIANT var;
        VariantInit(&var);
        if ( SUCCEEDED(obj->Get(L"Caption", 0, &var, NULL, NULL)) && var.vt == VT_BSTR )
            info.name = narrow_wstring(var.bstrVal);

        VariantInit(&var);
        if ( SUCCEEDED(obj->Get(L"FullName", 0, &var, NULL, NULL)) && var.vt == VT_BSTR )
            info.full_name = narrow_wstring(var.bstrVal);

        VariantInit(&var);
        if ( SUCCEEDED(obj->Get(L"SID", 0, &var, NULL, NULL)) && var.vt == VT_BSTR )
            info.sid = narrow_wstring(var.bstrVal);

        std::wstring path_query = L"SELECT LocalPath from Win32_UserProfile WHERE SID = \"";
        path_query += var.bstrVal;
        path_query += L"\"";

        std::wcout << path_query << std::endl;

        if ( IEnumWbemClassObject* user_enum = GetQueryEnumerator(path_query) )
        {
            IWbemClassObject* user_obj;
            if (user_enum->Next(WBEM_INFINITE, 1, &user_obj, (ULONG*)&num_elems) != WBEM_S_FALSE)
            {
                VariantInit(&var);
                if (SUCCEEDED(user_obj->Get(L"LocalPath", 0, &var, NULL, NULL)) && var.vt == VT_BSTR)
                    info.home_directory = narrow_wstring(var.bstrVal);

                user_obj->Release();
            }

            user_enum->Release();
        }

        out.push_back(std::move(info));
        obj->Release();
    }

    enumerator->Release();

    return out;
}

std::vector<LogEntry> WMIManager::GetLogs() const
{
    IEnumWbemClassObject* enumerator = GetQueryEnumerator(L"SELECT TimeGenerated, SourceName, EventType, Message from Win32_NTLogEvent");
    if ( ! enumerator )
        return {};

    printf("got enumerator\n");

    std::vector<LogEntry> out;

    HRESULT res;
    IWbemClassObject* obj = nullptr;
    int num_elems = 0;
    while ( (res = enumerator->Next(WBEM_INFINITE, 1, &obj, (ULONG*)&num_elems)) != WBEM_S_FALSE ) {
        if ( FAILED(res) )
            break;

        printf("%d\n", ++num_elems);

        LogEntry entry;

        VARIANT var;
        VariantInit(&var);
        if ( SUCCEEDED(obj->Get(L"SourceName", 0, &var, NULL, NULL)) && var.vt == VT_BSTR )
            entry.source = narrow_wstring(var.bstrVal);

        VariantClear(&var);
        if ( SUCCEEDED(obj->Get(L"Message", 0, &var, NULL, NULL)) && var.vt == VT_BSTR )
            entry.message = narrow_wstring(var.bstrVal);

        VariantClear(&var);
        if ( SUCCEEDED(obj->Get(L"EventType", 0, &var, NULL, NULL)) && var.vt == VT_UI8 )
            entry.priority = var.uintVal;

        VariantClear(&var);
        if ( SUCCEEDED(obj->Get(L"TimeGenerated", 0, &var, NULL, NULL)) && var.vt == VT_DATE )
            entry.ts = static_cast<int64_t>(var.date);

        VariantClear(&var);
        obj->Release();

        out.push_back(std::move(entry));
    }

    enumerator->Release();

    return out;
}
