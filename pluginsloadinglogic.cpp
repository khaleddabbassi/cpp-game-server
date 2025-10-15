#include "server_system.h"
#include "server_plugin_interface.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

// Load a single plugin DLL
void loadPlugin(const std::string& dllPath) {
    HMODULE hLib = LoadLibraryA(dllPath.c_str());
    if (!hLib) {
        g_server->addGuiLog("Failed to load plugin DLL: " + dllPath);
        return;
    }

    g_pluginHandles.push_back(hLib);

    auto init = (PluginInitFunc)GetProcAddress(hLib, "initPlugin");
    if (init) {
        try {
            init(g_server);
            g_server->addGuiLog("Plugin loaded: " + dllPath);
        } catch (const std::exception& e) {
            g_server->addGuiLog("Plugin threw exception: " + std::string(e.what()));
        } catch (...) {
            g_server->addGuiLog("Plugin threw unknown exception: " + dllPath);
        }
    } else {
        g_server->addGuiLog("initPlugin not found in: " + dllPath);
    }
}

// Load all DLLs from a folder
void loadAllPluginsFromFolder(const std::string& folderPath) {
    if (!fs::exists(folderPath)) {
        g_server->addGuiLog("Plugins folder does not exist: " + folderPath);
        return;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            loadPlugin(entry.path().string());
        }
    }
}

// Unload all loaded plugins
void unloadAllPlugins() {
    for (auto hLib : g_pluginHandles)
        FreeLibrary(hLib);
    g_pluginHandles.clear();
    g_server->addGuiLog("All plugins unloaded.");
}
