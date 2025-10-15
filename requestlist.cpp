#include "server_system.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>

void handleSystemCommand(SOCKET sock,
                         const sockaddr_in& clientAddr,
                         const std::string& command,
                         const std::vector<std::string>& args,
                         const std::string& request){
							 
    if (command == "engineupdate") {
        g_server->addGuiLog("engine query detected");
    }
	
    else if (command == "system") {
        g_server->addGuiLog("system query detected");
    }

    else {
        executePluginCommand(sock, clientAddr, args, request);
    }
}
