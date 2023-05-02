#include <iostream>
#include <string>
#include <Windows.h>
#include <Shlobj.h>
#include <comdef.h>
#include <taskschd.h>

#pragma comment(lib, "taskschd.lib")

// Function to delete a file
bool DeleteFile(const std::wstring& filePath) {
    if (DeleteFileW(filePath.c_str())) {
        std::wcout << "File deleted: " << filePath << std::endl;
        return true;
    }
    else {
        std::wcerr << "Failed to delete file: " << filePath << std::endl;
        return false;
    }
}

// Function to stop and delete a task
bool DeleteTask(const std::wstring& taskName) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library." << std::endl;
        return false;
    }

    ITaskService* pService = NULL;
    hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        std::cerr << "Failed to create Task Scheduler instance." << std::endl;
        CoUninitialize();
        return false;
    }

    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        std::cerr << "Failed to connect to Task Scheduler service." << std::endl;
        pService->Release();
        CoUninitialize();
        return false;
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        std::cerr << "Failed to get the root folder of Task Scheduler." << std::endl;
        pService->Release();
        CoUninitialize();
        return false;
    }

    IRegisteredTask* pRegisteredTask = NULL;
    hr = pRootFolder->GetTask(_bstr_t(taskName.c_str()), &pRegisteredTask);
    if (FAILED(hr)) {
        std::cerr << "Failed to get the registered task." << std::endl;
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return false;
    }

    hr = pRegisteredTask->Stop(0);
    if (FAILED(hr)) {
        std::cerr << "Failed to stop the task." << std::endl;
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return false;
    }

    hr = pRootFolder->DeleteTask(_bstr_t(taskName.c_str()), 0);
    if (FAILED(hr)) {
        std::cerr << "Failed to delete the task." << std::endl;
        pRegisteredTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return false;
    }

    //std::cout << "Task deleted: " << taskName << std::endl;
    printf("Task Deleted!\n");

    pRegisteredTask->Release();
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();

    return true;
}

int main(int argc, char* argv[]) {
    std::wstring system32Dir;
    PWSTR system32Path = NULL;

    // Retrieve the System32 directory path
    if (SHGetKnownFolderPath(FOLDERID_System, 0, NULL, &system32Path) != S_OK) {
        std::cerr << "Failed to retrieve the System32 directory path." << std::endl;
        return 1;
    }

    system32Dir = system32Path;
    CoTaskMemFree(system32Path);

    // Check if the command line argument is provided
    if (argc < 2) {
        std::cerr << "Usage: program_name.exe <command>" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    // Process the command
    if (command == "/uninstall") {
        std::wstring filePath = system32Dir + L"\\MBRHELPER.exe";
        system("taskkill /f /im MBRHELPER.exe");

        // Delete the file
        if (DeleteFile(filePath)) {
            // Delete the task
            if (DeleteTask(L"MBR Utility")) {
                std::cout << "Uninstallation completed successfully. And There's A MBR Backup still left in System32\n As a bonus you can recover your MBR whenever you want!" << std::endl;
                return 0;
            }
            else {
                std::cerr << "Failed to delete the task." << std::endl;
                return 1;
            }
        }
        else {
            std::cerr << "Failed to delete the file." << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << "Invalid command." << std::endl;
        return 1;
    }
}
