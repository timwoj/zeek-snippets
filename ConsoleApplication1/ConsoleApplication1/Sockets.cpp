#include "Sockets.h"

#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <Windows.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <winternl.h>
#include <ip2string.h>
#include <Psapi.h>

#include "utils.h"

// There's something really screwy about including windows headers that causes this
// to end up not defined. If I #include the header that defines it, I get an error
// about a conflict with winsock.h. If I don't include it, it's not defined.
#ifndef AF_INET6
#define AF_INET6 23
#endif

std::vector<Socket> Sockets::snapshot() const
{
    std::vector<Socket> result;

    getTCPSockets(result);
    //getTCP6Sockets(result);
    getUDPSockets(result);
    //getUDP6Sockets(result);

    return result;
}

void Sockets::getTCPSockets(std::vector<Socket>& result) const
{
    DWORD buffer_size = sizeof(MIB_TCPTABLE_OWNER_MODULE);
    DWORD ret = GetExtendedTcpTable(NULL, &buffer_size, FALSE, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0);
    if ((ret != NO_ERROR && ret != ERROR_INSUFFICIENT_BUFFER) || buffer_size < sizeof(MIB_TCPTABLE_OWNER_MODULE))
        return;
    
    char* table = new char[buffer_size];

    ret = GetExtendedTcpTable(table, &buffer_size, FALSE, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0);
    if (ret != NO_ERROR)
    {
        delete[] table;
        return;
    }

    char addr_str[16]{};
    struct in_addr ipaddr{};

    auto tcp_table = reinterpret_cast<PMIB_TCPTABLE_OWNER_MODULE>(table);
    for (DWORD i = 0; i < tcp_table->dwNumEntries; i++)
    {
        Socket s{};
        s.pid = tcp_table->table[i].dwOwningPid;
        s.process = getProcessFromPID(tcp_table->table[i].dwOwningPid);
        s.family = AF_INET;
        s.protocol = IPPROTO_TCP;
        s.local_port = ntohs(static_cast<uint16_t>(tcp_table->table[i].dwLocalPort));
        s.remote_port = ntohs(static_cast<uint16_t>(tcp_table->table[i].dwRemotePort));
        s.status = getTCPStateString(tcp_table->table[i].dwState);

        memcpy(&(ipaddr.S_un.S_addr), &(tcp_table->table[i].dwLocalAddr), sizeof(ipaddr));
        RtlIpv4AddressToStringA(&ipaddr, addr_str);
        s.local_addr = addr_str;

        memcpy(&(ipaddr.S_un.S_addr), &(tcp_table->table[i].dwRemoteAddr), sizeof(ipaddr));
        RtlIpv4AddressToStringA(&ipaddr, addr_str);
        s.remote_addr = addr_str;

        result.push_back(std::move(s));
    }

    delete[] table;
}

void Sockets::getTCP6Sockets(std::vector<Socket>& result) const
{
    DWORD buffer_size = sizeof(MIB_TCP6TABLE_OWNER_MODULE);
    DWORD ret = GetExtendedTcpTable(NULL, &buffer_size, FALSE, AF_INET6, TCP_TABLE_OWNER_MODULE_ALL, 0);
    if ((ret != NO_ERROR && ret != ERROR_INSUFFICIENT_BUFFER) || buffer_size < sizeof(MIB_TCP6TABLE_OWNER_MODULE))
        return;

    char* table = new char[buffer_size];
    ret = GetExtendedTcpTable(table, &buffer_size, FALSE, AF_INET6, TCP_TABLE_OWNER_MODULE_ALL, 0);
    if (ret != NO_ERROR)
    {
        delete[] table;
        return;
    }

    char addr_str[128]{};
    struct in6_addr ipaddr{};

    auto tcp_table = reinterpret_cast<PMIB_TCP6TABLE_OWNER_MODULE>(table);
    for (DWORD i = 0; i < tcp_table->dwNumEntries; i++)
    {
        Socket s{};
        s.pid = tcp_table->table[i].dwOwningPid;
        s.process = getProcessFromPID(tcp_table->table[i].dwOwningPid);
        s.family = AF_INET6;
        s.protocol = IPPROTO_TCP;
        s.local_port = ntohs(static_cast<uint16_t>(tcp_table->table[i].dwLocalPort));
        s.remote_port = ntohs(static_cast<uint16_t>(tcp_table->table[i].dwRemotePort));
        s.status = getTCPStateString(tcp_table->table[i].dwState);
        
        memcpy(ipaddr.u.Byte, tcp_table->table[i].ucLocalAddr, sizeof(ipaddr));
        RtlIpv6AddressToStringA(&ipaddr, addr_str);
        s.local_addr = addr_str;

        memcpy(ipaddr.u.Byte, tcp_table->table[i].ucRemoteAddr, sizeof(ipaddr));
        RtlIpv6AddressToStringA(&ipaddr, addr_str);
        s.remote_addr = addr_str;

        result.push_back(std::move(s));
    }

    delete[] table;
}

