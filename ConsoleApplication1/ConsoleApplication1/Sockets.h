#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Socket
{
    int64_t pid;
    std::string process;
    int64_t family;
    int64_t protocol;
    int64_t local_port;
    int64_t remote_port;
    std::string local_addr;
    std::string remote_addr;
    std::string status;
};

class Sockets
{
public:
    [[nodiscard]] std::vector<Socket> snapshot() const;

private:

    void getTCPSockets(std::vector<Socket>& result) const;
    void getTCP6Sockets(std::vector<Socket>& result) const;
    void getUDPSockets(std::vector<Socket>& result) const;
    void getUDP6Sockets(std::vector<Socket>& result) const;
    
    [[nodiscard]] std::string getProcessFromPID(unsigned long pid) const;
    [[nodiscard]] std::string getTCPStateString(unsigned long state) const;
};
