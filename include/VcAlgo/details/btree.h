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

#include <deque>
#include <memory>
#include <utility>
#include <functional>
#include <Vc/Vc>

#include "operators.h"

namespace VcAlgo {
namespace details {

template< typename Key_T,    // Key to the search tree
          typename Index_T,  // Index on btree container
          template <class...> class Alloc_T = std::allocator >
class btree_index
{
public:
    using key_type = Key_T;
    using index_type = Index_T;
    using pair_type = std::pair< key_type, index_type >;

    void insert( key_type key, index_type index );
    void remove( key_type key );
    index_type find( key_type key ); // TODO should be the iterator to index_type
    void reserve( size_t size );
};


template< typename Type_T,
          class Hash_T = std::hash<Type_T>,
          class Pred_T = std::equal_to<Type_T>,
          template <class...> class Alloc_T = std::allocator,
          class Container_T = std::deque<Type_T, Alloc_T<Type_T>> >
class btree
{
public:
    // specific member types
    template <class... T>
    using generic_allocator_type = Alloc_T<T...>;
    using index_type = btree_index<int32_t, size_t, generic_allocator_type>;

    // std-like member types
    using key_type = Type_T;
    using value_type = Type_T;
    using hasher = Hash_T;
    using key_equal = Pred_T;
    using container_type = Container_T;
    using allocator_type = typename container_type::allocator_type;
    using size_type = typename container_type::size_type;

    // iterator types

    void reserve( size_t size )
    {
        data_.reserve( size );
        index_.reserve( size );
    }

    void insert( value_type value )
    {
        data_.push_back( std::move(value) );
        index_.insert( hasher()(data_.back()), data_.size()-1 );
    }

    template< typename... Args_T >
    void emplace( Args_T&&... args )
    {
        data_.emplace_back( std::forward<Args_T...>( args... ) );
        index_.insert( hasher()(data_.back()), data_.size()-1 );
    }



private:
    container_type data_;
    index_type index_;
};

} // namespace VcAlgo::details

//using details::lower_bound;

} // namespace VcAlgo
