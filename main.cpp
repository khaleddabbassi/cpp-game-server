#include "server_system.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <future>
#include <chrono>
#include <queue>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <iostream>   // <— add this line

// Define global variables
std::atomic<bool> running{true};
std::vector<HMODULE> g_pluginHandles;
std::mutex queueMutex;
std::condition_variable queueCondVar;
std::queue<std::string> messageQueue;
std::mutex clientAddrMutex;
std::deque<sockaddr_in> clientAddrList;
std::mutex serverMutex;
std::condition_variable serverCondVar;
ServerInterface* g_server = nullptr;


void asyncReceive(SOCKET sock) {
    char buffer[1024];
    sockaddr_in clientAddr{};
    int addrLen = sizeof(clientAddr);

    while (running) {
        int bytesReceived = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                     (sockaddr*)&clientAddr, &addrLen);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string msg(buffer);

            enqueueMessage(msg, &clientAddr);

        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void asyncProcessMessages(SOCKET sock) {
    while (running) {
        // Wait for a message in the queue
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondVar.wait(lock, [] { return !messageQueue.empty() || !running; });

        if (!running) break;

        std::string msg = messageQueue.front();
        messageQueue.pop();
        lock.unlock();

        // Get client address
        sockaddr_in clientAddr{};
        {
            std::lock_guard<std::mutex> lock2(clientAddrMutex);
            if (!clientAddrList.empty()) {
                clientAddr = clientAddrList.front();
                clientAddrList.pop_front();
            }
        }

        auto parts = splitWords(msg);
        if (!parts.empty()) {
            // the first token is the command name
            const std::string& command = parts[0];
            // the rest of parts can be passed directly as args
            handleSystemCommand(sock, clientAddr, command, parts, msg);
        }
    }
}


void asyncSend(SOCKET sock, const std::string& message, const sockaddr_in& clientAddr) {
    auto _ = std::async(std::launch::async, [sock, message, clientAddr]() {
        int bytesSent = sendto(sock, message.c_str(), message.length(), 0,
                               (const sockaddr*)&clientAddr, sizeof(clientAddr));
        if (bytesSent == SOCKET_ERROR) {
            g_server->addGuiLog("Failed to send message to " +
                                std::string(inet_ntoa(clientAddr.sin_addr)) + ":" +
                                std::to_string(ntohs(clientAddr.sin_port)) + ": " +
                                std::to_string(WSAGetLastError()));
        } else {
            g_server->addGuiLog("Sent message to " +
                                std::string(inet_ntoa(clientAddr.sin_addr)) + ":" +
                                std::to_string(ntohs(clientAddr.sin_port)) + ": " + message);
        }
    });
}

void runServer() {
	
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        g_server->addGuiLog("WSAStartup failed");
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        g_server->addGuiLog("Socket creation failed");
        WSACleanup();
        return;
    }

    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(8080);

    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        g_server->addGuiLog("Bind failed: " + std::to_string(WSAGetLastError()));
        closesocket(sock);
        WSACleanup();
        return;
    }

    g_server->addGuiLog("Server listening on 127.0.0.1:8080");

    auto recvFut = std::async(std::launch::async, asyncReceive, sock);
    auto processFut = std::async(std::launch::async, asyncProcessMessages, sock);

    {
        std::unique_lock<std::mutex> lock(serverMutex);
        serverCondVar.wait(lock, [] { return !running; });
    }

    queueCondVar.notify_all();

    recvFut.wait();
    processFut.wait();

    closesocket(sock);
    WSACleanup();
    g_server->addGuiLog("Server shut down gracefully");
}

int main() {

    // Initialize ServerInterface
    ServerInterface serverInterface;
    serverInterface.addGuiLog = addGuiLog;
    serverInterface.pluginCommandMap = &pluginCommandMap;
    g_server = &serverInterface;

    // Load plugins
	registerDefaultPluginCommands();
    loadAllPluginsFromFolder("plugins");
	

    // Start server and GUI threads
    std::thread serverThread(runServer);
    std::thread guiThread(runGui);

    // Wait for threads to finish
    serverThread.join();
    guiThread.join();    

    // Cleanup
    unloadAllPlugins();
    WSACleanup();

    return 0;
}
