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
							 
    if (command == "engine") { //client engine updates
        g_server->addGuiLog("engine query detected");
    }
	
    else if (command == "system") { //connections registrations
        g_server->addGuiLog("system query detected");
    }
	
	else if (command == "say") { //plugins facelet executes logic at the server only
		executePluginCommand(sock, clientAddr, args, request);      
    }
	
	else if (command == "callbacks") { //callbacks facelet executes logic at the client side
		//excallbacks(sock, clientAddr, args, request);      
    }

    else {
		
		g_server->addGuiLog(request);
    
	}
}
