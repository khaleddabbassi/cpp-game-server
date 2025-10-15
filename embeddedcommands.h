#pragma once
#include <string>
#include <queue>
#include <deque>

extern std::queue<std::string> guiLogQueue;
extern std::deque<std::string> logLines;

void server_shutdown();
void server_restart();
void server_clearlogs();
