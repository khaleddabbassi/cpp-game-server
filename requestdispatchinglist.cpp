#include "server_system.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>

// Handles incoming system commands from clients
void handleSystemCommand(SOCKET sock,
                         const sockaddr_in& clientAddr,
                         const std::string& command,
                         const std::vector<std::string>& args,
                         const std::string& request) 
{
    // Engine updates (e.g., position, state)
    if (command == "engine") {
        g_server->addGuiLog("Engine update received from client port " + std::to_string(ntohs(clientAddr.sin_port)));
        // TODO: process engine update here
    }

    // System-level requests (e.g., registration, connections)
    else if (command == "system") {
        g_server->addGuiLog("System connection request received from client port " + std::to_string(ntohs(clientAddr.sin_port)));
        // TODO: handle new client registration
    }

    // Plugin command execution on the server
    else if (command == "say") {
        executePluginCommand(sock, clientAddr, args, request);
        g_server->addGuiLog("Plugin command executed: " + request);
    }

    // Callback procedure triggered for client-side execution
    else if (command == "callback") {
        // excallbacks(sock, clientAddr, args, request); // Uncomment when implemented
        g_server->addGuiLog("Client callback procedure initiated from port " + std::to_string(ntohs(clientAddr.sin_port)));
    }

    // Unknown or raw requests
    else {
        g_server->addGuiLog("Unrecognized command received: " + request);
    }
}
