#ifndef _H_STARTUPSOUND_H_
#define _H_STARTUPSOUND_H_
#include <initguid.h>
#include <Windows.h>

DEFINE_GUID(__uuidof_AuthUILogonSound,
    0x0A0D018EE,
    0x1100, 0x4389, 0xAB, 0x44,
    0x46, 0x4F, 0xAF, 0x00, 0x12, 0x88
);

#ifdef __cplusplus
enum LOGON_SOUND_CLIENT
{
    LSC_LOGONUI,
    LSC_EXPLORER,
};

MIDL_INTERFACE("c35243ea-4cfc-435a-91c2-9dbdecbffc95")
IAuthUILogonSound : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE PlayIfNecessary(LOGON_SOUND_CLIENT client) = 0;
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

BOOL AreLogonLogoffShutdownSoundsEnabled();
HRESULT HookLogonSound();
BOOL InitSoundWindow();
void TermSoundWindow();
HRESULT SHPlaySound(LPCWSTR pszSound, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif