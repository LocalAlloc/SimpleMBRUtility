#include <windows.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <taskschd.h>
#include <comutil.h>
#include <PowrProf.h>
#include <CommDlg.h>
#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <Shlobj.h>
#include <fstream>
#include "resource.h"
#pragma comment(lib, "Taskschd.lib")
#pragma comment(lib, "Comsuppw.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")
#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "shlwapi.lib")
using namespace std;
void ProtectPartition();
//void runtask();

#define BOOT_SECTOR_SIZE 512
char backupPath[MAX_PATH] = "C:\\Windows\\System32\\MBRBackup.bin";
void logtofile(const char* message) {
    FILE* file = fopen("C:\\Windows\\log.txt", "a");
    if (file == NULL) {
        printf("Failed to open log file.\n");
        return;
    }

    fprintf(file, "%s\n", message);
    fclose(file);
}
BOOL IsWindows64Bit()
{
#if defined(_WIN64)
    return TRUE;  // 64-bit program running on 64-bit Windows
#elif defined(_WIN32)
    // 32-bit program running on 64-bit Windows?
    BOOL isWow64 = FALSE;
    typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    if (fnIsWow64Process != NULL)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &isWow64))
        {
            return FALSE;  // function call failed
        }
        else if (isWow64)
        {
            return TRUE;  // 32-bit program running on 64-bit Windows
        }
    }
    return FALSE;  // 32-bit program running on 32-bit Windows
#else
    return FALSE;  // not on Windows
#endif
}

//BOOL IsElevated() {
//    BOOL isAdmin = FALSE;
//    PSID adminSID = NULL;
//    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
//
//    if (AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminSID))
//    {
//        if (!CheckTokenMembership(NULL, adminSID, &isAdmin))
//        {
//            isAdmin = FALSE;
//        }
//
//        FreeSid(adminSID);
//    }
//
//    return isAdmin;
//}
bool isValidMBR(unsigned char* mbr) {
    return (mbr[0x1FE] == 0x55 && mbr[0x1FF] == 0xAA);
}



bool backupMBR(char* path) {
    HANDLE hDisk = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        printf("Failed to open disk. Error: %lu\n", GetLastError());
        ExitProcess(-1);
    }

    unsigned char mbr[512];
    DWORD bytesRead;
    if (!ReadFile(hDisk, mbr, sizeof(mbr), &bytesRead, NULL)) {
        DWORD errorMessageId = GetLastError();
        LPVOID errorMessageBuffer = NULL;
        DWORD bufferSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorMessageId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorMessageBuffer,
            0,
            NULL);
        MessageBox(NULL, (LPCWSTR)errorMessageBuffer, L"Error reading MBR", MB_ICONERROR | MB_OK);
        LocalFree(errorMessageBuffer);
        CloseHandle(hDisk);
        return -1;
    }

    if (isValidMBR(mbr)) {
        FILE* fp;
        fp = fopen(path, "wb");
        if (fp == NULL) {
            DWORD errorMessageId = GetLastError();
            LPVOID errorMessageBuffer = NULL;
            DWORD bufferSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorMessageId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&errorMessageBuffer,
                0,
                NULL);
            MessageBox(NULL, (LPCWSTR)errorMessageBuffer, L"Failed to create backup file.", MB_ICONERROR | MB_OK);
            LocalFree(errorMessageBuffer);
            CloseHandle(hDisk);
            return -1;
        }

        fwrite(mbr, 1, sizeof(mbr), fp);
        fclose(fp);
        MessageBoxA(NULL, "MBR BACKED UP SUCCESSFULLY! ", "INFORMATION", MB_ICONINFORMATION | MB_OK);
    }
    else {
        MessageBoxA(NULL, "INVALID MBR DUDE!", "INFORMATION", MB_ICONINFORMATION | MB_OK);
    }

    CloseHandle(hDisk);
    return 0;
}
//void checkFile(const char* filePath) {
//    DWORD fileAttributes = GetFileAttributesA(filePath);
//    if (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
//
//        // Perform the action when the file exists
//        // TODO: code here
//        if (IsElevated) {
//            logtofile("ELEVATED SUCCESSFULLY!\n");
//            while (true) {
//                ProtectPartition();
//                Sleep(10);
//            }
//        }
//        else {
//            logtofile("NOT ELEVATED, ELEVATING\n");
//            runtask();
//            PostQuitMessage(0);
//        }
//    }
//    else {
//
//        // Perform the action when the file does not exist
//        // TODO: Add your code here
//        logtofile("FILE NOT FOUND\n");
//    }
//}

