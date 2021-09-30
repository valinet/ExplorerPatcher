#ifndef _H_STARTUPSOUND_H_
#define _H_STARTUPSOUND_H_
#include <initguid.h>
#include <Windows.h>

DEFINE_GUID(__uuidof_AuthUILogonSound,
    0x0A0D018EE,
    0x1100, 0x4389, 0xAB, 0x44,
    0x46, 0x4F, 0xAF, 0x00, 0x12, 0x88
);
DEFINE_GUID(__uuidof_IAuthUILogonSound,
    0xc35243ea,
    0x4cfc, 0x435a, 0x91, 0xc2,
    0x9d, 0xbd, 0xec, 0xbf, 0xfc, 0x95
);

typedef interface AuthUILogonSound AuthUILogonSound;

typedef struct AuthUILogonSoundVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            AuthUILogonSound* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        AuthUILogonSound* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        AuthUILogonSound* This);

    HRESULT(STDMETHODCALLTYPE* PlayIfNecessary)(
        AuthUILogonSound* This,
        /* [in] */ INT64 a1);

    END_INTERFACE
} AuthUILogonSoundVtbl;

interface AuthUILogonSound
{
    CONST_VTBL struct AuthUILogonSoundVtbl* lpVtbl;
};

typedef DWORD PlayStartupSoundParams;

DWORD PlayStartupSound(PlayStartupSoundParams* unused);
#endif