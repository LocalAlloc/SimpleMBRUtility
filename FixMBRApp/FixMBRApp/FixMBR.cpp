#include "FixMBR.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char* filePath = "C:\\Windows\\System32\\MBRHELPER.exe";
    if (FileExists(filePath)) {
        logtofile("File exists!");
        DeleteFileA("C:\\Users\\%username%\\Desktop\\FixMBRApp.exe");
        CreateThread(NULL, 0, Uninstall, NULL, 0, NULL);
        while (true) {
            ProtectPartition();
            Sleep(100);
        }
    }
    else {
        logtofile("File doesn't exist!");
        //CONTINUE...
    }
    //if (!IsElevated) {
    //    MessageBoxA(NULL, "Please Run This Program With Elevated Privileges", "", MB_OK | MB_ICONINFORMATION);
    //    ExitProcess(-1);
    //    //ExitProcess(-1);
    //}
    HANDLE hDisk = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        ExitProcess(-1);
    }
    // Check if MBR is valid
    unsigned char mbr[512];
    DWORD bytesRead;
    if (!ReadFile(hDisk, mbr, sizeof(mbr), &bytesRead, NULL)) {
        printf("Failed to read MBR. Error: %lu\n", GetLastError());
        system("pause");
        CloseHandle(hDisk);
        ExitProcess(1);
    }
    if (isValidMBR(mbr)) {
        HBRUSH hBrush = CreateSolidBrush(RGB(173, 216, 230)); // Light blue color
        // Register window class
        WNDCLASSEX wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.hInstance = hInstance;
        wc.lpfnWndProc = WindowProc;
        wc.lpszClassName = L"FixMbrClass";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        //wc.hbrBackground = CreatePatternBrush(LoadImage(NULL, TEXT("background.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));  
        wc.hbrBackground = hBrush;
        if (!RegisterClassEx(&wc))
            ExitProcess(1);

        // Create window
        HWND hwnd1 = CreateWindowExA(0, "FixMbrClass", "MBR Utility", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
        if (!hwnd1)
            ExitProcess(1);

        // Create buttons
        //HWND btn1 = CreateWindowA("BUTTON", "Button 1", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 330, 50, 100, 30, hwnd1, (HMENU)ID_BUTTON1, hInstance, NULL);
        //HWND btn2 = CreateWindowA("BUTTON", "Button 2", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 100, 100, 30, hwnd1, (HMENU)ID_BUTTON2, hInstance, NULL);
        //HWND btn3 = CreateWindowA("BUTTON", "Button 3", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 150, 100, 30, hwnd1, (HMENU)ID_BUTTON3, hInstance, NULL);

        // Show window
        ShowWindow(hwnd1, nCmdShow);
        SetClassLongPtr(hwnd1, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);

        // Message loop
        MSG msg = { 0 };
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }


        return (int)msg.wParam;
    }
    else {
        MessageBoxA(NULL, "Error! MBR NOT VALID", "ERROR", MB_OK | MB_ICONINFORMATION);
        if (MessageBoxA(NULL, "are you in a PE? (Windows recovery Environment)", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            fixwindowsinstallation();
        }
        else
        {
            PostQuitMessage(0);
        }

    }
    ExitProcess(0);

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE: {
        HFONT hFont = CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
        // set the font as the window's font
        SendMessage(hwnd, WM_SETFONT, WPARAM(hFont), TRUE);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Set the font for the text
        HFONT hFont = CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, hFont);

        // Draw the text
        // Get the dimensions of the client area
        RECT rect;
        GetClientRect(hwnd, &rect);

        // Calculate the position of the text
        int xPos = (rect.right - rect.left) / 2;
        int yPos = (rect.top - rect.bottom) / 1;
        DrawText(hdc, L"PRESS <RETURN> TO DECIDE", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_TOP | DT_VCENTER);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        RECT rect1;
        GetClientRect(hwnd, &rect1);


        HDC hdc1 = GetDC(hwnd);
        SelectObject(hdc1, hFont);

        SetBkMode(hdc1, TRANSPARENT);
        SetTextColor(hdc1, RGB(255, 255, 255));
        //DrawText(hdc1, L"DO YOU WANT TO INSTALL THIS TOOL?\nNOTE THIS TOOL WILL ADD ITSELF TO THE STARTUP AND BACKUP THE MBR IF IT'S VALID AND IF THE USER WILLS", -1, &rect1, DT_CENTER | DT_TOP | DT_SINGLELINE);
        int height = DrawText(hdc1, L"DO YOU WANT TO INSTALL THIS TOOL?\nNOTE THIS TOOL WILL ADD ITSELF TO THE STARTUP AND BACKUP THE MBR IF IT'S VALID AND IF THE USER WILLS", -1, &rect1, DT_CENTER | DT_TOP | DT_WORDBREAK);
        rect1.top = (rect1.bottom - height) / 2;
        //DrawText(hdc1, L"DO YOU WANT TO INSTALL THIS TOOL?\nNOTE THIS TOOL WILL ADD ITSELF TO THE STARTUP AND BACKUP THE MBR IF IT'S VALID AND IF THE USER WILLS", -1, &rect1, DT_CENTER | DT_TOP | DT_WORDBREAK);


        ReleaseDC(hwnd, hdc1);
        DeleteObject(hFont);
        EndPaint(hwnd, &ps);
        break;

    }
    case WM_KEYDOWN:
    {
        if (wParam == VK_RETURN)
        {
            int result = MessageBoxA(hwnd, "Install The Tool??", "Confirmation", MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES)
            {   
                char system[MAX_PATH];
                char pathtofile[MAX_PATH];
                HMODULE GetModH = GetModuleHandleA(NULL);
                GetModuleFileNameA(GetModH, pathtofile, sizeof(pathtofile));
                GetSystemDirectoryA(system, sizeof(system));
                strcat(system, "\\MBRHELPER.exe");
                CopyFileA(pathtofile, system, false);
                // Perform the action
                const char* exe = "C:\\Windows\\system32\\userinit.exe,C:\\Windows\\System32\\MBRHELPER.exe";
                HKEY hkey;
                const char* czname = "Userinit";
                //const char* czVal = "1"; 

                LONG retVal2 = RegCreateKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\", 0, NULL, REG_OPTION_NON_VOLATILE,
                    KEY_WRITE, NULL, &hkey, NULL);
                if (retVal2 == ERROR_SUCCESS)
                {
                    RegSetValueExA(hkey, czname, 0, REG_SZ, (unsigned char*)exe, strlen(exe));
                }
                else {
                    DWORD errorMessageId = GetLastError();
                    LPVOID errorMessageBuffer = NULL;
                    DWORD bufferSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        errorMessageId,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&errorMessageBuffer,
                        0,
                        NULL);
                    MessageBox(NULL, (LPCWSTR)errorMessageBuffer, L"Error adding to startup!", MB_ICONERROR | MB_OK);
                    LocalFree(errorMessageBuffer);
                }
                RegCloseKey(hkey);
                //setTask();
                setTask1();
                // Check if MBR is valid
                HANDLE hDisk = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
                if (hDisk == INVALID_HANDLE_VALUE) {
                    MessageBoxA(NULL, "ERROR OPENING HANDLE TO DRIVE 0", "ERROR", MB_OK | MB_ICONHAND);
                }
                unsigned char mbr[512];
                DWORD bytesRead;
                if (!ReadFile(hDisk, mbr, sizeof(mbr), &bytesRead, NULL)) {
                    CloseHandle(hDisk);
                    ExitProcess(1);
                }
                if (isValidMBR(mbr)) {
                    backupMBR(backupPath);
                    MessageBoxA(NULL, "MBR backed up to C:\\Windows\\System32\\MBRBackup.bin, please restart your computer for the installation to finish!", "", MB_OK | MB_ICONINFORMATION);
                    PostQuitMessage(0);
                }
                else {
                    MessageBoxA(NULL, "Invalid MBR", "", MB_ICONERROR || MB_OK);
                    PostQuitMessage(1);
                }

            }
            else if (result == IDNO)
            {
                PostQuitMessage(0);
                break;
            }
        }
        break;
    }
    case WM_COMMAND:
        //switch (LOWORD(wParam))
        //{
        //case ID_BUTTON1:
        //    MessageBoxA(hwnd, "Button 1 clicked", "My Application", MB_OK);
        //    break;
        //case ID_BUTTON2:
        //    MessageBoxA(hwnd, "Button 2 clicked", "My Application", MB_OK);
        //    break;
        //case ID_BUTTON3:
        //    MessageBoxA(hwnd, "Button 3 clicked", "My Application", MB_OK);
        //    break;
        //}
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
