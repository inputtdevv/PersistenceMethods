#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <string>
#include <fstream>

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
    std::cout << "\033[38;2;0;255;0m[+] \033[38;2;144;238;144mExe was added to PowerShell Profile\033[0m\n";
}

void Failed3(DWORD error = 0) {
    std::cout << "\033[38;2;139;0;0m[-] \033[38;2;255;0;0mFailed to add exe to PowerShell profile. Error: " << error << "\033[0m\n";
}

void There3() {
    gradientAdder3("[=] Exe is already in PowerShell profile", 255, 140, 0, 255, 255, 0);
}

int main() {
    enableVTMode();
    const std::string folderName = "InputtShell"; // name of folder
    const std::string appName = "Inputt"; // name of app/exe

    char localAppDataPath[MAX_PATH];
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppDataPath))) {
        Failed3(GetLastError());
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    std::filesystem::path destDir = std::filesystem::path(localAppDataPath) / folderName;
    std::filesystem::path destPath = destDir / (appName + ".exe");

    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        Failed3(GetLastError());
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    try {
        std::filesystem::create_directories(destDir);
        if (!std::filesystem::exists(destPath) || !std::filesystem::equivalent(exePath, destPath)) {
            std::filesystem::copy_file(exePath, destPath, std::filesystem::copy_options::overwrite_existing);
        }
    }
    catch (...) {
        Failed3();
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    wchar_t* profilePath = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &profilePath))) {
        Failed3(GetLastError());
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    std::wstring psProfilePath = std::wstring(profilePath) + L"\\WindowsPowerShell\\Microsoft.PowerShell_profile.ps1";
    CoTaskMemFree(profilePath);

    try {
        std::filesystem::create_directories(std::filesystem::path(psProfilePath).parent_path());
        std::ifstream inFile(psProfilePath);
        std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();

        std::string psCommand = "Start-Process \"" + destPath.string() + "\"\n";
        if (content.find(destPath.string()) != std::string::npos) {
            There3();
            std::cout << "Press Enter to exit...";
            std::cin.get();
            return 0;
        }

        std::ofstream outFile(psProfilePath, std::ios::app);
        if (!outFile.is_open()) {
            Failed3(GetLastError());
            std::cout << "Press Enter to exit...";
            std::cin.get();
            return 1;
        }

        outFile << psCommand;
        outFile.close();
        Success3();
    }
    catch (...) {
        Failed3();
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
}
