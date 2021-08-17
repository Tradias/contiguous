// Copyright (c) 2021 Dennis Hezel
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CNTGS_NEARESTNEIGHBOR_DISTANCE_HPP
#define CNTGS_NEARESTNEIGHBOR_DISTANCE_HPP

#ifndef NO_MANUAL_VECTORIZATION
#ifdef _MSC_VER
#if (defined(_M_AMD64) || defined(_M_X64) || defined(_M_IX86_FP) == 2)
#define __SSE__
#define __SSE2__
#elif defined(_M_IX86_FP) == 1
#define __SSE__
#endif
#endif

#ifdef __SSE__
#define USE_SSE
#ifdef __AVX__
#define USE_AVX
#endif
#endif
#endif

#if defined(USE_AVX) || defined(USE_SSE)
#ifdef _MSC_VER
#include <intrin.h>

#include <stdexcept>
#else
#include <x86intrin.h>
#endif
#endif

namespace cntgs::bench
{
namespace distances
{
class L2Float
{
  public:
    inline static float compare(const void* pVect1v, const void* pVect2v, const void* qty_ptr)
    {
        float* pVect1 = (float*)pVect1v;
        float* pVect2 = (float*)pVect2v;
        size_t qty = *((size_t*)qty_ptr);

        float res = 0;
        for (size_t i = 0; i < qty; i++)
        {
            float t = *pVect1 - *pVect2;
            pVect1++;
            pVect2++;
            res += t * t;
        }
        return (res);
    }
};

class L2Float16Ext
{
  public:
    inline static float compare(const void* pVect1v, const void* pVect2v, const void* qty_ptr)
    {
        float* a = (float*)pVect1v;
        float* b = (float*)pVect2v;

        size_t size = *((size_t*)qty_ptr);
        const float* last = a + size;
#if defined(USE_AVX512)
        __m512 sum512 = _mm512_setzero_ps();
        while (a < last)
        {
            __m512 v = _mm512_sub_ps(_mm512_load_ps(a), _mm512_load_ps(b));
            sum512 = _mm512_add_ps(sum512, _mm512_mul_ps(v, v));
            a += 16;
            b += 16;
        }

        __m256 sum256 = _mm256_add_ps(_mm512_extractf32x8_ps(sum512, 0), _mm512_extractf32x8_ps(sum512, 1));
        __m128 sum128 = _mm_add_ps(_mm256_extractf128_ps(sum256, 0), _mm256_extractf128_ps(sum256, 1));
#elif defined(USE_AVX)
        __m256 sum256 = _mm256_setzero_ps();
        __m256 v;
        while (a < last)
        {
            v = _mm256_sub_ps(_mm256_load_ps(a), _mm256_load_ps(b));
            sum256 = _mm256_fmadd_ps(v, v, sum256);
            a += 8;
            b += 8;
            v = _mm256_sub_ps(_mm256_load_ps(a), _mm256_load_ps(b));
            sum256 = _mm256_fmadd_ps(v, v, sum256);
            a += 8;
            b += 8;
        }
        __m128 sum128 = _mm_add_ps(_mm256_extractf128_ps(sum256, 0), _mm256_extractf128_ps(sum256, 1));
#elif defined(USE_SSE)
        __m128 sum128 = _mm_setzero_ps();
        __m128 v;
        while (a < last)
        {
            v = _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
            sum128 = _mm_add_ps(sum128, _mm_mul_ps(v, v));
            a += 4;
            b += 4;
            v = _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
            sum128 = _mm_add_ps(sum128, _mm_mul_ps(v, v));
            a += 4;
            b += 4;
            v = _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
            sum128 = _mm_add_ps(sum128, _mm_mul_ps(v, v));
            a += 4;
            b += 4;
            v = _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
            sum128 = _mm_add_ps(sum128, _mm_mul_ps(v, v));
            a += 4;
            b += 4;
        }
#endif

        alignas(32) float f[4];
        _mm_storeu_ps(f, sum128);

        return f[0] + f[1] + f[2] + f[3];
    }
};

class L2Float4Ext
{
  public:
    inline static float compare(const void* pVect1v, const void* pVect2v, const void* qty_ptr)
    {
        alignas(32) float TmpRes[8];
        float* pVect1 = (float*)pVect1v;
        float* pVect2 = (float*)pVect2v;
        size_t qty = *((size_t*)qty_ptr);

        size_t qty4 = qty >> 2;

        const float* pEnd1 = pVect1 + (qty4 << 2);

        __m128 diff, v1, v2;
        __m128 sum = _mm_set1_ps(0);

        while (pVect1 < pEnd1)
        {
            v1 = _mm_loadu_ps(pVect1);
            pVect1 += 4;
            v2 = _mm_loadu_ps(pVect2);
            pVect2 += 4;
            diff = _mm_sub_ps(v1, v2);
            sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));
        }
        _mm_store_ps(TmpRes, sum);
        return TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3];
    }
};

