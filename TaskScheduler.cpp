#include <iostream>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <string>
#include <taskschd.h>
#include <comdef.h>

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
    std::cout << "\033[38;2;0;255;0m[+] \033[38;2;144;238;144mExe was added to Task Scheduler and copied to LocalAppData\033[0m\n";
}

void Failed3(DWORD error = 0) {
    std::cout << "\033[38;2;139;0;0m[-] \033[38;2;255;0;0mFailed to add exe to Task Scheduler or copy to LocalAppData. Error: " << error << "\033[0m\n";
}

void There3() {
    gradientAdder3("[=] Task is already in Task Scheduler", 255, 140, 0, 255, 255, 0);
}

int main() {
    enableVTMode();
    const std::string folderName = "InputtTasks"; // folder name in localappdata
    const std::string appName = "Inputt"; // exe/app name
    const std::wstring taskName = L"InputtTask"; // name for task

    char localAppDataPath[MAX_PATH];
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppDataPath))) {
        Failed3(GetLastError());
        return 1;
    }

    std::filesystem::path destDir = std::filesystem::path(localAppDataPath) / folderName;
    std::filesystem::path destPath = destDir / (appName + ".exe");

    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        Failed3(GetLastError());
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
        return 1;
    }

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ITaskService* pService = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        CoUninitialize();
        Failed3(hr);
        return 1;
    }

    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        pService->Release();
        CoUninitialize();
        Failed3(hr);
        return 1;
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        pService->Release();
        CoUninitialize();
        Failed3(hr);
        return 1;
    }

    IRegisteredTask* pExistingTask = NULL;
    hr = pRootFolder->GetTask(_bstr_t(taskName.c_str()), &pExistingTask);
    if (SUCCEEDED(hr)) {
        pExistingTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        There3();
    }
    else {
        ITaskDefinition* pTask = NULL;
        hr = pService->NewTask(0, &pTask);
        if (FAILED(hr)) {
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            Failed3(hr);
            return 1;
        }

        ITriggerCollection* pTriggerCollection = NULL;
        hr = pTask->get_Triggers(&pTriggerCollection);
        if (FAILED(hr)) {
            pTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            Failed3(hr);
            return 1;
        }

        ITrigger* pTrigger = NULL;
        hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
        pTriggerCollection->Release();
        if (FAILED(hr)) {
            pTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            Failed3(hr);
            return 1;
        }
        pTrigger->Release();

        IActionCollection* pActionCollection = NULL;
        hr = pTask->get_Actions(&pActionCollection);
        if (FAILED(hr)) {
            pTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            Failed3(hr);
            return 1;
        }

        IAction* pAction = NULL;
        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
        pActionCollection->Release();
        if (FAILED(hr)) {
            pTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            Failed3(hr);
            return 1;
        }

        IExecAction* pExecAction = NULL;
        hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
        pAction->Release();
        if (FAILED(hr)) {
            pTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            Failed3(hr);
            return 1;
        }

        hr = pExecAction->put_Path(_bstr_t(destPath.wstring().c_str()));
        pExecAction->Release();
        if (FAILED(hr)) {
            pTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            Failed3(hr);
            return 1;
        }

        IRegisteredTask* pRegisteredTask = NULL;
        hr = pRootFolder->RegisterTaskDefinition(_bstr_t(taskName.c_str()), pTask, TASK_CREATE_OR_UPDATE, _variant_t(), _variant_t(), TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(L""), &pRegisteredTask);
        if (FAILED(hr)) {
            pTask->Release();
            pRootFolder->Release();
            pService->Release();
            CoUninitialize();
            Failed3(hr);
            return 1;
        }

        pRegisteredTask->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        Success3();
    }

    for (int i = 0; i < 10; ++i) std::cout << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}
