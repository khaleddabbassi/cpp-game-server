#include "server_plugin_interface.h"

extern "C" __declspec(dllexport) void initPlugin(ServerInterface* server) {
    server->pluginCommandMap->operator[]("/testingplugins") = [server](SOCKET sock, const sockaddr_in& clientAddr,
                                                              const std::vector<std::string>& args,
                                                              const std::string& request) {
        server->addGuiLog("testingplugins command executed: " + request);
    };

    server->addGuiLog("/testingplugins Plugin Initialized with success stat.");
}