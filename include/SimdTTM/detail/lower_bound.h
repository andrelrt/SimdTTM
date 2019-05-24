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

#include <SimdTTM/detail/simd/compatibility.h>

namespace SimdTTM {
namespace detail {

template< typename Val_T, size_t NUM_T >
class set_functor
{
    using value_type = Val_T;
    using simd_type = simd::simd_type< value_type >;

public:
    template< class ForwardIterator >
    simd_type operator()( simd_type vec, ForwardIterator it, size_t step )
    {
        auto ret = set_functor< Val_T, NUM_T -1 >()( vec, it, step );
        std::advance( it, step * (NUM_T+1) );
        ret[ NUM_T ] = *it;
        return ret;
    }
};

template< typename Val_T >
class set_functor< Val_T, 0 >
{
    using value_type = Val_T;
    using simd_type = simd::simd_type< value_type >;

public:
    template< class ForwardIterator >
    simd_type operator()( simd_type vec, ForwardIterator it, size_t step )
    {
        std::advance( it, step );
        vec[ 0 ] = *it;
        return vec;
    }
};

template< class ForwardIterator, size_t array_size >
class simd_filler
{
    using iterator_type = ForwardIterator;
    using value_type = typename std::iterator_traits< iterator_type >::value_type;
    using simd_type = simd::simd_type< value_type >;
    static_assert( array_size <= simd_type::size(), "array_size should be less or equal to simd::size()" );

    //std::array<iterator_type, array_size +1> iterators;

    size_t step_;
    iterator_type beg_;

public:
    inline iterator_type operator[](const size_t idx)
    {
        auto it = beg_;
        std::advance( it, idx * step_ );
        return it;
    }

    inline void prefetch( size_t step, iterator_type beg )
    {
#if defined(USE_MEMORY_PREFETCH)
        auto it = beg;
        for( size_t i = 0; i < array_size; ++i )
        {
            std::advance( it, step );
            simd::prefetch( reinterpret_cast<const void*>( &(*it) ) );
        }
#endif // defined(USE_MEMORY_PREFETCH)
    }

    inline simd_type get_compare( size_t step, iterator_type beg )
    {
        beg_ = beg;
        step_ = step;
        auto it = beg;
        simd_type cmp( std::numeric_limits< value_type >::max() );
        return set_functor< value_type, array_size -1 >()( cmp, it, step );
    }
};

template <size_t array_size, class ForwardIterator, class SIMD_T>
ForwardIterator small_lower_bound( ForwardIterator beg, size_t size, SIMD_T skey )
{
    using simd_type = SIMD_T;
    using iterator_type = ForwardIterator;
    using value_type = typename std::iterator_traits< iterator_type >::value_type;
    //simd::prefetch( reinterpret_cast<const void*>( &(*beg) ) );

    //simd_filler<ForwardIterator, array_size> filler;
    simd_type cmp = simd::load_unaligned< value_type >( &(*beg) );
         //filler.get_compare( 1, beg );
    // N-Way search
    size_t i = simd::greater_than( cmp, skey );
    i = std::min<size_t>( i, size-1 );

    std::advance( beg, i );
    return beg;
}

template <class ForwardIterator, class T,
          size_t array_size = simd::simd_size< typename std::iterator_traits< ForwardIterator >::value_type >(),
          typename std::enable_if<
                std::is_arithmetic< typename std::iterator_traits< ForwardIterator >::value_type >
                   ::value >
             ::type* = nullptr >
ForwardIterator lower_bound( ForwardIterator ibeg, ForwardIterator iend, const T& key )
{
    using iterator_type = ForwardIterator;
    using value_type = typename std::iterator_traits< iterator_type >::value_type;
    using simd_type = simd::simd_type< value_type >;
    static_assert( array_size <= simd_type::size(), "array_size should be less or equal to simd::size()" );

    auto beg = ibeg;
    auto end = iend;
    simd_filler<ForwardIterator, array_size> filler;

    size_t size = std::distance( beg, end );
    simd_type skey = simd::from_value( key );
    if( size < array_size+1 )
    {
        // Standard lower_bound on small sizes
        //return std::lower_bound( beg, end, key );
        return small_lower_bound< array_size >( beg, size, skey );
    }

    size_t step = size / (array_size + 1);
    filler.prefetch( step, beg );

    while( 1 )
    {
        simd_type cmp = filler.get_compare( step, beg );

        step /= (array_size + 1);

        // N-Way search
        size_t i = simd::greater_than( cmp, skey );
        i = std::min<size_t>( i, array_size );

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
        if( __builtin_expect( std::distance( beg, end ) < array_size+1, 0 ) )
        {
            // Standard lower_bound on small sizes
            //return std::lower_bound( beg, end, key );
            return small_lower_bound< array_size >( beg, std::distance( beg, end ), skey );
        }
    }
}

} // namespace SimdTTM::detail

using detail::lower_bound;

} // namespace SimdTTM
