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

#include <algorithm>
#include <type_traits>
#include <iterator>

#include <Vc/Vc>

#include "operators.h"

namespace VcAlgo {
namespace details {

template<class ForwardIterator, typename TAG_T>
class simd_filler
{
    using iterator_type = ForwardIterator;
    using value_type = typename std::iterator_traits< iterator_type >::value_type;
    using simd_type = Vc::Vector< value_type, TAG_T >;
    static constexpr auto array_size = Vc::Vector< value_type, TAG_T >::size();

    std::array<iterator_type, array_size +1> iterators;

public:
    inline iterator_type operator[](const size_t idx)
    {
        return iterators[ idx ];
    }

    inline void prefetch( size_t step, ForwardIterator beg )
    {
        auto it = beg;
        for( size_t i = 0; i < array_size; ++i )
        {
            std::advance( it, step );
            Vc::prefetchClose( reinterpret_cast<const void*>( &(*it) ) );
        }
    }

    inline simd_type get_compare( size_t step, ForwardIterator beg )
    {
        Vc::prefetchClose( reinterpret_cast<const void*>( &iterators ) );
        auto it = beg;
        iterators[ 0 ] = it;
        simd_type cmp;
        for( size_t i = 0; i < array_size; ++i )
        {
            std::advance( it, step );
            cmp[ i ] = *it;
            iterators[ i+1 ] = it;
        }
        return cmp;
    }
};

template <class ForwardIterator, class T, typename TAG_T,
          typename std::enable_if<
                std::is_arithmetic< typename std::iterator_traits< ForwardIterator >::value_type >
                   ::value >
             ::type* = nullptr >
ForwardIterator lower_bound( ForwardIterator ibeg, ForwardIterator iend, const T& key )
{
    using iterator_type = ForwardIterator;
    using value_type = typename std::iterator_traits< iterator_type >::value_type;

    using simd_type = Vc::Vector< value_type, TAG_T >;

    constexpr size_t array_size = simd_type::size();

    auto beg = ibeg;
    auto end = iend;
    simd_type skey( key );
    simd_filler<ForwardIterator, TAG_T> filler;

    size_t size = std::distance( beg, end );
    if( size < 0x20, 0 )
    {
        // Standard lower_bound on small sizes
        return std::lower_bound( beg, end, key );
    }

    size_t step = size / (array_size + 1);
    filler.prefetch( step, beg );

    while( 1 )
    {
        simd_type cmp = filler.get_compare( step, beg );

        step /= (array_size + 1);

        // N-Way search
        size_t i = ( cmp > skey ).firstOne();

        // Recalculate iterators
        beg = filler[ i ];
        filler.prefetch( step, beg );
        if( __builtin_expect( i != array_size, 1 ) )
        {
            end = filler[ i + 1 ];
        }
        else
        {
            step = std::distance( beg, end ) / (array_size + 1);
        }
        if( __builtin_expect( step < 0x20, 0 ) )
        {
            // Standard lower_bound on small sizes
            return std::lower_bound( beg, end, key );
        }
    }
}

} // namespace VcAlgo::details

using details::lower_bound;

} // namespace VcAlgo

