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

#include <Vc/Vc>

namespace SimdTTM {
namespace detail {
namespace simd {

template< typename Val_T >
using simd_type = Vc::Vector< Val_T >;

template< typename Val_T >
using allocator = Vc::Allocator< Val_T >;

template< typename Val_T >
static constexpr size_t simd_size()
{
    return simd_type< Val_T >::size();
}

static inline void prefetch( const void* addr )
{
    Vc::prefetchClose( addr );
}

template< typename Val_T >
static inline simd_type< Val_T > from_value( Val_T val )
{
    return simd_type< Val_T >( val );
}

template< typename Vector_T, typename Val_T >
static inline int greater_than( const Vector_T vec, const Val_T val )
{
    int mask = (vec > val).toInt();
    int def = simd_size< typename Vector_T::value_type >();
    int bit;
    __asm__(
        "bsf %[mask], %[bit];"
        "cmovz %[def], %[bit];"
        :[bit] "=r" (bit)
        :[mask] "r"(mask), [def] "r"(def));
    return bit;
}

template< typename Val_T >
static inline simd_type< Val_T > load_unaligned( const void* p )
{
    simd_type< Val_T > ret;
    ret.load( reinterpret_cast<const Val_T*>( p ), Vc::Unaligned );
    return ret;
}

}}} // namespace SimdTTM::detail::simd