BOOL FileExists(const char* filePath) {
    DWORD fileAttributes = GetFileAttributesA(filePath);
    return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

//int setTask() {
//    HRESULT hr1 = CoInitialize(NULL);
//    if (FAILED(hr1)) {
//        ExitProcess(-1);
//    }
//    ITaskService* pService = NULL;
//    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
//    if (FAILED(hr)) {
//        ExitProcess(1);
//    }
//    //_variant_t(), _variant_t(), _variant_t(), _variant_t()
//    const VARIANT variant{ {{VT_NULL, 0}} };
//    hr = pService->Connect(variant, variant, variant, variant);
//    if (FAILED(hr)) {
//        pService->Release();
//        ExitProcess(1);
//    }
//
//    ITaskFolder* pRootFolder = NULL;
//    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
//    if (FAILED(hr)) {
//        pService->Release();
//        ExitProcess(1);
//    }
//
//    ITaskDefinition* pTask = NULL;
//    hr = pService->NewTask(0, &pTask);
//    if (FAILED(hr)) {
//        pRootFolder->Release();
//        pService->Release();
//        ExitProcess(1);
//    }
//
//    IRegistrationInfo* pRegInfo = NULL;
//    hr = pTask->get_RegistrationInfo(&pRegInfo);
//    if (FAILED(hr)) {
//        pTask->Release();
//        pRootFolder->Release();
//        pService->Release();
//        ExitProcess(1);
//    }
//
//    hr = pRegInfo->put_Author(_bstr_t(L"LocalAlloc"));
//    if (FAILED(hr)) {
//        pRegInfo->Release();
//        pTask->Release();
//        pRootFolder->Release();
//        pService->Release();
//        ExitProcess(1);
//    }
//
//    IActionCollection* pActionCollection = NULL;
//    hr = pTask->get_Actions(&pActionCollection);
//    if (FAILED(hr)) {
//        pRegInfo->Release();
//        pTask->Release();
//        pRootFolder->Release();
//        pService->Release();
//        ExitProcess(1);
//    }
//
//    IAction* pAction = NULL;
//    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
//    if (FAILED(hr)) {
//        pActionCollection->Release();
//        pRegInfo->Release();
//        pTask->Release();
//        pRootFolder->Release();
//        pService->Release();
//        ExitProcess(1);
//    }
//
//    IExecAction* pExecAction = NULL;
//    hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
//    if (FAILED(hr)) {
//        pAction->Release();
//        pActionCollection->Release();
//        pRegInfo->Release();
//        pTask->Release();
//        pRootFolder->Release();
//        pService->Release();
//        ExitProcess(1);
//    }
//    hr = pExecAction->put_Path(_bstr_t(L"C:\\Windows\\System32\\MBRHELPER.exe"));
//    //hr = pTask->put_ExecAction(pAction);
//    hr = pTask->get_Actions(&pActionCollection);
//
//    // Set the task to run on system startup
//    ITaskSettings* pSettings = NULL;
//    hr = pTask->get_Settings(&pSettings);
//
//    if (FAILED(hr))
//    {
//        pRootFolder->Release();
//        pTask->Release();
//        CoUninitialize();
//        return -1;
//    }
//
//    // Disable all power options.
//    hr = pSettings->put_AllowDemandStart(VARIANT_TRUE);
//    if (FAILED(hr))
//    {
//        pRootFolder->Release();
//        pTask->Release();
//        CoUninitialize();
//        return -1;
//    }
//    hr = pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
//    if (FAILED(hr))
//    {
//        pRootFolder->Release();
//        pTask->Release();
//        CoUninitialize();
//        return -1;
//    }
//    hr = pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
//    if (FAILED(hr))
//    {
//        pRootFolder->Release();
//        pTask->Release();
//        CoUninitialize();
//        return -1;
//    }
//    hr = pSettings->put_StartWhenAvailable(VARIANT_FALSE);
//    if (FAILED(hr))
//    {
//        pRootFolder->Release();
//        pTask->Release();
//        CoUninitialize();
//        return -1;
//    }
//    hr = pSettings->put_RunOnlyIfIdle(VARIANT_FALSE);
//    if (FAILED(hr))
//    {
//        pRootFolder->Release();
//        pTask->Release();
//        CoUninitialize();
//        return -1;
//    }
//    //IPrincipal* pPrincipal = NULL;
//
//    //hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
//    //if (FAILED(hr))
//    //{
//    //    printf("failed highest level thingy");
//    //    pRootFolder->Release();
//    //    pTask->Release();
//    //    CoUninitialize();
//    //    return -1;
//    //}
//    IPrincipal* pPrincipal = NULL;
//    hr = pTask->get_Principal(&pPrincipal);
//
//    if (SUCCEEDED(hr) && pPrincipal != NULL) {
//        hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
//    }
//    else {
//        PostQuitMessage(0);
//    }
//
//    //VARIANT_BOOL vTrue;
//    VARIANT_BOOL vTrue = VARIANT_TRUE;
//    _variant_t varBool(vTrue);
//    vTrue = VARIANT_TRUE;
//    varBool = vTrue;
//    //vTrue.boolVal = TRUE;
//    //vTrue.vt = VT_BOOL;
//    hr = pSettings->put_StartWhenAvailable(vTrue);
//    if (FAILED(hr))
//    {
//        pSettings->Release();
//        pRootFolder->Release();
//        pTask->Release();
//        CoUninitialize();
//        return -1;
//    }
//    // Register the task
//    IRegisteredTask* pRegisteredTask = NULL;
//    hr = pRootFolder->RegisterTaskDefinition(
//        _bstr_t(L"MBR Utility"),  // Task name
//        pTask,  // Task definition
//        TASK_CREATE_OR_UPDATE,  // Create or update the task
//        _variant_t(),  // No user account information
//        _variant_t(),  // No password information
//        TASK_LOGON_INTERACTIVE_TOKEN,  // Run the task with the interactive user token
//        _variant_t(L""),  // No sddl security descriptor information
//        &pRegisteredTask  // Task registration
//    );
//    if (FAILED(hr))
//    {
//        pSettings->Release();
//        pRootFolder->Release();
//        pTask->Release();
//        CoUninitialize();
//        return -1;
//    }
//    pSettings->Release();
//    pRootFolder->Release();
//    pTask->Release();
//    CoUninitialize();
//}
void test()
{
    OPENFILENAME ofn;
    wchar_t szFileName[MAX_PATH] = {};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"Binary Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"bin";

    if (GetOpenFileName(&ofn)) {

        // Open the binary file
        std::ifstream file(szFileName, std::ios::binary);
        if (!file) {
            MessageBoxA(NULL, "Error unable to open binary file!", "ERROR", MB_ICONHAND || MB_OK);
            ExitProcess(-1);
        }

        // Get the size of the binary file
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Read the binary file into a buffer
        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size)) {
            MessageBoxA(NULL, "Error Unable to read the binary file", "ERROR", MB_ICONHAND || MB_OK);
            ExitProcess(-1);
        }

        // Open the physical drive for writing
        HANDLE hDrive = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDrive == INVALID_HANDLE_VALUE) {
            MessageBoxA(NULL, "Error opening handle to \\\\.\\PhysicalDrive0", "ERROR", MB_ICONHAND || MB_OK);
            ExitProcess(-1);
        }

        // Write the buffer to the boot sector of the physical drive
        DWORD bytesWritten;
        if (!WriteFile(hDrive, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesWritten, NULL)) {
            MessageBoxA(NULL, "Error not able to write to \\\\.\\PhysicalDrive0", "ERROR", MB_ICONHAND || MB_OK);
            ExitProcess(-1);
        }

        // Close the physical drive
        CloseHandle(hDrive);

        MessageBoxA(NULL, "Bootloader backup has been recovered!", "INFORMATION", MB_OK | MB_ICONINFORMATION);
    }

    PostQuitMessage(0);
}
bool ExtractResource(int iId, LPCWSTR pDest) {
    HRSRC aResourceH = FindResource(NULL, MAKEINTRESOURCE(iId), L"EXE");
    if (!aResourceH) {
        MessageBoxA(NULL, "Unable to find resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    HGLOBAL aResourceHGlobal = LoadResource(NULL, aResourceH);
    if (!aResourceHGlobal) {
        MessageBoxA(NULL, "Unable to load resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned char* aFilePtr = (unsigned char*)LockResource(aResourceHGlobal);
    if (!aFilePtr) {
        MessageBoxA(NULL, "Unable to lock resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned long aFileSize = SizeofResource(NULL, aResourceH);

    HANDLE file_handle = CreateFile(pDest, FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
        int err = GetLastError();
        if ((ERROR_ALREADY_EXISTS == err) || (32 == err)) {
            return true;
        }
        return false;
    }

    unsigned long numWritten;
    WriteFile(file_handle, aFilePtr, aFileSize, &numWritten, NULL);
    CloseHandle(file_handle);

    return true;
}
void fixwindowsinstallation() {
    if (MessageBoxA(NULL, "Do You Have A Boot Loader Backup?", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        test();
    }
    else {
        int result;

        system("bootrec /fixmbr");
        Sleep(3000);
        system("bootrec /fixboot");
        system("bootrec /rebuildbcd");
        // Fix the MBR
        result = system("dism /image:C:\\ /cleanup-image /revertpendingactions");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to Fix MBR", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }

        // Fix the boot sector
        result = system("bootsect /nt60 all /force");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to Fix boot sector", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }
        result = system("REG ADD \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\CrashControl\" /v \"AutoReboot\" /t REG_DWORD /d 0 /f");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to add registry key to disable AutoReboot", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }


        result = system("del /F /Q C:\\boot\\bcd");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to delete OLD bcd", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }

        result = system("bcdedit /createstore C:\\boot\\bcd");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to Create NEW bcd store", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }

        result = system("bcdedit /store C:\\boot\\bcd /import C:\\Windows\\Boot\\BCD");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to Import Boot Configuration data", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }
        MessageBoxA(NULL, "Applied Potential Fixes Possible, If It Doesn't Work \n Please Run MBR RECOVERY TOOL PROVIDED IN WINDOWS! ", "INFORMATION", MB_ICONINFORMATION || MB_OK);
    }
    PostQuitMessage(0);
}
bool restoreMBR(char* path) {
    HANDLE hDisk = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        char errorMessage[256];
        DWORD error = GetLastError();
        sprintf_s(errorMessage, "Failed to open disk. Error: %lu", error);
        MessageBoxA(NULL, errorMessage, "Error", MB_ICONERROR | MB_OK);
        return false;
    }

    FILE* fp;
    fp = fopen(path, "rb");
    if (fp == NULL) {
        MessageBoxA(NULL, "Failed to open backup file.", "Error", MB_ICONERROR | MB_OK);
        CloseHandle(hDisk);
        return false;
    }

    unsigned char mbr[512];
    size_t bytesRead = 0;
    bytesRead = fread(mbr, 1, sizeof(mbr), fp);
    if (bytesRead != sizeof(mbr)) {
        MessageBoxA(NULL, "Failed to read backup file.", "Error", MB_ICONERROR | MB_OK);
        fclose(fp);
        CloseHandle(hDisk);
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hDisk, mbr, sizeof(mbr), &bytesWritten, NULL)) {
        char errorMessage[256];
        DWORD error = GetLastError();
        sprintf_s(errorMessage, "Failed to write MBR. Error: %lu", error);
        MessageBoxA(NULL, errorMessage, "Error", MB_ICONERROR | MB_OK);
        fclose(fp);
        CloseHandle(hDisk);
        return false;
    }

    fclose(fp);
    MessageBoxA(NULL, "MBR restored successfully.", "Success", MB_ICONINFORMATION | MB_OK);

    CloseHandle(hDisk);
    return true;
}
// Function to restore the MBR from the backup file
bool RestoreMBRFromBackup()
{
    HANDLE hDrive = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDrive == INVALID_HANDLE_VALUE)
    {
        MessageBoxA(NULL, "Error Invalid DRIVE HANDLE!", "", MB_OK || MB_ICONHAND);
        return false;
    }

    BYTE buffer[BOOT_SECTOR_SIZE];
    DWORD bytesRead;

    // Read the MBR from the backup file
    ifstream file("C:\\Windows\\System32\\MBRBackup.bin", ios::in | ios::binary);
    if (!file.read(reinterpret_cast<char*>(buffer), BOOT_SECTOR_SIZE))
    {
        CloseHandle(hDrive);
        file.close();
        return false;
    }

    // Write the MBR to the drive
    if (!WriteFile(hDrive, buffer, BOOT_SECTOR_SIZE, &bytesRead, NULL))
    {
        MessageBoxA(NULL, "Error Can't Write backup file to MBR", "", MB_OK | MB_ICONHAND);
        CloseHandle(hDrive);
        file.close();
        ExitProcess(1);
    }
    MessageBoxA(NULL, "SUCCESS", "", MB_OK | MB_ICONINFORMATION);
    CloseHandle(hDrive);
    file.close();
    return 0;
}

// Function to check if the MBR has been modified
//SysWOW64
void ProtectPartition()
{
    // Read the MBR into a buffer
    HANDLE hPhysicalDisk = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hPhysicalDisk == INVALID_HANDLE_VALUE)
    {
        return;
    }

    BYTE buffer[BOOT_SECTOR_SIZE];
    DWORD bytesRead;
    if (!ReadFile(hPhysicalDisk, buffer, BOOT_SECTOR_SIZE, &bytesRead, NULL))
    {
        CloseHandle(hPhysicalDisk);
        return;
    }

    // Close the physical disk handle
    CloseHandle(hPhysicalDisk);

    // Read the backup file into a buffer
    std::ifstream backupFile("C:\\Windows\\System32\\MBRBackup.bin", std::ios::binary);
    BYTE backupBuffer[BOOT_SECTOR_SIZE];
    backupFile.read(reinterpret_cast<char*>(backupBuffer), BOOT_SECTOR_SIZE);

    // Compare the current MBR with the backup buffer
    if (memcmp(buffer, backupBuffer, BOOT_SECTOR_SIZE) != 0)
    {
        // Restore the backup
        MessageBoxA(NULL, "CORRUPT MBR FOUND RECOVERING!", "", MB_OK | MB_ICONHAND);
        RestoreMBRFromBackup();
    }
}
void ProtectPartition64()
{
    // Read the MBR into a buffer
    HANDLE hPhysicalDisk = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hPhysicalDisk == INVALID_HANDLE_VALUE)
    {
        return;
    }

    BYTE buffer[BOOT_SECTOR_SIZE];
    DWORD bytesRead;
    if (!ReadFile(hPhysicalDisk, buffer, BOOT_SECTOR_SIZE, &bytesRead, NULL))
    {
        CloseHandle(hPhysicalDisk);
        return;
    }

    // Close the physical disk handle
    CloseHandle(hPhysicalDisk);

    // Read the backup file into a buffer
    std::ifstream backupFile("C:\\Windows\\SysWOW64\\MBRBackup.bin", std::ios::binary);
    BYTE backupBuffer[BOOT_SECTOR_SIZE];
    backupFile.read(reinterpret_cast<char*>(backupBuffer), BOOT_SECTOR_SIZE);

    // Compare the current MBR with the backup buffer
    if (memcmp(buffer, backupBuffer, BOOT_SECTOR_SIZE) != 0)
    {
        // Restore the backup
        MessageBoxA(NULL, "CORRUPT MBR FOUND RECOVERING!", "", MB_OK | MB_ICONHAND);
        RestoreMBRFromBackup();
    }
}
int setTask() {
    HRESULT hr1 = CoInitialize(NULL);
    if (FAILED(hr1)) {
        ExitProcess(-1);
    }

    ITaskService* pService = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        ExitProcess(1);
    }

    const VARIANT variant{ {{VT_NULL, 0}} };
    hr = pService->Connect(variant, variant, variant, variant);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskDefinition* pTask = NULL;
    hr = pService->NewTask(0, &pTask);
    if (FAILED(hr)) {
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IRegistrationInfo* pRegInfo = NULL;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pRegInfo->put_Author(_bstr_t(L"LocalAlloc"));
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IActionCollection* pActionCollection = NULL;
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IAction* pAction = NULL;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    if (FAILED(hr)) {
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IExecAction* pExecAction = NULL;
    hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
    if (FAILED(hr)) {
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pExecAction->put_Path(_bstr_t(L"C:\\Windows\\System32\\MBRHELPER.exe"));

    // Set the task to run on user logon
    ITriggerCollection* pTriggerCollection = NULL;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ITrigger* pTrigger = NULL;
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ILogonTrigger* pLogonTrigger = NULL;
    hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger);
    if (FAILED(hr)) {
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    //hr = pTriggerCollection->Create(pTrigger);
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pTriggerCollection->Release();
    pLogonTrigger->Release();
    pTrigger->Release();

    // Set the task to run with highest privileges
    IPrincipal* pPrincipal = NULL;
    hr = pTask->get_Principal(&pPrincipal);
    if (SUCCEEDED(hr) && pPrincipal != NULL) {
        hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        pPrincipal->Release();
    }
    else {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    // Register the task
    IRegisteredTask* pRegisteredTask = NULL;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(L"MBR Utility"),  // Task name
        pTask,  // Task definition
        TASK_CREATE_OR_UPDATE,  // Create or update the task
        _variant_t(),  // No user account information
        _variant_t(),  // No password information
        TASK_LOGON_INTERACTIVE_TOKEN,  // Run the task with the interactive user token
        _variant_t(L""),  // No sddl security descriptor information
        &pRegisteredTask  // Task registration
    );
        if (FAILED(hr)) {
            pTask->Release();
            pRootFolder->Release();
            pService->Release();
            ExitProcess(1);
        }

    pRegisteredTask->Release();
    pTask->Release();
    pRootFolder->Release();
    pService->Release();

    CoUninitialize();
}
int setTask64() {
    HRESULT hr1 = CoInitialize(NULL);
    if (FAILED(hr1)) {
        ExitProcess(-1);
    }

    ITaskService* pService = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        ExitProcess(1);
    }

    const VARIANT variant{ {{VT_NULL, 0}} };
    hr = pService->Connect(variant, variant, variant, variant);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskDefinition* pTask = NULL;
    hr = pService->NewTask(0, &pTask);
    if (FAILED(hr)) {
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IRegistrationInfo* pRegInfo = NULL;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pRegInfo->put_Author(_bstr_t(L"LocalAlloc"));
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IActionCollection* pActionCollection = NULL;
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IAction* pAction = NULL;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    if (FAILED(hr)) {
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IExecAction* pExecAction = NULL;
    hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
    if (FAILED(hr)) {
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pExecAction->put_Path(_bstr_t(L"C:\\Windows\\SysWOW64\\MBRHELPER.exe"));

    // Set the task to run on user logon
    ITriggerCollection* pTriggerCollection = NULL;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ITrigger* pTrigger = NULL;
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ILogonTrigger* pLogonTrigger = NULL;
    hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger);
    if (FAILED(hr)) {
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    //hr = pTriggerCollection->Create(pTrigger);
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pTriggerCollection->Release();
    pLogonTrigger->Release();
    pTrigger->Release();

    // Set the task to run with highest privileges
    IPrincipal* pPrincipal = NULL;
    hr = pTask->get_Principal(&pPrincipal);
    if (SUCCEEDED(hr) && pPrincipal != NULL) {
        hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        pPrincipal->Release();
    }
    else {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    // Register the task
    IRegisteredTask* pRegisteredTask = NULL;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(L"MBR Utility"),  // Task name
        pTask,  // Task definition
        TASK_CREATE_OR_UPDATE,  // Create or update the task
        _variant_t(),  // No user account information
        _variant_t(),  // No password information
        TASK_LOGON_INTERACTIVE_TOKEN,  // Run the task with the interactive user token
        _variant_t(L""),  // No sddl security descriptor information
        &pRegisteredTask  // Task registration
    );
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pRegisteredTask->Release();
    pTask->Release();
    pRootFolder->Release();
    pService->Release();

    CoUninitialize();
}

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