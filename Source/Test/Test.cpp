#include <iostream>
#include <vector>
#include <windows.h>
#include "../half.h"

int main(int, char*[])
{
    snorm8  _s8 = 0.1f;
    unorm8  _u8 = 0.1f;
    unorm8n _u8n = 0.1f;
    snorm16 _s16 = 0.1f;
    unorm16 _u16 = 0.1f;
    snorm24 _s24 = 0.1f;
    snorm32 _s32 = 0.1f;

    std::vector<half> hv;
    hv.resize(64);
    for (size_t i = 0; i < hv.size(); ++i)
        hv[i] = float(i * i) * 0.1f;

    __debugbreak(); // evaluate values in the locals or watch window.
}
