#pragma once

#include <shellapi.h>
#include <string>
#include <vector>

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
    std::wstring PathToLogger;
    std::vector<std::wstring> PathsToResources;
};
