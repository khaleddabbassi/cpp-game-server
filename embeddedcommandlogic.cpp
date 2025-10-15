#include "embeddedcommands.h"
#include "server_system.h"


void server_clearlogs() {
    std::lock_guard<std::mutex> lock(guiLogMutex);

    std::queue<std::string> emptyQueue;
    std::swap(guiLogQueue, emptyQueue);

    logLines.clear();
}

void server_shutdown(){
	
	running = false;
	
}

void server_restart(){
	
	g_server->addGuiLog("the server will restart");
	
}


