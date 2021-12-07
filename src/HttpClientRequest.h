#pragma once

#include "CmdArgumentParser.h"
#include "FileInteractions.h"

#include <winhttp.h>

void SendGetRequest(std::wstring connectSiteName, std::wstring pathToResource);

void SendGetRequest(std::wstring connectSiteName, std::wstring pathToResource)
{
    static size_t fileNumber = 0;

    // TODO: Make replacement of slashes with "_"
    std::wstring fileName = L"downloadedFile" + std::to_wstring(fileNumber++);

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
        hConnect = WinHttpConnect(hSession, connectSiteName.c_str(),
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
        HANDLE hFile = CreateNewFile(fileName);

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
            WriteTextChunkToFile(hFile, pszOutBuffer, dwSize);

            // Free the memory allocated to the buffer.
            delete[] pszOutBuffer;

            // This condition should never be reached since WinHttpQueryDataAvailable
            // reported that there are bits to read.
            if (!dwDownloaded)
                break;

        } while (dwSize > 0);

        CloseHandle(hFile);
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
