// MIT License
//
// Copyright (c) 2018-2019 André Tupinambá
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <nmmintrin.h> // SSE 4.2
#include <type_traits>
#include <iostream>

#include <boost/align/aligned_allocator.hpp>

namespace SimdTTM {
namespace detail {
namespace simd {

template< typename Val_T > struct simd_type{};
template<> struct simd_type< int8_t >
{
    using type = __m128i;
    type data_;

    simd_type() : data_( _mm_setzero_si128() ) {}
    simd_type( int8_t val ) : data_( _mm_set1_epi8( val ) ) {}

    static constexpr size_t size() { return sizeof(type) / sizeof(int8_t); }
    inline int8_t& operator[]( size_t i ) { return reinterpret_cast<int8_t*>(&data_)[i]; }

    inline int greater_than( simd_type< int8_t > rhs ) const
    {
        auto simdmask = _mm_cmpgt_epi8( data_, rhs.data_ );
        auto mask = _mm_movemask_epi8( simdmask );
        int def = 16;
        int bit;
        __asm__(
            "bsf %[mask], %[bit];"
            "cmovz %[def], %[bit];"
            :[bit] "=r" (bit)
            :[mask] "r"(mask), [def] "r"(def));
        return bit;
    }
};

template<> struct simd_type< int16_t >
{
    using type = __m128i;
    type data_;

    simd_type() : data_( _mm_setzero_si128() ) {}
    simd_type( int16_t val ) : data_( _mm_set1_epi16( val ) ) {}

    static constexpr size_t size() { return sizeof(type) / sizeof(int16_t); }
    inline int16_t& operator[]( size_t i ) { return reinterpret_cast<int16_t*>(&data_)[i]; }

    inline int greater_than( simd_type< int16_t > rhs ) const
    {
        auto simdmask = _mm_cmpgt_epi16( data_, rhs.data_ );
        auto mask = _mm_movemask_epi8( simdmask );
        int def = 16;
        int bit;
        __asm__(
            "bsf %[mask], %[bit];"
            "cmovz %[def], %[bit];"
            :[bit] "=r" (bit)
            :[mask] "r"(mask), [def] "r"(def));
        return bit >> 1;
    }
};

template<> struct simd_type< int32_t >
{
    using type = __m128i;
    type data_;

    simd_type() : data_( _mm_setzero_si128() ) {}
    simd_type( int32_t val ) : data_( _mm_set1_epi32( val ) ) {}

    static constexpr size_t size() { return sizeof(type) / sizeof(int32_t); }
    inline int32_t& operator[]( size_t i ) { return reinterpret_cast<int32_t*>(&data_)[i]; }

    inline int greater_than( simd_type< int32_t > rhs ) const
    {
        auto simdmask = _mm_cmpgt_epi32( data_, rhs.data_ );
        auto mask = _mm_movemask_epi8( simdmask );
        int def = 16;
        int bit;
        __asm__(
            "bsf %[mask], %[bit];"
            "cmovz %[def], %[bit];"
            :[bit] "=r" (bit)
            :[mask] "r"(mask), [def] "r"(def));
        return bit >> 2;
    }
};

template<> struct simd_type< int64_t >
{
    using type = __m128i;
    type data_;

    simd_type() : data_( _mm_setzero_si128() ) {}
    simd_type( int64_t val ) : data_( _mm_set1_epi64x( val ) ) {}

    static constexpr size_t size() { return sizeof(type) / sizeof(int64_t); }
    inline int64_t& operator[]( size_t i ) { return reinterpret_cast<int64_t*>(&data_)[i]; }

    inline int greater_than( simd_type< int64_t > rhs ) const
    {
        auto simdmask = _mm_cmpgt_epi64( data_, rhs.data_ );
        auto mask = _mm_movemask_epi8( simdmask );
        int def = 16;
        int bit;
        __asm__(
            "bsf %[mask], %[bit];"
            "cmovz %[def], %[bit];"
            :[bit] "=r" (bit)
            :[mask] "r"(mask), [def] "r"(def));
        return bit >> 3;
    }
};

template<> struct simd_type< float >
{
    using type = __m128;
    type data_;

    simd_type() : data_( _mm_setzero_ps() ) {}
    simd_type( float val ) : data_( _mm_set1_ps( val ) ) {}

    static constexpr size_t size() { return sizeof(type) / sizeof(float); }
    inline float& operator[]( size_t i ) { return reinterpret_cast<float*>(&data_)[i]; }

    inline int greater_than( simd_type< float > rhs ) const
    {
        auto simdmask = _mm_cmpgt_ps( data_, rhs.data_ );
        auto mask = _mm_movemask_ps( simdmask );
        int def = 4;
        int bit;
        __asm__(
            "bsf %[mask], %[bit];"
            "cmovz %[def], %[bit];"
            :[bit] "=r" (bit)
            :[mask] "r"(mask), [def] "r"(def));
        return bit;
    }
};

template<> struct simd_type< double >
{
    using type = __m128d;
    type data_;

    simd_type() : data_( _mm_setzero_pd() ) {}
    simd_type( double val ) : data_( _mm_set1_pd( val ) ) {}

    static constexpr size_t size() { return sizeof(type) / sizeof(double); }
    inline double& operator[]( size_t i ) { return reinterpret_cast<double*>(&data_)[i]; }

    inline int greater_than( simd_type< double > rhs ) const
    {
        auto simdmask = _mm_cmpgt_pd( data_, rhs.data_ );
        auto mask = _mm_movemask_pd( simdmask );
        int def = 2;
        int bit;
        __asm__(
            "bsf %[mask], %[bit];"
            "cmovz %[def], %[bit];"
            :[bit] "=r" (bit)
            :[mask] "r"(mask), [def] "r"(def));
        return bit;
    }
};

template< typename Val_T >
using allocator = boost::alignment::aligned_allocator< Val_T >;

template< typename Val_T >
static constexpr size_t simd_size()
{
    return sizeof(__m128i) / sizeof(Val_T);
}

static inline void prefetch( const void* addr )
{
    _mm_prefetch( addr, _MM_HINT_T0 );
}

template< typename Val_T >
static inline simd_type< Val_T > from_value( Val_T val )
{
    return simd_type< Val_T >( val );
}

template< typename LHS, typename RHS >
static inline int greater_than( const LHS vec, const RHS val )
{
    return vec.greater_than( val );
}

template< typename Val_T >
std::ostream& operator<<( std::ostream& out, simd_type< Val_T > rhs )
{
    bool first = true;

    for( size_t i = 0; i < simd_size< Val_T >(); ++i )
    {
        if( first )
        {
            out << "[";
            first = false;
        }
        else
        {
            out << ",";
        }
        out << rhs[ i ];
    }
    out << "]";
    return out;
}

}}} // namespace SimdTTM::detail::simd
