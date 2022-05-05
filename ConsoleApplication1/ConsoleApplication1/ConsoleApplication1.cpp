// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <algorithm>
#include <iostream>

#include "utils.h"
#include "Processes.h"
#include "WMIManager.h"
#include "EventLogManager.h"
#include "Sockets.h"
#include "Glob.h"

#include "psapi.h"

void users()
{
    WMIManager mgr;
    auto users = mgr.GetUserData();

    printf("got %llu users\n\n", users.size());
    for ( const auto& user : users )
    {
        std::cout << std::endl;
        std::cout << "name:      " << user.name << std::endl;
        std::cout << "full_name: " << user.full_name << std::endl;
        std::cout << "sid:       " << user.sid << std::endl;
        std::cout << "home:      " << user.home_directory << std::endl;
    }

}

void os_version()
{
    WMIManager mgr;
    std::cout << mgr.GetOSVersion() << std::endl;
}

void processes_enum()
{
    Processes::GetFromEnum();
}

void processes_snapshot()
{
    Processes::GetFromSnapshot();
}

void logs()
{
    EventLogManager elm;
    auto result = elm.GetLogEntries();

    printf("got %llu results from event log\n\n", result.size());
    for (const auto& res : result)
    {
        printf("%lld (%lld): %s: %s\n", res.ts, res.priority, res.source.c_str(), res.message.c_str());
    }
}

void wmi_logs()
{
    WMIManager mgr;
    auto result = mgr.GetLogs();

    printf("got %llu results from event log\n\n", result.size());
    for (const auto& res : result)
    {
        printf("%lld (%lld): %s: %s\n", res.ts, res.priority, res.source.c_str(), res.message.c_str());
    }
}

void sockets()
{
    Sockets s;
    auto result = s.snapshot();

    printf("got %llu sockets\n\n", result.size());

    std::sort(result.begin(), result.end(), [](const Socket& a, const Socket& b)
    {
        if (a.local_addr == b.local_addr)
        {
            if (a.local_port == b.local_port)
            {
                if (a.remote_addr == b.remote_addr)
                {
                    if (a.remote_port == b.remote_port)
                        return a.pid < b.pid;

                    return a.remote_port < b.remote_port;
                }

                return a.remote_addr.compare(b.remote_addr) < 0;
            }

            return a.local_port < b.local_port;
        }

        return a.local_addr.compare(b.local_addr) < 0;
    });

    std::string local, remote;
    for (const auto& sock : result)
    {
        if (sock.family == AF_INET)
        {
            local = sock.local_addr;
            remote = sock.remote_addr;
        }
        else
        {
            local = "[" + sock.local_addr + "]";
            remote = "[" + sock.remote_addr + "]";
        }

        std::string addr_open = sock.family == AF_INET ? "" : "[";
        std::string addr_close = sock.family == AF_INET ? "" : "]";

        if (sock.protocol == IPPROTO_TCP)
        {
            printf("%s (%5lld)  %lld %lld\t%s:%lld\t%s:%lld\t%s\n", 
                sock.process.c_str(), sock.pid, sock.family, sock.protocol, 
                local.c_str(), sock.local_port,
                remote.c_str(), sock.remote_port, sock.status.c_str());
        }
        else
        {
            printf("%s (%5lld)  %lld %lld\t%s:%lld\t*:*\n", 
                sock.process.c_str(), sock.pid, sock.family, sock.protocol, 
                local.c_str(), sock.local_port);
        }
    }
}

void glob()
{
    glob_t g{};
}

int main()
{
    processes_snapshot();
}
