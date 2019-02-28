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

namespace VcAlgo {
namespace details {

template< typename Vector_T, typename Val_T >
static inline int VcGreaterThan( const Vector_T vec, const Val_T val )
{
    auto mask = (vec > val);
    return Vc::none_of(mask) ? Vector_T::size() : mask.firstOne();
}

template<class ForwardIterator, typename TAG_T>
class simd_filler
{
    using iterator_type = ForwardIterator;
    using value_type = typename std::iterator_traits< iterator_type >::value_type;
    using simd_type = Vc::Vector< value_type, TAG_T >;
    static constexpr auto array_size = Vc::Vector< value_type, TAG_T >::size();

    std::array<iterator_type, array_size +1> iterators;

    void fill_iterators( size_t step, ForwardIterator beg )
    {
        auto it = beg;
        iterators[ 0 ] = it;
        for( size_t i = 0; i < array_size; ++i )
        {
            std::advance( it, step );
            Vc::prefetchClose( reinterpret_cast<const void*>( &(*it) ) );
            iterators[ i+1 ] = it;
        }
    }

public:
    inline iterator_type operator[](const size_t idx)
    {
        return iterators[ idx ];
    }

    template< typename IT,
              typename std::enable_if< !std::is_pointer<IT>::value >::type* = nullptr >
    simd_type get_compare( size_t step, IT beg )
    {
        fill_iterators( step, beg );

        // Create SIMD search key
        simd_type cmp;
        for( size_t i = 0; i < array_size; ++i )
        {
            cmp[ i ] = *(iterators[ i+1 ]);
        }
        return cmp;
    }

    template< typename IT,
              typename std::enable_if< std::is_pointer<IT>::value >::type* = nullptr >
    simd_type get_compare( size_t step, IT beg )
    {
        fill_iterators( step, beg );

        // Create Indexes
        simd_type index = (simd_type::IndexesFromZero() + 1) * simd_type( step );

        simd_type cmp;
        cmp.gather( beg, index );
        return cmp;
    }
};

template <class ForwardIterator, class T, typename TAG_T,
          typename std::enable_if<
                std::is_fundamental< typename std::iterator_traits< ForwardIterator >::value_type >
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

    while( 1 )
    {
        size_t size = std::distance( beg, end );
        if( size < 0x20 )
        {
            // Standard lower_bound on small sizes
            return std::lower_bound( beg, end, key );
        }

        size_t step = size / (array_size + 1);
        simd_type cmp = filler.get_compare( step, beg );

        // N-Way search
        size_t i = VcGreaterThan( cmp, skey );

        // Recalculate iterators
        beg = filler[ i ];
        end = ( i == array_size ) ? end : filler[ i + 1 ];
    }
}

} // namespace VcAlgo::details

using details::lower_bound;

} // namespace VcAlgo

