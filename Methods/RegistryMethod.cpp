#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <string>
// Detected by AV you can obfuscate it or smth idrc
void enableVTMode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void gradientAdder2(const std::string& text, int r1, int g1, int b1, int r2, int g2, int b2) {
    size_t len = text.length();
    for (size_t i = 0; i < len; ++i) {
        int r = r1 + (r2 - r1) * static_cast<int>(i) / static_cast<int>(len);
        int g = g1 + (g2 - g1) * static_cast<int>(i) / static_cast<int>(len);
        int b = b1 + (b2 - b1) * static_cast<int>(i) / static_cast<int>(len);
        std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m" << text[i];
    }
    std::cout << "\033[0m\n";
}

void Success2() {
    std::cout << "\033[38;2;0;255;0m[+] \033[38;2;144;238;144mExe was added to registry and copied to LocalAppData\033[0m\n";
}

void Failed2(DWORD error = 0) {
    std::cout << "\033[38;2;139;0;0m[-] \033[38;2;255;0;0mFailed to add exe to registry or copy to LocalAppData. Error: " << error << "\033[0m\n";
}

void There() {
    gradientAdder2("[=] Exe is already in registry", 255, 140, 0, 255, 255, 0);
}

int main() {
    enableVTMode();
    const std::string folderName = "Inputt On Top"; // name for folder in localappdata
    const std::string appName = "Inputt"; // exe/app name

    char localAppDataPath[MAX_PATH];
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppDataPath))) {
        Failed2(GetLastError());
        return 1;
    }

    std::filesystem::path destDir = std::filesystem::path(localAppDataPath) / folderName;
    std::filesystem::path destPath = destDir / (appName + ".exe");

    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        Failed2(GetLastError());
        return 1;
    }

    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
    if (result != ERROR_SUCCESS) {
        Failed2(result);
        return 1;
    }

    char regValue[MAX_PATH];
    DWORD regValueSize = sizeof(regValue);
    bool existsInRegistry = (RegQueryValueExA(hKey, appName.c_str(), NULL, NULL, (LPBYTE)regValue, &regValueSize) == ERROR_SUCCESS);

    try {
        std::filesystem::create_directories(destDir);

        if (!std::filesystem::exists(destPath) || !std::filesystem::equivalent(exePath, destPath)) {
            std::filesystem::copy_file(exePath, destPath, std::filesystem::copy_options::overwrite_existing);
        }

        if (!existsInRegistry) {
            std::string destPathStr = destPath.string();
            result = RegSetValueExA(hKey, appName.c_str(), 0, REG_SZ, (const BYTE*)destPathStr.c_str(), static_cast<DWORD>(destPathStr.length() + 1));
            if (result != ERROR_SUCCESS) {
                RegCloseKey(hKey);
                Failed2(result);
                return 1;
            }
            Success2();
        }
        else {
            There();
        }
    }
    catch (...) {
        RegCloseKey(hKey);
        Failed2();
        return 1;
    }

    RegCloseKey(hKey);

    for (int i = 0; i < 10; ++i) std::cout << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}
