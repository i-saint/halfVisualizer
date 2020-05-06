// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// _CppCustomVisualizerService.cpp : Implementation of FiletimeVisualizer

#include "stdafx.h"
#include "halfVisualizer.h"


template<class T>
void PackedScalarEvaluator<T>::to_string(CString& dst, T src)
{
    char buf[128];
    sprintf(buf, "%f", src.to_float());
    dst = buf;
}

template<class T>
bool PackedScalarEvaluator<T>::to_value(T& dst, const CString& src)
{
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

    // Format this FILETIME as a string
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

    DkmEvaluationResultFlags_t resultFlags = DkmEvaluationResultFlags::Expandable;
    if (strEditableValue.IsEmpty()) {
        resultFlags |= DkmEvaluationResultFlags::ReadOnly;
    }

    CComPtr<DkmSuccessEvaluationResult> pSuccessEvaluationResult;
    hr = DkmSuccessEvaluationResult::Create(
        pVisualizedExpression->InspectionContext(),
        pVisualizedExpression->StackFrame(),
        pRootVisualizedExpression->Name(),
        pRootVisualizedExpression->FullName(),
        resultFlags,
        pValue,
        pEditableValue,
        pRootVisualizedExpression->Type(),
        DkmEvaluationResultCategory::Class,
        DkmEvaluationResultAccessType::None,
        DkmEvaluationResultStorageType::None,
        DkmEvaluationResultTypeModifierFlags::None,
        pAddress,
        nullptr,
        (DkmReadOnlyCollection<DkmModuleInstance*>*)nullptr,
        DkmDataItem::Null(),
        &pSuccessEvaluationResult
    );
    if (FAILED(hr))
    {
        return hr;
    }

    *ppResultObject = pSuccessEvaluationResult.Detach();
    return S_OK;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::UseDefaultEvaluationBehavior(Evaluation::DkmVisualizedExpression* pVisualizedExpression, bool* pUseDefaultEvaluationBehavior, Evaluation::DkmEvaluationResult** ppDefaultEvaluationResult)
{
    HRESULT hr;

    DkmRootVisualizedExpression* pRootVisualizedExpression = DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
    if (pRootVisualizedExpression == nullptr) {
        return E_NOTIMPL;
    }

    DkmInspectionContext* pParentInspectionContext = pVisualizedExpression->InspectionContext();

    CAutoDkmClosePtr<DkmLanguageExpression> pLanguageExpression;
    hr = DkmLanguageExpression::Create(
        pParentInspectionContext->Language(),
        DkmEvaluationFlags::TreatAsExpression,
        pRootVisualizedExpression->FullName(),
        DkmDataItem::Null(),
        &pLanguageExpression
    );
    if (FAILED(hr)) {
        return hr;
    }

    // show raw value
    CComPtr<DkmInspectionContext> pInspectionContext;
    if (DkmComponentManager::IsApiVersionSupported(DkmApiVersion::VS16RTMPreview)) {
        // If we are running in VS 16 or newer, use this overload...
        hr = DkmInspectionContext::Create(
            pParentInspectionContext->InspectionSession(),
            pParentInspectionContext->RuntimeInstance(),
            pParentInspectionContext->Thread(),
            pParentInspectionContext->Timeout(),
            DkmEvaluationFlags::TreatAsExpression | DkmEvaluationFlags::ShowValueRaw,
            pParentInspectionContext->FuncEvalFlags(),
            pParentInspectionContext->Radix(),
            pParentInspectionContext->Language(),
            pParentInspectionContext->ReturnValue(),
            (Evaluation::DkmCompiledVisualizationData*)nullptr,
            Evaluation::DkmCompiledVisualizationDataPriority::None,
            pParentInspectionContext->ReturnValues(),
            pParentInspectionContext->SymbolsConnection(),
            &pInspectionContext
        );
    }
    else {
        hr = E_NOTIMPL;
    }
    if (FAILED(hr)) {
        return hr;
    }

    CComPtr<DkmEvaluationResult> pEEEvaluationResult;
    hr = pVisualizedExpression->EvaluateExpressionCallback(
        pInspectionContext,
        pLanguageExpression,
        pVisualizedExpression->StackFrame(),
        &pEEEvaluationResult
    );
    if (FAILED(hr)) {
        return hr;
    }

    *ppDefaultEvaluationResult = pEEEvaluationResult.Detach();
    *pUseDefaultEvaluationBehavior = true;
    return S_OK;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::GetChildren(Evaluation::DkmVisualizedExpression* pVisualizedExpression, UINT32 InitialRequestSize, Evaluation::DkmInspectionContext* pInspectionContext, DkmArray<Evaluation::DkmChildVisualizedExpression*>* pInitialChildren, Evaluation::DkmEvaluationResultEnumContext** ppEnumContext)
{
    // not needed
    return E_NOTIMPL;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::GetItems(Evaluation::DkmVisualizedExpression* pVisualizedExpression, Evaluation::DkmEvaluationResultEnumContext* pEnumContext, UINT32 StartIndex, UINT32 Count, DkmArray<Evaluation::DkmChildVisualizedExpression*>* pItems)
{
    // not needed
    return E_NOTIMPL;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::SetValueAsString(Evaluation::DkmVisualizedExpression* pVisualizedExpression, DkmString* pValue, UINT32 Timeout, DkmString** ppErrorText)
{
    // todo: float text to half
    return E_NOTIMPL;
}

template<class C, class E>
HRESULT STDMETHODCALLTYPE VisualizerBoilerplate<C, E>::GetUnderlyingString(Evaluation::DkmVisualizedExpression* pVisualizedExpression, DkmString** ppStringValue)
{
    // not needed
    return E_NOTIMPL;
}

