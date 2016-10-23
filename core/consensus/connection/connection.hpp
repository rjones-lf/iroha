#ifndef __CONNECTION__
#define __CONNECTION__

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace connection {

    struct Config{
        std::string name;
        std::string ip_addr;
        std::string port;
    };

    void initialize_peer(std::unique_ptr<connection::Config> config);

    bool sendAll(const std::string& message);
    bool send(const std::string& to, const std::string& message);
    bool receive(const std::function<void(std::string from, std::string message)>& callback);

};  // end connection

#endif
