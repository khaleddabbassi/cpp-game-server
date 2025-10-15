#pragma once
#include "server_plugin_interface.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <winsock2.h>
#include <windows.h>
#include <atomic>
#include <mutex>
#include <queue>
#include <deque>
#include <condition_variable>

using CommandHandler = std::function<void(SOCKET, const sockaddr_in&, const std::vector<std::string>&, const std::string&)>;

using PluginCommand = std::function<void(SOCKET, const sockaddr_in&, const std::vector<std::string>&, const std::string&)>;

extern std::atomic<bool> running;
extern std::atomic<bool> guiRunning;
extern std::vector<HMODULE> g_pluginHandles;

extern std::mutex queueMutex;
extern std::condition_variable queueCondVar;
extern std::queue<std::string> messageQueue;

extern std::mutex clientAddrMutex;
extern std::deque<sockaddr_in> clientAddrList;

extern std::unordered_map<std::string, CommandHandler> commandMap;
extern std::unordered_map<std::string, PluginCommand> pluginCommandMap;

extern std::mutex serverMutex;
extern std::condition_variable serverCondVar;

extern std::mutex guiLogMutex;
extern std::queue<std::string> guiLogQueue;

// New GUI-related variables
extern std::mutex commandMutex;  // Protects commandQueue
extern std::queue<std::string> commandQueue;  // Stores commands from GUI
extern std::condition_variable commandCondVar;  // Notifies server of new commands

extern ServerInterface* g_server;

void registerDefaultPluginCommands();
void testerman(SOCKET sock, const sockaddr_in& clientAddr,
               const std::vector<std::string>& parts, const std::string& fullMsg);
void addGuiLog(const std::string& message);
void runServer();
void runGui();
void enqueueMessage(const std::string& msg, const sockaddr_in* clientAddr);
void asyncSend(SOCKET sock, const std::string& message, const sockaddr_in& clientAddr);
void executePluginCommand(SOCKET sock, const sockaddr_in& clientAddr, const std::vector<std::string>& args, const std::string& request);
void handleUnknownCommand(SOCKET sock, const sockaddr_in& clientAddr, const std::string& cmd, const std::vector<std::string>& args);

void handleSystemCommand(SOCKET sock,
                         const sockaddr_in& clientAddr,
                         const std::string& command,
                         const std::vector<std::string>& args,
                         const std::string& request);

std::vector<std::string> splitWords(const std::string& msg);
std::vector<std::string> reparseArgs(const std::vector<std::string>& args);

void loadAllPluginsFromFolder(const std::string& folderPath);
void unloadAllPlugins();