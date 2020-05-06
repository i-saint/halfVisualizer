// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// TargetApp.cpp : Example application which will be debuggeed

#include <iostream>
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


    __debugbreak(); // Evaluate values in the locals or watch window.
    return 0;
}
