#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace impl {
    template<class T> inline T clamp(T v, T vmin, T vmax) { return std::min<T>(std::max<T>(v, vmin), vmax); }
    template<class T> inline T clamp01(T v) { return clamp(v, T(0), T(1)); }
    template<class T> inline T clamp11(T v) { return clamp(v, T(-1), T(1)); }
    template<class T> inline T sign(T v) { return v < T(0) ? T(-1) : T(1); }


    // thanks: https://gist.github.com/rygorous/2156668
    typedef unsigned int uint;

    union FP32
    {
        uint u;
        float f;
        struct
        {
            uint Mantissa : 23;
            uint Exponent : 8;
            uint Sign : 1;
        };
    };

    union FP16
    {
        unsigned short u;
        struct
        {
            uint Mantissa : 10;
            uint Exponent : 5;
            uint Sign : 1;
        };
    };

    // Original ISPC reference version; this always rounds ties up.
    inline FP16 float_to_half_full(FP32 f)
    {
        FP16 o = { 0 };

        // Based on ISPC reference code (with minor modifications)
        if (f.Exponent == 0) // Signed zero/denormal (which will underflow)
            o.Exponent = 0;
        else if (f.Exponent == 255) // Inf or NaN (all exponent bits set)
        {
            o.Exponent = 31;
            o.Mantissa = f.Mantissa ? 0x200 : 0; // NaN->qNaN and Inf->Inf
        }
        else // Normalized number
        {
            // Exponent unbias the single, then bias the halfp
            int newexp = f.Exponent - 127 + 15;
            if (newexp >= 31) // Overflow, return signed infinity
                o.Exponent = 31;
            else if (newexp <= 0) // Underflow
            {
                if ((14 - newexp) <= 24) // Mantissa might be non-zero
                {
                    uint mant = f.Mantissa | 0x800000; // Hidden 1 bit
                    o.Mantissa = mant >> (14 - newexp);
                    if ((mant >> (13 - newexp)) & 1) // Check for rounding
                        o.u++; // Round, might overflow into exp bit, but this is OK
                }
            }
            else
            {
                o.Exponent = newexp;
                o.Mantissa = f.Mantissa >> 13;
                if (f.Mantissa & 0x1000) // Check for rounding
                    o.u++; // Round, might overflow to inf, this is OK
            }
        }

        o.Sign = f.Sign;
        return o;
    }

    inline FP32 half_to_float(FP16 h)
    {
        static const FP32 magic = { 113 << 23 };
        static const uint shifted_exp = 0x7c00 << 13; // exponent mask after shift
        FP32 o;

        o.u = (h.u & 0x7fff) << 13;     // exponent/mantissa bits
        uint exp = shifted_exp & o.u;   // just the exponent
        o.u += (127 - 15) << 23;        // exponent adjust

        // handle exponent special cases
        if (exp == shifted_exp)         // Inf/NaN?
            o.u += (128 - 16) << 23;    // extra exp adjust
        else if (exp == 0)              // Zero/Denormal?
        {
            o.u += 1 << 23;             // extra exp adjust
            o.f -= magic.f;             // renormalize
        }

        o.u |= (h.u & 0x8000) << 16;    // sign bit
        return o;
    }
}


struct half
{
    uint16_t value = 0;

    half() {}
    half(float v)
    {
        value = (const uint16_t&)float_to_half_full((impl::FP32&)v);
    }

    half& operator=(float v)
    {
        *this = half(v);
        return *this;
    }

    float to_float() const
    {
        return (const float&)half_to_float((impl::FP16&)value);
    }
    operator float() const { return to_float(); }

    static half zero() { return half(0.0f); }
    static half one() { return half(1.0f); }
};


// -1.0f - 1.0f <-> -127 - 127
struct snorm8
{
    static constexpr float C = float(0x7f);
    static constexpr float R = 1.0f / float(0x7f);

    int8_t value = 0;

    snorm8() {}
    snorm8(float v) : value(int8_t(impl::clamp11(v)* C)) {}

    snorm8& operator=(float v)
    {
        *this = snorm8(v);
        return *this;
    }

    float to_float() const { return (float)value * R; }
    operator float() const { return to_float(); }

    static snorm8 zero() { return snorm8(0.0f); }
    static snorm8 one() { return snorm8(1.0f); }
};

