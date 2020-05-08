#include "stdafx.h"
#include "halfVisualizer.h"


#define hvEnableIf(...) std::enable_if_t<__VA_ARGS__, bool> = true

// packed to hex string
template<class T, hvEnableIf(sizeof(T) == 1)>
void to_hexstring(CString& dst, T src)
{
    dst.AppendFormat(L"(0x%02x)", (uint8_t&)src);
}

template<class T, hvEnableIf(sizeof(T) == 2)>
void to_hexstring(CString& dst, T src)
{
    dst.AppendFormat(L"(0x%04x)", (uint16_t&)src);
}

template<class T, hvEnableIf(sizeof(T) == 4)>
void to_hexstring(CString& dst, T src)
{
    dst.AppendFormat(L"(0x%08x)", (uint32_t&)src);
}

template<class T, hvEnableIf(sizeof(T) == 3)>
void to_hexstring(CString& dst, T src_)
{
    auto* src = (uint8_t*)&src_;

    uint32_t u32 = 0;
    u32 |= src[0] << 0;
    u32 |= src[1] << 8;
    u32 |= src[2] << 16;
    dst.AppendFormat(L"(0x%06x)", u32);
}

// packed to string in "float (hex)" format
// (e.g. "1.000000 (0x3c00)")
template<class T>
void PackedScalarEvaluator<T>::to_string(CString& dst, T src)
{
    dst.Format(L"%f ", src.to_float());
    to_hexstring(dst, src);
}



template<class T, hvEnableIf(sizeof(T) == 1)>
bool from_hexstring(T& dst, const WCHAR* src)
{
    uint32_t t;
    if (swscanf(src, L"0x%x", &t) == 1) {
        (uint8_t&)dst = (uint8_t)t;
        return true;
    }
    return false;
}

template<class T, hvEnableIf(sizeof(T) == 2)>
bool from_hexstring(T& dst, const WCHAR* src)
{
    uint32_t t;
    if (swscanf(src, L"0x%x", &t) == 1) {
        (uint16_t&)dst = (uint16_t)t;
        return true;
    }
    return false;
}

template<class T, hvEnableIf(sizeof(T) == 4)>
bool from_hexstring(T& dst, const WCHAR* src)
{
    uint32_t t;
    if (swscanf(src, L"0x%x", &t) == 1) {
        (uint32_t&)dst = t;
        return true;
    }
    return false;
}

template<class T, hvEnableIf(sizeof(T) == 3)>
bool from_hexstring(T& dst_, const WCHAR* src)
{
    uint32_t t;
    if (swscanf(src, L"0x%x", &t) == 1) {
        auto* dst = (uint8_t*)&dst_;
        dst[0] = (t & 0xff) >> 0;
        dst[1] = (t & 0xff00) >> 8;
        dst[2] = (t & 0xff0000) >> 16;
        return true;
    }
    return false;
}

template<class T>
bool PackedScalarEvaluator<T>::from_string(T& dst, const WCHAR* src)
{
    if (from_hexstring(dst, src))
        return true;

    float t;
    if (swscanf(src, L"%f", &t) == 1) {
        dst = t;
        return true;
    }
    return false;
}


