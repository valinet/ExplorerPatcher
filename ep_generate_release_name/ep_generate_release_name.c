#include "../ExplorerPatcher/queryversion.h"

int main(int argc, char** argv)
{
    SetConsoleOutputCP(CP_UTF8);

    DWORD dwLeftMost = 0;
    DWORD dwSecondLeft = 0;
    DWORD dwSecondRight = 0;
    DWORD dwRightMost = 0;

    QueryVersionInfo(GetModuleHandle(NULL), VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

    printf("%d.%d.%d.%d", dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);

    return 0;
}