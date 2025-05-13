#pragma once

#include <Windows.h>

inline HRESULT ResultFromWin32(__in DWORD dwErr)
{
    return HRESULT_FROM_WIN32(dwErr);
}

inline HRESULT ResultFromLastError()
{
    return ResultFromWin32(GetLastError());
}

inline HRESULT ResultFromKnownLastError()
{
    HRESULT hr = ResultFromLastError();
    return (SUCCEEDED(hr) ? E_FAIL : hr);
}

inline HRESULT ResultFromWin32Bool(BOOL b)
{
    return b ? S_OK : ResultFromKnownLastError();
}

inline HRESULT ResultFromWin32Count(UINT cchResult, UINT cchBuffer)
{
    return cchResult && cchResult <= cchBuffer ? S_OK : ResultFromWin32(ERROR_INSUFFICIENT_BUFFER);
}
