#pragma once

#include <fileapi.h>
#include <string>

HANDLE CreateNewFile(std::wstring fileName);
void WriteTextChunkToFile(HANDLE hFile, LPSTR ptrToBuffer, DWORD sizeOfBuffer);

HANDLE CreateNewFile(std::wstring fileName)
{
    HANDLE hFile = CreateFileW(fileName.c_str(),    // name of the write
        GENERIC_WRITE,                              // open for writing
        0,                                          // do not share
        NULL,                                       // default security
        CREATE_NEW,                                 // create new file only
        FILE_ATTRIBUTE_NORMAL,                      // normal file
        NULL);                                      // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(
            NULL,
            (LPCWSTR)L"Unable to create file, received INVALID_HANDLE_VALUE",
            (LPCWSTR)L"CreateFile function failed",
            MB_ICONERROR
        );
    }

    return hFile;
}

void WriteTextChunkToFile(HANDLE hFile, LPSTR ptrToBuffer, DWORD sizeOfBuffer)
{
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;

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
    else if (dwBytesWritten != sizeOfBuffer)
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
