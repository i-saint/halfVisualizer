#pragma once
#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _CRT_SECURE_NO_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

#include <vsdebugeng.h>
#include <vsdebugeng.templates.h>

#include <cstdint>

using namespace ATL;
using namespace Microsoft::VisualStudio::Debugger;
using namespace Microsoft::VisualStudio::Debugger::Evaluation;

inline _Ret_range_(0x8000000, 0xffffffff) HRESULT WIN32_ERROR(LONG lError)
{
    HRESULT hr = HRESULT_FROM_WIN32(lError);
    if (SUCCEEDED(hr))
        hr = E_FAIL;
    return hr;
}

inline _Ret_range_(0x8000000, 0xffffffff) HRESULT WIN32_LAST_ERROR()
{
    return WIN32_ERROR(GetLastError());
}
