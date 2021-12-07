// src.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "src.h"

#include <shellapi.h>
#include <winhttp.h>
#include <fileapi.h>
#include <string>
#include <vector>

//#pragma comment(lib, "winhttp.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

class CmdArgumentParser
{
public:
    CmdArgumentParser(LPWSTR lpCmdLine)
    {
        if (std::wstring() == lpCmdLine)
        {
            MessageBox(
                NULL,
                (LPCWSTR)L"There is no arguments",
                (LPCWSTR)L"Error reading arguments from command line",
                MB_ICONERROR
            );
            return;
        }

        int numberOfArgs;
        LPWSTR* cmdArgList = CommandLineToArgvW(lpCmdLine, &numberOfArgs);

        if (NULL == cmdArgList)
        {
            MessageBox(
                NULL,
                (LPCWSTR)L"Something went wrong",
                (LPCWSTR)L"Error reading arguments from command line",
                MB_ICONERROR
            );
            return;
        }

        for (int i = 0; i < numberOfArgs; ++i)
        {
            std::wstring const argument(cmdArgList[i]);
            auto const delim = argument.find('=');
            auto const option = argument.substr(0, delim);
            auto const value = argument.substr(delim + 1);

            if (option == L"-p" || option == L"--path")
            {
                PathsToResources.push_back(value);
            }
            else if (option == L"-c" || option == L"--connect")
            {
                // If we place more than one such option the latest will be chosen
                // TODO: Remove slash on the end if it present
                ConnectSiteName = value;
            }
            else if (option == L"-l" || option == L"--pathToLogs")
            {
                // The same behaviour as for ConnectSiteName
                PathToLogger = value;
            }
            else
            {
                // TODO: throw some error, maybe
                MessageBox(
                    NULL,
                    (LPCWSTR)L"Unhandled error while splitting cmd argumnets",
                    (LPCWSTR)L"Error splitting args",
                    MB_ICONERROR
                );
            }
        }

        // Free memory allocated for CommandLineToArgvW arguments.
        LocalFree(cmdArgList);
    }

    std::wstring ConnectSiteName;
    // TODO: Mechanism for
    std::wstring PathToLogger;
    std::vector<std::wstring> PathsToResources;
};

void SendGetRequest(std::wstring connectSiteName, std::wstring pathToResource);
void WriteTextAndCreateFile(LPSTR ptrToBuffer, DWORD sizeOfBuffer);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SRC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SRC));

    // TODO: REMOVE IT
    MessageBox(
        NULL,
        (LPCWSTR)L"I'm here just for debug",
        (LPCWSTR)L"Debug",
        MB_ICONWARNING
    );

    CmdArgumentParser parser(lpCmdLine);

    // TODO: Rewrite this for-loop to download files
    for (auto arg : parser.PathsToResources)
    {
        // TODO: Use progress bar instead
        MessageBox(
            NULL,
            (LPCWSTR)arg.c_str(),
            (LPCWSTR)L"Info",
            MB_ICONINFORMATION
        );
        SendGetRequest(parser.ConnectSiteName, arg);
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}