void Sockets::getUDPSockets(std::vector<Socket>& result) const
{
    DWORD buffer_size = 0;
    DWORD ret = GetExtendedUdpTable(NULL, &buffer_size, FALSE, AF_INET, UDP_TABLE_OWNER_MODULE, 0);
    if (ret != NO_ERROR && ret != ERROR_INSUFFICIENT_BUFFER)
        return;

    char* table = new char[buffer_size];

    ret = GetExtendedUdpTable(table, &buffer_size, FALSE, AF_INET, UDP_TABLE_OWNER_MODULE, 0);
    if (ret != NO_ERROR)
    {
        delete[] table;
        return;
    }

    char addr_str[16]{};
    struct in_addr ipaddr{};

    auto udp_table = reinterpret_cast<PMIB_UDPTABLE_OWNER_MODULE>(table);
    for (DWORD i = 0; i < udp_table->dwNumEntries; i++)
    {
        Socket s{};
        s.pid = udp_table->table[i].dwOwningPid;
        s.process = getProcessFromPID(udp_table->table[i].dwOwningPid);
        s.family = AF_INET;
        s.protocol = IPPROTO_UDP;
        s.local_port = ntohs(static_cast<uint16_t>(udp_table->table[i].dwLocalPort));

        memcpy(&(ipaddr.S_un.S_addr), &(udp_table->table[i].dwLocalAddr), sizeof(ipaddr));
        RtlIpv4AddressToStringA(&ipaddr, addr_str);
        s.local_addr = addr_str;

        result.push_back(std::move(s));
    }

    delete[] table;
}

void Sockets::getUDP6Sockets(std::vector<Socket>& result) const
{
    DWORD buffer_size = 0;
    DWORD ret = GetExtendedUdpTable(NULL, &buffer_size, FALSE, AF_INET6, UDP_TABLE_OWNER_MODULE, 0);
    if (ret != NO_ERROR && ret != ERROR_INSUFFICIENT_BUFFER)
        return;
    
    char* table = new char[buffer_size];

    ret = GetExtendedUdpTable(table, &buffer_size, FALSE, AF_INET6, UDP_TABLE_OWNER_MODULE, 0);
    if (ret != NO_ERROR)
    {
        delete[] table;
        return;
    }

    char addr_str[128]{};
    struct in6_addr ipaddr{};

    auto udp_table = reinterpret_cast<PMIB_UDP6TABLE_OWNER_MODULE>(table);
    for (DWORD i = 0; i < udp_table->dwNumEntries; i++)
    {
        Socket s{};
        s.pid = udp_table->table[i].dwOwningPid;
        s.process = getProcessFromPID(udp_table->table[i].dwOwningPid);
        s.family = AF_INET6;
        s.protocol = IPPROTO_UDP;
        s.local_port = ntohs(static_cast<uint16_t>(udp_table->table[i].dwLocalPort));

        memcpy(ipaddr.u.Byte, udp_table->table[i].ucLocalAddr, sizeof(ipaddr));
        RtlIpv6AddressToStringA(&ipaddr, addr_str);
        s.local_addr = addr_str;

        result.push_back(std::move(s));
    }

    delete[] table;
}

std::string Sockets::getProcessFromPID(unsigned long pid) const
{
    // TODO: this could do some caching to avoid repeat lookups but it'd have to
    // be careful of the PID swapping to some other process which happens on Windows
    // sometimes.

    if ( pid == 4 )
        // PID 4 is always the windows system process. See https://superuser.com/a/571470/105420.
        return "System";
    else if ( pid == 0 )
        // PID 0 is always the system idle process.
        return "System Idle Process";

    // This requires the process to be run as Administrator in order to get the information needed
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (! process)
    {
        printf("Failed to open process %lu: %lu\n", pid, GetLastError());
        return {};
    }

    char name[MAX_PATH];
    if (GetProcessImageFileNameA(process, name, sizeof(name)) == 0)
    {
        printf("Failed to get process name from %lu: %lu\n", pid, GetLastError());
        return {};
    }

    return {name};
}

std::string Sockets::getTCPStateString(unsigned long state) const
{
    switch (state)
    {
    case MIB_TCP_STATE_CLOSED: return "CLOSED";
    case MIB_TCP_STATE_LISTEN: return "LISTEN";
    case MIB_TCP_STATE_SYN_SENT: return"SYN_SENT";
    case MIB_TCP_STATE_SYN_RCVD: return "SYN_RECEIVED";
    case MIB_TCP_STATE_ESTAB: return "ESTABLISHED";
    case MIB_TCP_STATE_FIN_WAIT1: return "FIN_WAIT_1";
    case MIB_TCP_STATE_FIN_WAIT2: return "FIN_WAIT_2";
    case MIB_TCP_STATE_CLOSE_WAIT: return "CLOSE_WAIT";
    case MIB_TCP_STATE_CLOSING: return "CLOSING";
    case MIB_TCP_STATE_LAST_ACK: return "LAST_ACK";
    case MIB_TCP_STATE_TIME_WAIT: return "TIME_WAIT";
    case MIB_TCP_STATE_DELETE_TCB: return "DELETE_TCB";
    default: return "unknown";
    }
}