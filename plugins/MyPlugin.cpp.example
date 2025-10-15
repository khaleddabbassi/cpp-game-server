#include "server_plugin_interface.h"

extern "C" __declspec(dllexport) void initPlugin(ServerInterface* server) {
    server->pluginCommandMap->operator[]("/example") = [server](SOCKET sock, const sockaddr_in& clientAddr,
                                                              const std::vector<std::string>& args,
                                                              const std::string& request) {
        server->addGuiLog("Example command executed: " + request);
    };

    server->addGuiLog("/example Plugin Initialized with success status.");
}