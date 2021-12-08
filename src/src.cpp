#include "src.h"
#include "StuffForWindow.h"
#include "HttpClientRequest.h"

#include <CommCtrl.h>

//HINSTANCE hInst; // Pointer to instance of app
HWND hProgress; // Progress bar descriptor
DWORD IDC_TIMER; // Number of timer
DWORD nCounter;

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hInst = hInstance;

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
        DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, DlgProc);
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

INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        hProgress = CreateWindowEx(
            0,
            PROGRESS_CLASS, // pointer to progress bar
            (LPTSTR)NULL,
            WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
            20, // x
            20, // y
            300, // width
            30, // height
            hWnd,
            (HMENU)0,
            hInst,
            NULL
        );
        // Setting progress bar range
        SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendMessage(hProgress, PBM_SETSTEP, (WPARAM)10, 0);
        // TODO: Replace with calculation of delivery time
        SetTimer(hWnd, IDC_TIMER, 100, 0);
        nCounter = 0;
        
        break;
    }
    case WM_TIMER:
    {
        ++nCounter;
        // Moving the timer
        SendMessage(hProgress, PBM_STEPIT, 0, 0);
        if (nCounter == 10)
        {
            KillTimer(hWnd, IDC_TIMER);
            EndDialog(hWnd, 0);
        }
        break;
    }
    }
    return FALSE;
}
