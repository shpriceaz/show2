// Show.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <map>

boolean mybool = 0;
char gtitle[256];
boolean bInfoOnly = 0;

// Holds info for the app listing feature
struct AppEntry {
    std::string exeName;   // just the filename (e.g. "notepad.exe")
    std::string exePath;   // full path
};

std::vector<AppEntry> gAppList;
std::map<std::string, bool> gSeenPaths; // deduplicate by full path

int PrintStyles(LONG style) {
    if(style & WS_CHILD)
        std::cout << "WS_CHILD" << ",";
    if (style & WS_MINIMIZE)
        std::cout << "WS_MINIMIZE" << ",";
    if (style & WS_VISIBLE)
        std::cout << "WS_VISIBLE" << ",";
    if (style & WS_DISABLED)
        std::cout << "WS_DISABLED" << ",";
    if (style & WS_CLIPSIBLINGS)
        std::cout << "WS_CLIPSIBLINGS" << ",";
    if (style & WS_CLIPCHILDREN)
        std::cout << "WS_CLIPCHILDREN" << ",";
    if (style & WS_MAXIMIZE)
        std::cout << "WS_MAXIMIZE" << ",";
    if (style & WS_CAPTION)
        std::cout << "WS_CAPTION" << ",";
    if (style & WS_BORDER)
        std::cout << "WS_BORDER" << ",";
    if (style & WS_DLGFRAME)
        std::cout << "WS_DLGFRAME" << ",";
    if (style & WS_VSCROLL)
        std::cout << "WS_VSCROLL" << ",";
    if (style & WS_HSCROLL)
        std::cout << "WS_HSCROLL" << ",";
    if (style & WS_SYSMENU)
        std::cout << "WS_SYSMENU" << ",";
    if (style & WS_THICKFRAME)
        std::cout << "WS_THICKFRAME" << ",";
    if (style & WS_GROUP)
        std::cout << "WS_GROUP" << ",";
    if (style & WS_TABSTOP)
        std::cout << "WS_TABSTOP" << ",";
    if (style & WS_MINIMIZEBOX)
        std::cout << "WS_MINIMIZEBOX" << ",";
    if (style & WS_MAXIMIZEBOX)
        std::cout << "WS_MAXIMIZEBOX" << ",";

    std::cout << "DONE" << std::endl;
    return 0;
}

// Callback to collect all unique applications with visible top-level windows
BOOL CALLBACK ListAppsProc(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd))
        return TRUE;

    // Skip windows with no title
    char windowTitle[256];
    if (GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle)) == 0)
        return TRUE;

    // Skip child windows
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (style & WS_CHILD)
        return TRUE;

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProc) {
        char exePath[MAX_PATH];
        if (GetModuleFileNameExA(hProc, NULL, exePath, MAX_PATH)) {
            std::string fullPath(exePath);
            if (gSeenPaths.find(fullPath) == gSeenPaths.end()) {
                gSeenPaths[fullPath] = true;
                // Extract just the filename
                size_t slashPos = fullPath.find_last_of("\\/");
                std::string exeName = (slashPos != std::string::npos)
                    ? fullPath.substr(slashPos + 1)
                    : fullPath;
                AppEntry entry;
                entry.exeName = exeName;
                entry.exeName += "*%$#";
                entry.exePath = fullPath;
                gAppList.push_back(entry);
            }
        }
        CloseHandle(hProc);
    }

    return TRUE;
}

// Callback function for EnumWindows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {

    char className[256];

    GetClassNameA(hwnd, className, sizeof(className));

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProc) {
        char exePath[MAX_PATH];
        if (GetModuleFileNameExA(hProc, NULL, exePath, MAX_PATH)) {
            std::string mytitle(exePath);
            if (mytitle.find(gtitle) != std::string::npos) {
                LONG style = GetWindowLong(hwnd, GWL_STYLE);
                if (style & WS_OVERLAPPEDWINDOW && style & WS_THICKFRAME) {
                    wchar_t titleString[MAX_PATH];
                    int titleout = GetWindowTextW(hwnd, titleString, MAX_PATH);
                    std::cout << gtitle << std::endl;
                    std::wcout << "Window Title: " << titleString << std::endl;
                    std::cout << exePath << std::endl;
                    std::cout << style << std::endl;
                    PrintStyles(style);
                    ShowWindow(hwnd, SW_RESTORE);
                    return TRUE;
                }
            }
        }

        CloseHandle(hProc);
    }

    return TRUE; // Continue enumeration
}

int main(int argc, char *argv[])
{
    // List all applications with open windows on startup
    EnumWindows(ListAppsProc, 0);

    std::cout << "=== Applications with open windows ===" << std::endl;
    for (int i = 0; i < (int)gAppList.size(); i++) {
        std::cout << "[" << (i + 1) << "] " << gAppList[i].exeName << std::endl;
    }
    std::cout << "=======================================" << std::endl;
    std::cout << std::endl;

    if (argc == 1) {
        std::cout << "Enter Application Name or ID number: ";
        char input[256];
        std::cin >> input;

        // Check if the user entered a number
        bool isNumber = true;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit((unsigned char)input[i])) {
                isNumber = false;
                break;
            }
        }

        if (isNumber) {
            int id = atoi(input);
            if (id >= 1 && id <= (int)gAppList.size()) {
                // Use just the exe name (without .exe extension) as the search term
                std::string chosen = gAppList[id - 1].exeName;
                // Strip .exe if present for a friendlier match
                size_t dotPos = chosen.rfind('.');
                if (dotPos != std::string::npos)
                    chosen = chosen.substr(0, dotPos);
                strcpy_s(gtitle, chosen.c_str());
            } else {
                std::cout << "Invalid ID." << std::endl;
                std::cin >> input; // wait before exit
                return 1;
            }
        } else {
            strcpy_s(gtitle, input);
        }
    }
    else {
        strcpy_s(gtitle, argv[1]);
        mybool = TRUE;
    }

    EnumWindows(EnumWindowsProc, 0);
    char done[256];
    std::cin >> done;
    return TRUE;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
