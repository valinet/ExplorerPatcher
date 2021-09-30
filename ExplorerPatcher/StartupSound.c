#include "StartupSound.h"

DWORD PlayStartupSound(PlayStartupSoundParams* unused)
{
    Sleep(2000);
    printf("Started \"Play startup sound\" thread.\n");

    HRESULT hr = CoInitialize(NULL);

    // this checks Software\\Microsoft\\Windows\\CurrentVersion\\Authentication\\LogonUI\\LogonSoundPlayed
    // and then plays the startup sound

    AuthUILogonSound* ppv;
    hr = CoCreateInstance(
        &__uuidof_AuthUILogonSound,
        NULL,
        CLSCTX_INPROC_SERVER,
        &__uuidof_IAuthUILogonSound,
        &ppv
    );
    if (SUCCEEDED(hr))
    {
        ppv->lpVtbl->PlayIfNecessary(ppv, 1);
        ppv->lpVtbl->Release(ppv);
    }

    printf("Ended \"Play startup sound\" thread.\n");
    return 0;
}