// 0.0f - 1.0f <-> 0 - 255
struct unorm8
{
    static constexpr float C = float(0xff);
    static constexpr float R = 1.0f / float(0xff);

    uint8_t value = 0;

    unorm8() {}
    unorm8(float v) : value(uint8_t(impl::clamp01(v)* C)) {}

    unorm8& operator=(float v)
    {
        *this = unorm8(v);
        return *this;
    }

    float to_float() const { return (float)value * R; }
    operator float() const { return to_float(); }

    static unorm8 zero() { return unorm8(0.0f); }
    static unorm8 one() { return unorm8(1.0f); }
};

// -1.0f - 1.0f <-> 0 - 255
// for audio sample
struct unorm8n
{
    static constexpr float C = float(0xff);
    static constexpr float R = 1.0f / float(0xff);

    uint8_t value = 0;

    unorm8n() {}
    unorm8n(float v) : value(uint8_t((impl::clamp11(v) * 0.5f + 0.5f)* C)) {}

    unorm8n& operator=(float v)
    {
        *this = unorm8n(v);
        return *this;
    }

    float to_float() const { return (float)value * R * 2.0f - 1.0f; }
    operator float() const { return to_float(); }

    static unorm8n zero() { return unorm8n(0.0f); }
    static unorm8n one() { return unorm8n(1.0f); }
};

// -1.0f - 1.0f <-> -32767 - 32767
struct snorm16
{
    static constexpr float C = float(0x7fff);
    static constexpr float R = 1.0f / float(0x7fff);

    int16_t value = 0;

    snorm16() {}
    snorm16(float v) : value(int16_t(impl::clamp11(v)* C)) {}

    snorm16& operator=(float v)
    {
        *this = snorm16(v);
        return *this;
    }

    float to_float() const { return (float)value * R; }
    operator float() const { return to_float(); }

    static snorm16 zero() { return snorm16(0.0f); }
    static snorm16 one() { return snorm16(1.0f); }
};

// 0.0f - 1.0f <-> 0 - 65535
struct unorm16
{
    static constexpr float C = float(0xffff);
    static constexpr float R = 1.0f / float(0xffff);

    uint16_t value = 0;

    unorm16() {}
    unorm16(float v) : value(uint16_t(impl::clamp01(v)* C)) {}

    unorm16& operator=(float v)
    {
        *this = unorm16(v);
        return *this;
    }

    float to_float() const { return (float)value * R; }
    operator float() const { return to_float(); }

    static unorm16 zero() { return unorm16(0.0f); }
    static unorm16 one() { return unorm16(1.0f); }
};

// -1.0f - 1.0f <-> -2147483647 - 2147483647
// for audio sample
struct snorm24
{
    static constexpr double C = double(0x7fffffff);
    static constexpr double R = 1.0 / double(0x7fffffff);

    uint8_t value[3]{};

    snorm24() {}
    snorm24(float v)
    {
        // store upper 24 bits
        int i32 = int((double)impl::clamp11(v) * C);
        value[0] = uint8_t((i32 & 0x0000ff00) >> 8);
        value[1] = uint8_t((i32 & 0x00ff0000) >> 16);
        value[2] = uint8_t((i32 & 0xff000000) >> 24);
    }

    snorm24& operator=(float v)
    {
        *this = snorm24(v);
        return *this;
    }

    float to_float() const
    {
        int i32 = (value[0] << 8) | (value[1] << 16) | (value[2] << 24);
        return float((double)i32 * R);
    }
    operator float() const { return to_float(); }

    static snorm24 zero() { return snorm24(0.0f); }
    static snorm24 one() { return snorm24(1.0f); }
};

// -1.0f - 1.0f <-> -2147483647 - 2147483647
// for audio sample
struct snorm32
{
    static constexpr double C = double(0x7fffffff);
    static constexpr double R = 1.0 / double(0x7fffffff);

    int value = 0;

    snorm32() {}
    snorm32(float v) : value(int((double)impl::clamp11(v)* C)) {}

    snorm32& operator=(float v)
    {
        *this = snorm32(v);
        return *this;
    }

    float to_float() const { return float((double)value * R); }
    operator float() const { return to_float(); }

    static snorm32 zero() { return snorm32(0.0f); }
    static snorm32 one() { return snorm32(1.0f); }
};