class L2Float16ExtResiduals
{
  public:
    inline static float compare(const void* pVect1v, const void* pVect2v, const void* qty_ptr)
    {
        size_t qty = *((size_t*)qty_ptr);
        size_t qty16 = qty >> 4 << 4;
        float res = bench::distances::L2Float16Ext::compare(pVect1v, pVect2v, &qty16);
        float* pVect1 = (float*)pVect1v + qty16;
        float* pVect2 = (float*)pVect2v + qty16;

        size_t qty_left = qty - qty16;
        float res_tail = bench::distances::L2Float::compare(pVect1, pVect2, &qty_left);
        return (res + res_tail);
    }
};

class L2Float4ExtResiduals
{
  public:
    inline static float compare(const void* pVect1v, const void* pVect2v, const void* qty_ptr)
    {
        size_t qty = *((size_t*)qty_ptr);
        size_t qty4 = qty >> 2 << 2;

        float res = bench::distances::L2Float4Ext::compare(pVect1v, pVect2v, &qty4);
        size_t qty_left = qty - qty4;

        float* pVect1 = (float*)pVect1v + qty4;
        float* pVect2 = (float*)pVect2v + qty4;
        float res_tail = bench::distances::L2Float::compare(pVect1, pVect2, &qty_left);

        return (res + res_tail);
    }
};
}  // namespace distances

template <typename MTYPE>
using DISTFUNC = MTYPE (*)(const void*, const void*, const void*);

class L2Space
{
    DISTFUNC<float> fstdistfunc_;
    size_t data_size_;
    size_t dim_;

  public:
    L2Space(const size_t dim)
    {
        fstdistfunc_ = bench::distances::L2Float::compare;
#if defined(USE_SSE) || defined(USE_AVX)
        if (dim % 16 == 0)
            fstdistfunc_ = bench::distances::L2Float16Ext::compare;
        else if (dim % 4 == 0)
            fstdistfunc_ = bench::distances::L2Float4Ext::compare;
        else if (dim > 16)
            fstdistfunc_ = bench::distances::L2Float16ExtResiduals::compare;
        else if (dim > 4)
            fstdistfunc_ = bench::distances::L2Float4ExtResiduals::compare;
#endif
        dim_ = dim;
        data_size_ = dim * sizeof(float);
    }

    size_t dim() const { return dim_; }

    size_t get_data_size() const { return data_size_; }

    DISTFUNC<float> get_dist_func() const { return fstdistfunc_; }

    const void* get_dist_func_param() const { return &dim_; }
};

inline void prefetch(const char* ptr)
{
#if defined(USE_AVX) || defined(USE_SSE)
    _mm_prefetch(ptr, _MM_HINT_T0);
#endif
}
}  // namespace cntgs::bench

#endif  // CNTGS_NEARESTNEIGHBOR_DISTANCE_HPP
