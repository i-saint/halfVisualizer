#include <iostream>
#include <vector>
#include <windows.h>
#include "../halfVisualizer/half.h"

int wmain(int argc, WCHAR* argv[])
{
    half h1 = 0.1f;
    half h2 = 1.0f;

    snorm8 s8 = 0.1f;
    unorm8 u8 = 0.1f;
    unorm8n u8n = 0.1f;
    snorm16 s16 = 0.1f;
    unorm16 u16 = 0.1f;
    snorm24 s24 = 0.1f;
    snorm32 s32 = 0.1f;

    std::vector<half> hv;
    hv.resize(8, 0.5f);

    half ha[4]{};

    __debugbreak(); // Evaluate values in the locals or watch window.
    return 0;
}
