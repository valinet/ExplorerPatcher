#include <Windows.h>
#include <stdio.h>
#include "../ExplorerPatcher/queryversion.h"
#define FILE_NAME "CHANGELOG.md"
#define MAX_LINE_LENGTH 200000

int main(int argc, char** argv)
{
    SetConsoleOutputCP(CP_UTF8);

    char szStartPattern[MAX_PATH];
    char szEndPattern[MAX_PATH];

    DWORD dwLeftMost = 0;
    DWORD dwSecondLeft = 0;
    DWORD dwSecondRight = 0;
    DWORD dwRightMost = 0;

    QueryVersionInfo(GetModuleHandle(NULL), VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

    sprintf_s(szStartPattern, MAX_PATH, "## %d.%d.%d\n", dwLeftMost, dwSecondLeft, dwSecondRight);
    sprintf_s(szEndPattern, MAX_PATH, "## ");

    FILE* f = NULL;
    fopen_s(&f, "..\\..\\..\\" FILE_NAME, "r");
    if (!f)
    {
        fopen_s(&f, "..\\..\\" FILE_NAME, "r");
        if (!f)
        {
            fopen_s(&f, "..\\" FILE_NAME, "r");
            if (!f)
            {
                fopen_s(&f, "" FILE_NAME, "r");
            }
        }
    }
    if (f)
    {
        int state = 0;
        size_t bufsiz = MAX_LINE_LENGTH, numChRd = 0;
        char* line = malloc(MAX_LINE_LENGTH * sizeof(char));
        while ((numChRd = getline(&line, &bufsiz, f)) != -1)
        {
            if (state == 0 && !strcmp(line, szStartPattern))
            {
                state = 1;
                numChRd = getline(&line, &bufsiz, f);
                continue;
            }
            else if (state == 1 && !strncmp(line, szEndPattern, strlen(szEndPattern)))
            {
                state = 2;
                break;
            }
            if (state == 1)
            {
                printf("%s", line);
            }
        }
        free(line);
        printf(
            "Please consult the [README](https://github.com/valinet/ExplorerPatcher/blob/master/README.md) for more details.\n"
            "A detailed change log is available [here](https://github.com/valinet/ExplorerPatcher/blob/master/CHANGELOG.md).\n"
            "An archive containing all the files generated during the build process (including `dxgi.dll` and symbol files) is available [here](%s).\n\n"
            "*This release has been published automatically from %s %s in branch [%s](https://github.com/valinet/ExplorerPatcher/tree/%s/).*",
            argc == 4 ? argv[3] : "https://github.com/valinet/ExplorerPatcher/actions",
            argc == 4 ? "commits up to and including" : "the latest commits",
            argc == 4 ? argv[1] : "",
            argc == 4 ? argv[2] : "master",
            argc == 4 ? argv[2] : "master"
        );
        fclose(f);
    }

    return 0;
}