template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::EvaluateVisualizedExpression(Evaluation::DkmVisualizedExpression* pVisualizedExpression, Evaluation::DkmEvaluationResult** ppResultObject)
{
    HRESULT hr;

    Evaluation::DkmPointerValueHome* pPointerValueHome = Evaluation::DkmPointerValueHome::TryCast(pVisualizedExpression->ValueHome());
    if (pPointerValueHome == nullptr) {
        return E_NOTIMPL;
    }

    DkmRootVisualizedExpression* pRootVisualizedExpression = DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
    if (pRootVisualizedExpression == nullptr) {
        return E_NOTIMPL;
    }

    DkmProcess* pTargetProcess = pVisualizedExpression->RuntimeInstance()->Process();
    value_t data;
    hr = pTargetProcess->ReadMemory(pPointerValueHome->Address(), DkmReadMemoryFlags::None, &data, sizeof(data), nullptr);
    if (FAILED(hr)) {
        return E_NOTIMPL;
    }

    CString strValue;
    E::to_string(strValue, data);

    CString strEditableValue;

    // If we are formatting a pointer, we want to also show the address of the pointer
    if (pRootVisualizedExpression->Type() != nullptr && wcschr(pRootVisualizedExpression->Type()->Value(), '*') != nullptr)
    {
        // Make the editable value just the pointer string
        UINT64 address = pPointerValueHome->Address();
        if ((pTargetProcess->SystemInformation()->Flags() & DefaultPort::DkmSystemInformationFlags::Is64Bit) != 0) {
            strEditableValue.Format(L"0x%08x%08x", static_cast<DWORD>(address >> 32), static_cast<DWORD>(address));
        }
        else {
            strEditableValue.Format(L"0x%08x", static_cast<DWORD>(address));
        }

        // Prefix the value with the address
        CString strValueWithAddress;
        strValueWithAddress.Format(L"%s {%s}", static_cast<LPCWSTR>(strEditableValue), static_cast<LPCWSTR>(strValue));
        strValue = strValueWithAddress;
    }

    CComPtr<DkmString> pValue;
    hr = DkmString::Create(DkmSourceString(strValue), &pValue);
    if (FAILED(hr)) {
        return hr;
    }

    CComPtr<DkmString> pEditableValue;
    hr = DkmString::Create(strEditableValue, &pEditableValue);
    if (FAILED(hr)) {
        return hr;
    }

    CComPtr<DkmDataAddress> pAddress;
    hr = DkmDataAddress::Create(pVisualizedExpression->RuntimeInstance(), pPointerValueHome->Address(), nullptr, &pAddress);
    if (FAILED(hr)) {
        return hr;
    }

    CComPtr<DkmSuccessEvaluationResult> pSuccessEvaluationResult;
    hr = DkmSuccessEvaluationResult::Create(
        pVisualizedExpression->InspectionContext(),
        pVisualizedExpression->StackFrame(),
        pRootVisualizedExpression->Name(),
        pRootVisualizedExpression->FullName(),
        DkmEvaluationResultFlags::None,
        pValue,
        pEditableValue,
        pRootVisualizedExpression->Type(),
        DkmEvaluationResultCategory::Class,
        DkmEvaluationResultAccessType::None,
        DkmEvaluationResultStorageType::None,
        DkmEvaluationResultTypeModifierFlags::None,
        pAddress,
        (DkmReadOnlyCollection<DkmCustomUIVisualizerInfo*>*)nullptr,
        (DkmReadOnlyCollection<DkmModuleInstance*>*)nullptr,
        DkmDataItem::Null(),
        &pSuccessEvaluationResult
    );
    if (FAILED(hr)) {
        return hr;
    }

    *ppResultObject = pSuccessEvaluationResult.Detach();
    return S_OK;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::UseDefaultEvaluationBehavior(Evaluation::DkmVisualizedExpression* pVisualizedExpression, bool* pUseDefaultEvaluationBehavior, Evaluation::DkmEvaluationResult** ppDefaultEvaluationResult)
{
    // not use default behavior because it disables SetValueAsString()
    *pUseDefaultEvaluationBehavior = false;
    return S_OK;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::GetChildren(Evaluation::DkmVisualizedExpression* pVisualizedExpression, UINT32 InitialRequestSize, Evaluation::DkmInspectionContext* pInspectionContext, DkmArray<Evaluation::DkmChildVisualizedExpression*>* pInitialChildren, Evaluation::DkmEvaluationResultEnumContext** ppEnumContext)
{
    return E_NOTIMPL;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::GetItems(Evaluation::DkmVisualizedExpression* pVisualizedExpression, Evaluation::DkmEvaluationResultEnumContext* pEnumContext, UINT32 StartIndex, UINT32 Count, DkmArray<Evaluation::DkmChildVisualizedExpression*>* pItems)
{
    return E_NOTIMPL;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::SetValueAsString(Evaluation::DkmVisualizedExpression* pVisualizedExpression, DkmString* pValue, UINT32 Timeout, DkmString** ppErrorText)
{
    value_t data;
    if (!pValue || !E::from_string(data, pValue->Value())) {
        return E_INVALIDARG;
    }

    HRESULT hr;
    Evaluation::DkmPointerValueHome* pPointerValueHome = Evaluation::DkmPointerValueHome::TryCast(pVisualizedExpression->ValueHome());
    if (pPointerValueHome == nullptr) {
        return E_NOTIMPL;
    }

    DkmArray<BYTE> tmp;
    tmp.Members = (BYTE*)&data;
    tmp.Length = sizeof(data);
    DkmProcess* pTargetProcess = pVisualizedExpression->RuntimeInstance()->Process();
    hr = pTargetProcess->WriteMemory(pPointerValueHome->Address(), tmp);
    if (FAILED(hr)) {
        return E_NOTIMPL;
    }
    return S_OK;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::GetUnderlyingString(Evaluation::DkmVisualizedExpression* pVisualizedExpression, DkmString** ppStringValue)
{
    return E_NOTIMPL;
}

