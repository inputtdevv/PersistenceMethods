#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <string>





// Add any malicious functions here idgaf


void enableVTMode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void gradientAdder(const std::string& text, int r1, int g1, int b1, int r2, int g2, int b2) {
    int len = text.length();
    for (int i = 0; i < len; ++i) {
        int r = r1 + (r2 - r1) * i / len;
        int g = g1 + (g2 - g1) * i / len;
        int b = b1 + (b2 - b1) * i / len;
        std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m" << text[i];
    }
    std::cout << "\033[0m\n";
}

void Success() {
    std::cout << "\033[38;2;0;255;0m[+] \033[38;2;144;238;144mExe was added to startup\033[0m\n";
}

void Failed() {
    std::cout << "\033[38;2;139;0;0m[-] \033[38;2;255;0;0mFailed to add exe to startup.\033[0m\n";
}

void alreadyThere() {
    gradientAdder("[=] Exe is already in startup", 255, 140, 0, 255, 255, 0);
}

int main() {
    enableVTMode();

    char startupPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        std::filesystem::path destPath = std::filesystem::path(startupPath) / std::filesystem::path(exePath).filename();

        try {
            if (std::filesystem::exists(destPath)) {
                alreadyThere();
            }
            else {
                std::filesystem::copy_file(exePath, destPath, std::filesystem::copy_options::overwrite_existing);
                Success();
            }
        }
        catch (...) {
            Failed();
        }
    }
    else {
        Failed();
    }

    for (int i = 0; i < 10; ++i) std::cout << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}
