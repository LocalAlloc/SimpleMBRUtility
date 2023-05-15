#include <Windows.h>
#include <stdio.h>

#define YELLOW_COLOR 14
#define WHITE_COLOR 15
#define RED_COLOR 12
#define DARK_RED_COLOR 4

bool isValidMBR(unsigned char* mbr) {
    return (mbr[0x1FE] == 0x55 && mbr[0x1FF] == 0xAA);
}

void print_usage() {
    printf("Usage: <file>.exe [-w <bin_file> <drive_name>]\n");
    printf("System Can't File Specified\n");
    exit(1);
}
void fixwindowsinstallation() {
    int result;

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
}

int main(int argc, char** argv)
{
    HANDLE hDrive = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (hDrive == INVALID_HANDLE_VALUE) {
        printf("Error opening the physical drive: %d\n", GetLastError());
        return 1;
    }

    unsigned char mbr[512];
    DWORD bytesRead;

    if (!ReadFile(hDrive, mbr, sizeof(mbr), &bytesRead, NULL)) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DARK_RED_COLOR);
        printf("Error reading the physical drive: %d\n", GetLastError());
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
        CloseHandle(hDrive);
        return 1;
    }
    if (!isValidMBR(mbr)) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
        printf("Invalid MBR found!!!, Use switches -w or -fix to fix the MBR\n\n");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
    }

    //CloseHandle(hDrive);

    /*if (argc == 4 && strcmp(argv[1], "-w") == 0) {
        char* bin_filename = argv[2];
        char* drive_name = argv[3];
        HANDLE hDrive = CreateFileA(drive_name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDrive == INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED_COLOR);
            printf("Error opening drive: %s. Error code: %d\n", drive_name, GetLastError());
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
            return 1;
        }

        FILE* bin_file = fopen(bin_filename, "rb");
        if (bin_file == NULL) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
            printf("Error opening bin file: %s. Error code: %d\n", bin_filename, GetLastError());
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
            CloseHandle(hDrive);
            return 1;
        }

        unsigned char* boot_loader = (unsigned char*)malloc(sizeof(mbr));
        fread(boot_loader, sizeof(mbr), 1, bin_file);
        fclose(bin_file);

        DWORD bytesWritten;
        if (!WriteFile(hDrive, boot_loader, sizeof(mbr), &bytesWritten, NULL)) {
            printf("Error writing to drive. Error code: %d\n", GetLastError());
            CloseHandle(hDrive);
            return 1;
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
        printf("Bootloader successfully written to %s\n", drive_name);
        printf("Bytes written: %d\n", bytesWritten);
        printf("Result: %s\n", bytesWritten == sizeof(mbr) ? "(TRUE)" : "(FALSE)");
        CloseHandle(hDrive);
        free(boot_loader);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
        return 0;
    }*/
    if (argc == 3 && strcmp(argv[2], "-fix") == 0) {
        printf("FIXING....");
        fixwindowsinstallation();
        printf("MIGHT work if you're lucky, but you ought to use the -w switch \n\n USAGE : <programname>.exe -w <bin_file> <drive_name> example : \\\\.\\PhysicalDrive0");
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "-b") == 0) {
        char* backup_filename = argv[2];
        HANDLE backup_file = CreateFileA(backup_filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (backup_file == INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED_COLOR);
            printf("Error opening backup file: %s. Error code: %d\n", backup_filename, GetLastError());
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
            return 1;
        }

        DWORD bytesWritten;
        if (!WriteFile(backup_file, mbr, sizeof(mbr), &bytesWritten, NULL)) {
            printf("Error writing to backup file. Error code: %d\n", GetLastError());
            CloseHandle(backup_file);
            return 1;
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
        printf("MBR successfully backed up to %s\n", backup_filename);
        printf("Bytes written: %d\n", bytesWritten);
        printf("Result: %s\n", bytesWritten == sizeof(mbr) ? "(TRUE)" : "(FALSE)");
        CloseHandle(backup_file);
        return 0;
    }

    if (argc == 4 && strcmp(argv[1], "-w") == 0) {
        char* bin_filename = argv[2];
        char* drive_name = argv[3];
        HANDLE hDrive1 = CreateFileA(drive_name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDrive1 == INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED_COLOR);
            printf("Error opening drive: %s. Error code: %d\n", drive_name, GetLastError());
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
            return 1;
        }

        FILE* bin_file = fopen(bin_filename, "rb");
        if (bin_file == NULL) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
            printf("Error opening bin file: %s. Error code: %d\n", bin_filename, GetLastError());
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
            CloseHandle(hDrive1);
            return 1;
        }

        unsigned char* boot_loader = (unsigned char*)malloc(sizeof(mbr));
        fread(boot_loader, sizeof(mbr), 1, bin_file);
        fclose(bin_file);

        DWORD bytesWritten;
        if (!WriteFile(hDrive1, boot_loader, sizeof(mbr), &bytesWritten, NULL)) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED_COLOR);
            printf("Error writing to drive. Error code: %d\n", GetLastError());
            CloseHandle(hDrive1);
            free(boot_loader);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
            return 1;
        }

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
        printf("Bootloader successfully written to %s\n", drive_name);
        printf("Bytes written: %d\n", bytesWritten);
        printf("Result: %s\n", bytesWritten == sizeof(mbr) ? "(TRUE)" : "(FALSE)");
        CloseHandle(hDrive1);
        return 0;
    }

    // Set console output color to yellow
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);

    printf("Bootloader data in \"\\\\.\\PhysicalDrive0\"\n\n");
    // Print the byte header
    printf("Byte:\n");
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("Column ");
    for (int i = 1; i <= 8; i++) {
        printf("%02d ", i);
    }
    for (int i = 9; i <= 9; i++) {
        printf(" %02d ", i);
    }
    for (int i = 10; i <= 16; i++) {
        printf("%02d ", i);
    }
    printf("        Data        \n");
    //printf("      ");
    //for (int i = 0; i < 16; i++) {
    //    printf("%02X ", i);
    //    if (i == 7) printf(" ");
    //}
    //printf("| ");
    //for (int i = 0; i < 16; i++) {
    //    printf("%02X ", i + 16);
    //}
    //printf("| ");
    //for (int i = 0; i < 16; i++) {
    //    printf("%02X ", i + 32);
    //    if (i == 7) printf(" ");
    //}
    //printf("| ");
    //for (int i = 0; i < 16; i++) {
    //    printf("%02X ", i + 48);
    //}
    //printf("| ");
    //for (int i = 0; i < 16; i++) {
    //    printf("%02X ", i + 64);
    //    if (i == 7) printf(" ");
    //}
    //printf("| ");
    //for (int i = 0; i < 16; i++) {
    //    printf("%02X ", i + 80);
    //}

    // Print the MBR
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
    for (int i = 0; i < sizeof(mbr); i += 16) {
        printf("0x%03X:", i);
        for (int j = 0; j < 16; j++) {
            if (i + j >= sizeof(mbr)) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
                printf("   ");
            }
            else {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
                printf(" %02X", mbr[i + j]);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED_COLOR);
            }
            if (j == 7) printf(" ");
        }
        printf(" | ");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED_COLOR);
        for (int j = 0; j < 16; j++) {
            if (i + j >= sizeof(mbr)) {
                printf(" ");
            }
            else {
                if (mbr[i + j] < 32 || mbr[i + j] > 126) {
                    printf(".");
                }
                else {
                    printf("%c", mbr[i + j]);
                }
            }
        }
        printf("\n");
    }
    //print ANSI value
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
    printf("\nANSI:\n");
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
    for (int i = 0; i < 512; i += 32) {
        printf(" 0x%03X: ", i);
        for (int j = 0; j < 32; j++) {
            int idx = i + j;
            if (isprint(mbr[idx])) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
                printf("%c", mbr[idx]);
            }
            else {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED_COLOR);
                printf(".");
            }
        }
        printf("\n");
        if (i == 256) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED_COLOR);
            printf("\n");
        }
    }

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW_COLOR);
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("\nRead File Returned: 1 (TRUE)\n");
    printf("Bytes Read : 512 Bytes\n");

    CloseHandle(hDrive);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE_COLOR);
    return 0;
}