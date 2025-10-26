#include "server_system.h"
#include "server_plugin_interface.h"
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <winsock2.h>
#include <sstream> // needed for std::istringstream

// Reparse arguments by skipping the first element
std::vector<std::string> reparseArgs(const std::vector<std::string>& args) {
    if (args.empty()) return {};
    return std::vector<std::string>(args.begin() + 1, args.end());
}

// Split a message into words (space-separated)
std::vector<std::string> splitWords(const std::string& msg) {
    std::vector<std::string> words;
    std::istringstream iss(msg);
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    return words;
}

// Enqueue a message and optionally store the client address
void enqueueMessage(const std::string& msg, const sockaddr_in* clientAddr) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.push(msg);
    }

    if (clientAddr) {
        std::lock_guard<std::mutex> lock(clientAddrMutex);
        clientAddrList.push_back(*clientAddr);
    }

    queueCondVar.notify_one();
}
