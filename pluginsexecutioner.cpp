#include "server_system.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <sstream>
#include "nativeservercmds.h"

std::unordered_map<std::string, PluginCommand> pluginCommandMap;

// Handle unknown commands
void handleUnknownCommand(SOCKET sock, const sockaddr_in& clientAddr,
                          const std::string& cmd, const std::vector<std::string>& args) {
    std::string msg = "interactor input:" + cmd;
    g_server->addGuiLog(msg);
    //asyncSend(sock, msg, clientAddr);

}


void executePluginCommand(SOCKET sock, const sockaddr_in& clientAddr,
                          const std::vector<std::string>& args, const std::string& request) {
    if (args.size() < 2) {
        g_server->addGuiLog("No command provided by source sender.");
					std::ostringstream oss;
		oss << "reason Args: ";
		for (const auto& arg : args) oss << arg << " ";
		g_server->addGuiLog(oss.str());
        return;
    }

    std::string cmd = args[1];

    auto it = pluginCommandMap.find(cmd);
    if (it != pluginCommandMap.end()) {
        // Execute the registered plugin command
        it->second(sock, clientAddr, args, request);
    } else {
        handleUnknownCommand(sock, clientAddr, cmd, args);
    }
}

// Register built-in plugin commands (can later be overridden by DLLs)
void registerDefaultPluginCommands() {
    pluginCommandMap["team"] = [](SOCKET sock, const sockaddr_in& clientAddr,
                                  const std::vector<std::string>& args,
                                  const std::string& request) {
        std::vector<std::string> reparsedArgs = reparseArgs(args);
        executePluginCommand(sock, clientAddr, reparsedArgs, request);
    };

    pluginCommandMap["server"] = [](SOCKET sock, const sockaddr_in& clientAddr,
                                    const std::vector<std::string>& args,
                                    const std::string& request) {
        std::vector<std::string> reparsedArgs = reparseArgs(args);
        executePluginCommand(sock, clientAddr, reparsedArgs, request);
    };

    pluginCommandMap["/sayhello"] = [](SOCKET sock, const sockaddr_in& clientAddr,
                                      const std::vector<std::string>& args,
                                      const std::string& request) {
        std::string message = "Hello from built-in plugin command!";
        g_server->addGuiLog("/sayhello executed: " + message);
    };
	
    pluginCommandMap["/clearall"] = [](SOCKET sock, const sockaddr_in& clientAddr,
                                      const std::vector<std::string>& args,
                                      const std::string& request) {
        server_clearlogs();
    };
	
    pluginCommandMap["/shutdown"] = [](SOCKET sock, const sockaddr_in& clientAddr,
                                      const std::vector<std::string>& args,
                                      const std::string& request) {
        server_shutdown();
    };
	
	pluginCommandMap["/restart"] = [](SOCKET sock, const sockaddr_in& clientAddr,
                                      const std::vector<std::string>& args,
                                      const std::string& request) {
        g_server->addGuiLog("restarting in progress ");
    };
	
	pluginCommandMap["/fetchpps"] = [](SOCKET sock, const sockaddr_in& clientAddr,
                                      const std::vector<std::string>& args,
                                      const std::string& request) {
        g_server->addGuiLog("printing player's permission codes...");
		
		//player sends callback request
		//server fetches data 
		// and sends it back to the client
		// for processing.
    };

}
