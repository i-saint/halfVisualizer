#pragma once
#include "halfVisualizer.Contract.h"
#include "..\half.h"


template<class T>
class PackedScalarEvaluator
{
public:
    using value_t = T;
    static void to_string(CString& dst, T src);
    static bool from_string(T& dst, const WCHAR* src);
};
using halfEvaluator = PackedScalarEvaluator<half>;
using snorm8Evaluator = PackedScalarEvaluator<snorm8>;
using unorm8Evaluator = PackedScalarEvaluator<unorm8>;
using unorm8nEvaluator = PackedScalarEvaluator<unorm8n>;
using snorm16Evaluator = PackedScalarEvaluator<snorm16>;
using unorm16Evaluator = PackedScalarEvaluator<unorm16>;
using snorm24Evaluator = PackedScalarEvaluator<snorm24>;
using snorm32Evaluator = PackedScalarEvaluator<snorm32>;


template<class Contract, class Evaluator>
class ATL_NO_VTABLE VisualizerBoilerplate :
    public Contract,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<VisualizerBoilerplate<Contract, Evaluator>, &Contract::ClassId>
{
public:
    using value_t = typename Evaluator::value_t;

    VisualizerBoilerplate() {}
    ~VisualizerBoilerplate() {}

    HRESULT STDMETHODCALLTYPE EvaluateVisualizedExpression(
        _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
        _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppResultObject
    );
    HRESULT STDMETHODCALLTYPE UseDefaultEvaluationBehavior(
        _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
        _Out_ bool* pUseDefaultEvaluationBehavior,
        _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppDefaultEvaluationResult
    );
    HRESULT STDMETHODCALLTYPE GetChildren(
        _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
        _In_ UINT32 InitialRequestSize,
        _In_ Evaluation::DkmInspectionContext* pInspectionContext,
        _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pInitialChildren,
        _Deref_out_ Evaluation::DkmEvaluationResultEnumContext** ppEnumContext
    );
    HRESULT STDMETHODCALLTYPE GetItems(
        _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
        _In_ Evaluation::DkmEvaluationResultEnumContext* pEnumContext,
        _In_ UINT32 StartIndex,
        _In_ UINT32 Count,
        _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pItems
    );
    HRESULT STDMETHODCALLTYPE SetValueAsString(
        _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
        _In_ DkmString* pValue,
        _In_ UINT32 Timeout,
        _Deref_out_opt_ DkmString** ppErrorText
    );
    HRESULT STDMETHODCALLTYPE GetUnderlyingString(
        _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
        _Deref_out_opt_ DkmString** ppStringValue
    );
};


#define DefVisualizer(T)\
    class ATL_NO_VTABLE T##Visualizer : public VisualizerBoilerplate<T##VisualizerContract, T##Evaluator>\
    {\
    public:\
        DECLARE_NO_REGISTRY();\
        DECLARE_NOT_AGGREGATABLE(T##Visualizer);\
    };\
    OBJECT_ENTRY_AUTO(T##Visualizer::ClassId, T##Visualizer)

DefVisualizer(half)
DefVisualizer(snorm8)
DefVisualizer(unorm8)
DefVisualizer(unorm8n)
DefVisualizer(snorm16)
DefVisualizer(unorm16)
DefVisualizer(snorm24)
DefVisualizer(snorm32)

#undef DefVisualizer
