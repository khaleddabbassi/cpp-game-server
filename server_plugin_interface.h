#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <winsock2.h>

struct ServerInterface;

using PluginCommand = std::function<void(SOCKET, const sockaddr_in&, const std::vector<std::string>&, const std::string&)>;

extern "C" {
    typedef void (*PluginInitFunc)(ServerInterface* server);
}

struct ServerInterface {
    void (*addGuiLog)(const std::string& message);
    std::unordered_map<std::string, PluginCommand>* pluginCommandMap;
};