void SendGetRequest(std::wstring connectSiteName, std::wstring pathToResource)
{
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    // TODO: Make it LPWSTR (char --> wchar_t)s
    LPSTR pszOutBuffer;
    BOOL  bResults = FALSE;
    HINTERNET hSession = NULL,
        hConnect = NULL,
        hRequest = NULL;

    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"DownloadUtil/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect(hSession, /*L"en.wikipedia.org"*/connectSiteName.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);

    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"GET", pathToResource.c_str(),
            NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);
    else
    {
        MessageBox(
            NULL,
            (LPCWSTR)L"WinHttpConnect failed",
            (LPCWSTR)(L"Error #" + std::to_wstring(GetLastError())).c_str(),
            MB_ICONERROR
        );
        return;
    }

    // Send a request.
    if (hRequest)
        bResults = WinHttpSendRequest(hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0, WINHTTP_NO_REQUEST_DATA, 0,
            0, 0);

    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);

    // Keep checking for data until there is nothing left.
    if (bResults)
    {
        do
        {
            // Check for available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
            {
                MessageBox(
                    NULL,
                    (LPCWSTR)L"By WinHttpQueryDataAvailable there is no queriable data",
                    (LPCWSTR)(L"Error #" + std::to_wstring(GetLastError())).c_str(),
                    MB_ICONERROR
                );
                break;
            }

            // No more available data.
            if (!dwSize)
                break;

            // Allocate space for the buffer.
            pszOutBuffer = new char[dwSize];
            if (!pszOutBuffer)
            {
                MessageBox(
                    NULL,
                    (LPCWSTR)L"Can't allocate such amount of memory",
                    (LPCWSTR)L"Out of memory",
                    MB_ICONERROR
                );
                break;
            }

            // Read the Data.
            ZeroMemory(pszOutBuffer, dwSize);

            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
                dwSize, &dwDownloaded))
            {
                MessageBox(
                    NULL,
                    (LPCWSTR)L"Error in WinHttpReadData",
                    (LPCWSTR)(L"Error #" + std::to_wstring(GetLastError())).c_str(),
                    MB_ICONERROR
                );
                break;
            }

            // Write data to file
            WriteTextAndCreateFile(pszOutBuffer, dwSize);

            // Free the memory allocated to the buffer.
            delete[] pszOutBuffer;

            // This condition should never be reached since WinHttpQueryDataAvailable
            // reported that there are bits to read.
            if (!dwDownloaded)
                break;

        } while (dwSize > 0);
    }
    else
    {
        // Report any errors
        MessageBox(
            NULL,
            (LPCWSTR)L"Unhandled error occurred",
            (LPCWSTR)(L"Error #" + std::to_wstring(GetLastError())).c_str(),
            MB_ICONERROR
        );
    }

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
}






void WriteTextAndCreateFile(LPSTR ptrToBuffer, DWORD sizeOfBuffer)
{
    static size_t fileNumber = 0;

    std::string fileName = "downloadedFile" + std::to_string(fileNumber++);

    HANDLE hFile;
    //char DataBuffer[] = "This is some test data to write to the file";
    //DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;

    hFile = CreateFileA(fileName.c_str(),   // name of the write
        GENERIC_WRITE,                      // open for writing
        0,                                  // do not share
        NULL,                               // default security
        CREATE_NEW,                         // create new file only
        FILE_ATTRIBUTE_NORMAL,              // normal file
        NULL);                              // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(
            NULL,
            (LPCWSTR)L"Unable to create file, received INVALID_HANDLE_VALUE",
            (LPCWSTR)L"CreateFile function failed",
            MB_ICONERROR
        );
        return;
    }

    bErrorFlag = WriteFile(
        hFile,              // open file handle
        ptrToBuffer,        // start of data to write
        sizeOfBuffer,       // number of bytes to write
        &dwBytesWritten,    // number of bytes that were written
        NULL);              // no overlapped structure

    if (FALSE == bErrorFlag)
    {
        MessageBox(
            NULL,
            (LPCWSTR)L"Unable to write to file",
            (LPCWSTR)L"WriteFile function failed",
            MB_ICONERROR
        );
    }
    else
    {
        if (dwBytesWritten != sizeOfBuffer)
        {
            // This is an error because a synchronous write that results in
            // success (WriteFile returns TRUE) should write all data as
            // requested. This would not necessarily be the case for
            // asynchronous writes.
            MessageBox(
                NULL,
                (LPCWSTR)L"Written bytes are not equal to buffer provided",
                (LPCWSTR)L"File wasn't loaded properly",
                MB_ICONERROR
            );
        }
    }

    CloseHandle(hFile);
}









//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SRC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SRC);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
