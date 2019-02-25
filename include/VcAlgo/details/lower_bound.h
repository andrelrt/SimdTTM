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

template<class VAL_T, typename TAG_T,
         typename std::enable_if< std::is_pointer<VAL_T>::value >::type* = nullptr >
class simd_filler
{
    using pointer_type = VAL_T;
    using value_type = typename std::remove_pointer< pointer_type >::type;
    using simd_type = Vc::Vector< value_type, TAG_T >;
    static constexpr auto array_size = Vc::Vector< value_type, TAG_T >::size();

    std::array<pointer_type, array_size +1> pointers;

public:
    inline pointer_type operator[](const size_t idx)
    {
        return pointers[ idx ];
    }

    inline simd_type get_compare( size_t step, VAL_T beg )
    {
        auto it = beg;
        pointers[ 0 ] = it;
        for( size_t i = 0; i < array_size; ++i )
        {
            it += step;
            Vc::prefetchClose( reinterpret_cast<const void*>( it ) );
            pointers[ i+1 ] = it;
        }

        // Create Indexes
        simd_type index = (simd_type::IndexesFromZero() + 1) * step;

        simd_type cmp;
        cmp.gather( beg, index );
        return cmp;
    }
};


template<class ForwardIterator, typename TAG_T,
         typename std::enable_if< !std::is_pointer<ForwardIterator>::value >::type* = nullptr >
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

    inline simd_type get_compare( size_t step, ForwardIterator beg )
    {
        auto it = beg;
        iterators[ 0 ] = it;
        for( size_t i = 0; i < array_size; ++i )
        {
            std::advance( it, step );
            Vc::prefetchClose( reinterpret_cast<const void*>( &(*it) ) );
            iterators[ i+1 ] = it;
        }

        // Create SIMD search key
        simd_type cmp;
        for( size_t i = 0; i < array_size; ++i )
        {
            cmp[ i ] = *(iterators[ i+1 ]);
        }
        return cmp;
    }
};


template <class ForwardIterator, class T, typename TAG_T >
ForwardIterator fwd_lower_bound( ForwardIterator beg, ForwardIterator end,
                                  const Vc::Vector< T, TAG_T > key )
{
    using iterator_type = ForwardIterator;
    using value_type = typename std::iterator_traits< iterator_type >::value_type;

    using simd_type = Vc::Vector< value_type, TAG_T >;

    constexpr auto array_size = Vc::Vector< value_type, TAG_T >::size();

    size_t size = std::distance( beg, end );
    if( size < 0x20 )
    {
        // Standard lower_bound on small sizes
        return std::lower_bound( beg, end, key[0] );
    }

    simd_filler<ForwardIterator, TAG_T> filler;

    size_t step = size / (array_size + 1);
    simd_type cmp = filler.get_compare( step, beg );

    // N-Way search
    size_t i = VcGreaterThan( cmp, key );

    // Recalculate iterators
    auto it = filler[ i ];
    auto itEnd = ( i == array_size ) ? end : filler[ i + 1 ];

    return fwd_lower_bound< ForwardIterator, T, TAG_T >( it, itEnd, key );
}

template <class ForwardIterator, class T, typename TAG_T,
          typename std::enable_if< 
                std::is_fundamental< typename std::iterator_traits< ForwardIterator >::value_type >
                   ::value >
             ::type* = nullptr >
ForwardIterator lower_bound( ForwardIterator beg, ForwardIterator end, const T& key )
{
    return fwd_lower_bound< ForwardIterator, T, TAG_T >( beg, end, Vc::Vector< T, TAG_T >( key ) );
}

} // namespace VcAlgo::detail

using details::lower_bound;

} // namespace VcAlgo

