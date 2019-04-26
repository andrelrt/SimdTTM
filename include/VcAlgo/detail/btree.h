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
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <VcAlgo/detail/simd/compatibility.h>

namespace SimdTTM {
namespace detail {

enum class RemoveMode
{
    NotFound,
    NodeOnly,
    NodeOnlyLessThanMin,
    ShiftRight,
    ShiftLeft,
    MergeRight,
    MergeLeft
};

template< typename Type_T, // Value type
          size_t NODE_SIZE = 256,
          typename std::enable_if< std::is_arithmetic< Type_T >::value >::type* = nullptr >
class btree_node
{
public:
    using value_type = Type_T;
    using simd_type = simd::simd_type< value_type >;
private:
    static constexpr size_t node_size = NODE_SIZE;
    static constexpr size_t byte_size = node_size * sizeof(value_type);
    static constexpr size_t simd_size = node_size / simd::simd_size< value_type >();
    static constexpr size_t node_middle = node_size / 2;
    static constexpr size_t simd_size_mask = simd::simd_size< value_type >() -1;
    using node_type = std::array<simd_type, simd_size>; // 256 bytes node
public:

    btree_node()
    {
        auto ev = empty_value;
        value_type* ptr = as_ptr();
        std::fill( ptr, ptr + node_size, ev );
    }

    void insert( value_type val, uint16_t pos, uint16_t size )
    {
        value_type* ptr = as_ptr();
        if( pos < size )
        {
            std::copy_backward( ptr + pos, ptr + size, ptr + size +1 );
        }
        ptr[pos] = val;
    }

    value_type remove( uint16_t pos, uint16_t size )
    {
        value_type* ptr = as_ptr();
        value_type ret = ptr[ pos ];
        if( pos < size )
            std::copy( ptr + pos + 1, ptr + size, ptr + pos );
        ptr[ size-1 ] = empty_value;
        return ret;
    }

    value_type split( btree_node& next, value_type val, uint16_t pos )
    {
        value_type* ptr = as_ptr();
        value_type* nextptr = next.as_ptr();

        value_type ret;
        if( pos < node_middle )
        {
            std::copy( ptr + node_middle, ptr + node_size, nextptr );
            ret = ptr[ node_middle -1 ];

            insert( val, pos, node_middle );
        }
        else if( pos > node_middle )
        {
            std::copy( ptr + node_middle +1, ptr + node_size, nextptr );
            ret = ptr[ node_middle ];

            next.insert( val, pos - node_middle -1, node_middle -1 );
        }
        else // pos == node_middle
        {
            std::copy( ptr + node_middle, ptr + node_size, nextptr );
            ret = val;
        }
        auto ev = empty_value;
        std::fill( ptr + node_middle, ptr + node_size, ev );
        return ret;
    }

    void merge( btree_node& next, value_type val, uint16_t size, uint16_t nextSize )
    {
        value_type* ptr = as_ptr();
        value_type* nextptr = next.as_ptr();

        ptr[ size ] = val;
        std::copy( nextptr, nextptr + nextSize, ptr + size + 1 );
        auto ev = empty_value;
        std::fill( nextptr, nextptr + nextSize, ev );
    }

    value_type back( uint16_t size ) const
    {
        return (*this)[ size -1 ];
    }

    value_type front() const
    {
        return (*this)[0];
    }

    value_type operator[]( size_t i ) const
    {
        return as_ptr()[i];
    }

    size_t upper_bound( value_type val ) const
    {
        int32_t pos = 0;
        int32_t cur = 0;
        simd_type simdVal(val);

        size_t idx = 0;
        do
        {
            cur = simd::greater_than( node_[idx], simdVal );
            pos += cur;
            ++idx;

        } while( cur == simd_type::size() && idx < node_.size() );
        return pos;
    }

private:
    node_type node_;

    static constexpr value_type empty_value = std::numeric_limits< value_type >::max();

    template<typename T, size_t s>
    friend std::ostream& operator<<( std::ostream&, const btree_node<T,s>& );

    value_type* as_ptr() const
    {
        return const_cast<value_type*>(reinterpret_cast<const value_type*>( node_.data() ));
    }
};


template< typename Type_T, // Value type
          size_t NODE_SIZE = 256,
          template <class...> class Alloc_T = simd::allocator,
          typename std::enable_if< std::is_arithmetic< Type_T >::value >::type* = nullptr >
class btree_row
{
public:
    using value_type = Type_T;
    using simd_type = simd::simd_type< value_type >;
private:
    static constexpr size_t node_size = NODE_SIZE;
    static constexpr size_t byte_size = node_size * sizeof(value_type);
    static constexpr size_t simd_size = node_size / simd::simd_size< value_type >();
    static constexpr size_t node_middle = node_size / 2;
    static constexpr size_t simd_size_mask = simd::simd_size< value_type >() -1;
public:
    template<typename Val_T> using allocator_template = Alloc_T<Val_T>;
    using node_type = btree_node< value_type, node_size >;
    using row_type = std::vector<node_type, allocator_template<node_type>>;

    btree_row() : last_ordered_node_(0), isLeaf_(false)
    {
        add_node();
    }

    void setLeaf(bool isLeaf = true) { isLeaf_ = isLeaf; }
    bool getLeaf() const { return isLeaf_; }

    bool isRoot() const
    {
        return node_map_[0] == map_end;
    }

    std::pair<bool, value_type> insert( size_t extnode, value_type val )
    {
        range_check( extnode );
        size_t node = translateNode( extnode ).first;

        if( node_sizes_[node] < node_size )
        {
            // Insert an item on a node
            int32_t pos = row_[node].upper_bound( val );
            row_[node].insert( val, pos, node_sizes_[node] );
            ++node_sizes_[node];
            return std::make_pair( false, value_type(0) );
        }

        return std::make_pair( true, split_insert_node( node, val ) );
    }

    std::pair<RemoveMode, value_type> remove( size_t extnode, value_type val,
                                              value_type leftRoot, value_type rightRoot )
    {
        range_check( extnode );
        size_t prev;
        size_t node;
        std::tie( node, prev ) = translateNode( extnode );

        // Find element on node
        int32_t pos = row_[node].upper_bound( val ) -1;
        std::cout << pos << ":"
                  << node_sizes_[node] << ":"
                  << node << ":"
                  << row_[node][pos] << ":"
                  << val << ",";

        if( pos == -1 || row_[node][pos] != val )
            return std::make_pair( RemoveMode::NotFound, value_type(0) );

        if( node_sizes_[node] > node_middle ||
            (node_map_[node] == map_end && node == 0) )
        {
            // Just remove from node
            row_[node].remove( pos, node_sizes_[node] );
            --node_sizes_[node];
            if( node_sizes_[node] >= node_middle )
                return std::make_pair( RemoveMode::NodeOnly, value_type(0) );
            else
                return std::make_pair( RemoveMode::NodeOnlyLessThanMin, value_type(0) );
        }

        // Try shifts, right and left
        if( node_map_[node] != map_end &&
            node_sizes_[ node_map_[node] ] > node_middle )
        {
            return std::make_pair( RemoveMode::ShiftRight,
                                   removeShiftRight( node, pos, rightRoot ) );
        }

        if( node != 0 &&
            node_sizes_[ prev ] > node_middle )
        {
            return std::make_pair( RemoveMode::ShiftLeft,
                                   removeShiftLeft( node, prev, pos, leftRoot ) );
        }

        // The last effort is merge right and left
        if( node_map_[node] != map_end )
        {
            removeMergeRight( node, pos, rightRoot );
            return std::make_pair( RemoveMode::MergeRight, value_type(0) );
        }
        removeMergeLeft( node, prev, pos, leftRoot );
        return std::make_pair( RemoveMode::MergeLeft, value_type(0) );
    }

    std::pair<size_t, bool> upper_bound( size_t extnode, value_type val )
    {
        range_check( extnode );
        size_t node = translateNode( extnode ).first;
        int32_t pos = row_[node].upper_bound( val );
        return std::make_pair( pos, pos != 0 && row_[node][pos-1] == val );
    }

    size_t countBeforeNode( size_t extnode )
    {
        range_check( extnode );
        size_t node = translateNode( extnode ).first;

        size_t sum = 0;
        for( size_t i = 0; i < node; ++ i )
            sum += node_sizes_[i];

        return sum;
        //return std::accumulate( node_sizes_.begin(), node_sizes_.begin() + node, size_t(0) );
    }

private:
    row_type row_;
    std::vector<uint16_t> node_map_;
    std::vector<uint16_t> node_sizes_;
    size_t last_ordered_node_;
    bool isLeaf_;

    template<typename T, size_t s>
    friend std::ostream& operator<<( std::ostream&, const btree_row<T,s>& );

    static constexpr uint16_t map_end = std::numeric_limits< uint16_t >::max();
    static constexpr value_type empty_value = std::numeric_limits< value_type >::max();

    value_type split_insert_node( size_t node, value_type val )
    {
        // Split the node and strip the middle item
        size_t next = add_node();
        node_map_[next] = node_map_[node];
        node_map_[node] = next;

        int32_t pos = row_[node].upper_bound( val );
        value_type ret = row_[node].split( row_[next], val, pos );
        node_sizes_[node] = node_middle;
        node_sizes_[next] = node_middle;
        return ret;
    }

    value_type removeShiftRight( size_t node, int32_t pos, value_type root )
    {
        row_[node].remove( pos, node_sizes_[node] );
        row_[node].insert( root, node_sizes_[node]-1, node_sizes_[node] );

        size_t next = node_map_[node];
        value_type ret = row_[next].remove( 0, node_sizes_[next] );
        --node_sizes_[next];
        return ret;
    }

    value_type removeShiftLeft( size_t node, size_t prev, int32_t pos, value_type root )
    {
        row_[node].remove( pos, node_sizes_[node] );
        row_[node].insert( root, 0, node_sizes_[node] );

        value_type ret = row_[prev].remove( node_sizes_[prev] -1, node_sizes_[prev] );
        --node_sizes_[prev];
        return ret;
    }

    void removeMergeRight( size_t node, int32_t pos, value_type root )
    {
        size_t next = node_map_[node];

        row_[node].remove( pos, node_sizes_[node] );
        row_[node].merge( row_[next], root, node_sizes_[node] -1, node_sizes_[next] );

        node_sizes_[node] += node_sizes_[next];
        node_map_[node] = node_map_[next];
        node_map_[next] = map_end;
        // XXX may introduce some block fragmentation here.
        //     possible solution: move last block to the empty space created here.
    }

    void removeMergeLeft( size_t node, size_t prev, int32_t pos, value_type root )
    {
        row_[node].remove( pos, node_sizes_[node] );
        row_[prev].merge( row_[node], root, node_sizes_[prev], node_sizes_[node] -1 );

        node_sizes_[prev] += node_sizes_[node];
        node_map_[prev] = node_map_[node];
        node_map_[node] = map_end;
        // XXX may introduce some block fragmentation here.
        //     possible solution: move last block to the empty space created here.
    }

    std::pair<size_t, size_t> translateNode( size_t extnode )
    {
        if( extnode < last_ordered_node_ )
            return std::make_pair( extnode, extnode -1 );

        uint16_t ret = 0;
        uint16_t prev = 0;
        size_t cur = extnode;
        while( cur != 0 )
        {
            prev = ret;
            ret = node_map_[ ret ];
            if( ret == prev + 1 )
            {
                last_ordered_node_ = ret;
            }
            --cur;
        }
        return std::make_pair( ret, prev );
    }

    void range_check( size_t node )
    {
        if( node >= row_.size() )
        {
            std::stringstream ss;
            ss << "Invalid node number " << node << " (max " << row_.size() -1 << ")";
            throw std::out_of_range(ss.str());
        }
    }

    size_t add_node()
    {
        row_.push_back( node_type() );
        node_sizes_.push_back( 0 );
        uint16_t val = map_end; // XXX Some weird bug on push_back
        node_map_.push_back( val );
        return node_map_.size() -1;
    }
};

template< typename Type_T, // Value type
          size_t NODE_SIZE = 256,
          template <class...> class Alloc_T = simd::allocator,
          typename std::enable_if< std::is_arithmetic< Type_T >::value >::type* = nullptr >
class btree
{
public:
    using value_type = Type_T;
    using simd_type = simd::simd_type< value_type >;
private:
    static constexpr size_t node_size = NODE_SIZE;
    static constexpr size_t byte_size = node_size * sizeof(value_type);
    static constexpr size_t simd_size = node_size / simd::simd_size< value_type >();
    static constexpr size_t node_middle = node_size / 2;
    static constexpr size_t simd_size_mask = simd::simd_size< value_type >() -1;
public:
    template<typename Val_T> using allocator_template = Alloc_T<Val_T>;
    using node_type = std::array<simd_type, simd_size>; // 256 bytes node
    using row_type = btree_row< value_type, node_size, allocator_template >;
    using btree_type = std::vector< row_type, allocator_template< row_type > >;


    btree() : data_(1)
    {
        data_.front().setLeaf( true );
    }

    void insert( value_type val )
    {
        assert( data_.back().isRoot() );

        std::vector< size_t > nodes;
        nodes.reserve( data_.size() );

        size_t curNode = 0;
        for( auto it = data_.rbegin(); it != data_.rend(); ++it )
        {
            auto next = it->upper_bound( curNode, val );
            // same value is not inserted twice
            if( next.second )
                return;

            nodes.push_back( curNode );
            size_t before = it->countBeforeNode( curNode );
            curNode = before + next.first;
        }

        std::pair<bool, value_type> ins = std::make_pair( false, val );
        for( auto& row : data_ )
        {
            ins = row.insert( nodes.back(), ins.second );
            if( !ins.first )
                return;

            nodes.pop_back();
        }
        data_.emplace_back();
        data_.back().insert( 0, ins.second );
    }

private:
    btree_type data_;

    template<typename T, size_t s>
    friend std::ostream& operator<<( std::ostream&, const btree<T,s>& );
};

template<typename T, size_t s>
std::ostream& operator<<( std::ostream& out, const btree_node<T,s>& node )
{
    bool first = true;
    for( auto&& vec : node.node_ )
    {
        if( first )
        {
            first = false;
        }
        else
        {
            out << ",";
        }
        out << vec;
    }
    return out;
}

template< typename Type_T, size_t S >
std::ostream& operator<<( std::ostream& out, const SimdTTM::detail::btree_row< Type_T, S >& row )
{
    size_t i = 0;
    bool first = true;
    for( auto&& node : row.row_ )
    {
        if( first )
        {
            out << "{";
        }
        else
        {
            out << " - ";
        }
        out << "(" << i << "->" << row.node_map_[i] << ":" << row.node_sizes_[i] << ")";
        out << node;
        ++i;
    }
    out << "}";
    return out;
}

template< typename Type_T, size_t S >
std::ostream& operator<<( std::ostream& out, const SimdTTM::detail::btree< Type_T, S >& btree )
{
    size_t i = 0;
    for( auto it = btree.data_.rbegin(); it != btree.data_.rend(); ++it )
    {
        out << "Row " << i << ": " << *it << std::endl;
        out << "-----------------------------------------------------------------------------" << std::endl;
        ++i;
    }
    return out;
}


//template< typename Type_T, // Value type
//          template <class...> class Alloc_T = std::allocator,
//          typename std::enable_if< std::is_arithmetic< Type_T >::value >::type* = nullptr >
//class base_btree
//{
//public:
//    using value_type = Type_T;
//    template<typename Val_T> using allocator_template = Alloc_T<Val_T>;
//    using allocator_type = allocator_template<value_type>;
//    using distance_type = ptrdiff_t;
//    using pointer = value_type*;
//    using reference = value_type&;
//
//    class btree_iterator;
//    using iterator = btree_iterator;
//    using const_iterator = const btree_iterator;
//
//    class btree_iterator :
//        public std::iterator< std::bidirectional_iterator_tag, value_type >
//    {
//    public:
//        ~btree_iterator(){}
//        btree_iterator(base_btree* ptr = nullptr, size_t idx = 0) :
//            ptr_(ptr), index_(idx){}
//
//        btree_iterator(btree_iterator&&) = default;
//        btree_iterator(const btree_iterator&) = default;
//        btree_iterator& operator=(btree_iterator&&) = default;
//        btree_iterator& operator=(const btree_iterator&) = default;
//
//        btree_iterator& operator++() { ++index_; return *this; }
//        const btree_iterator& operator++() const { ++index_; return *this; }
//        btree_iterator operator++(int) { ++index_; return btree_iterator(ptr_, index_-1); }
//        const btree_iterator operator++(int) const { ++index_; return btree_iterator(ptr_, index_-1); }
//
//        btree_iterator& operator--() { --index_; return *this; }
//        const btree_iterator& operator--() const { --index_; return *this; }
//        btree_iterator operator--(int) { --index_; return btree_iterator(ptr_, index_+1); }
//        const btree_iterator operator--(int) const { --index_; return btree_iterator(ptr_, index_+1); }
//
//        bool operator==(const btree_iterator& o) { return (index_ == o.index_) && (ptr_ == o.ptr_); }
//        bool operator!=(const btree_iterator& o) { return !(*this == o); }
//
//        value_type& operator*() {}
//        const value_type& operator*() const {}
//        value_type* operator->() {}
//
//    private:
//        base_btree* ptr_;
//        mutable size_t index_;
//    };
//
//
//    size_t size();
//
//    iterator begin() { return btree_iterator( this, 0 ); }
//    const_iterator begin() const { return btree_iterator( this, 0 ); }
//
//    iterator end() { return btree_iterator( this, size() ); }
//    const_iterator end() const { return btree_iterator( this, size() ); }
//
//    iterator insert( value_type val )
//    {
//    }
//
//private:
//    using container_type = std::vector< btree_row, allocator_template< btree_row > >;
//
//    container_type tree_;
//
//    std::pair<size_t, size_t> find_slot( value_type val )
//    {
//    }
//};
//
//template< typename Key_T,    // Key to the search tree
//          typename Index_T,  // Index on btree container
//          template <class...> class Alloc_T = std::allocator >
//class btree_index
//{
//public:
//    using key_type = Key_T;
//    using index_type = Index_T;
//    using pair_type = std::pair< key_type, index_type >;
//
//    void insert( key_type key, index_type index );
//    void remove( key_type key );
//    index_type find( key_type key ); // TODO should be the iterator to index_type
//    void reserve( size_t size );
//};
//
//
//template< typename Type_T,
//          class Hash_T = std::hash<Type_T>,
//          class Pred_T = std::equal_to<Type_T>,
//          template <class...> class Alloc_T = std::allocator,
//          class Container_T = std::deque<Type_T, Alloc_T<Type_T>> >
//class btree
//{
//public:
//    // specific member types
//    template <class... T>
//    using generic_allocator_type = Alloc_T<T...>;
//    using index_type = btree_index<int32_t, size_t, generic_allocator_type>;
//
//    // std-like member types
//    using key_type = Type_T;
//    using value_type = Type_T;
//    using hasher = Hash_T;
//    using key_equal = Pred_T;
//    using container_type = Container_T;
//    using allocator_type = typename container_type::allocator_type;
//    using size_type = typename container_type::size_type;
//
//    // iterator types
//
//    void reserve( size_t size )
//    {
//        data_.reserve( size );
//        index_.reserve( size );
//    }
//
//    void insert( value_type value )
//    {
//        data_.push_back( std::move(value) );
//        index_.insert( hasher()(data_.back()), data_.size()-1 );
//    }
//
//    template< typename... Args_T >
//    void emplace( Args_T&&... args )
//    {
//        data_.emplace_back( std::forward<Args_T...>( args... ) );
//        index_.insert( hasher()(data_.back()), data_.size()-1 );
//    }
//
//
//
//private:
//    container_type data_;
//    index_type index_;
//};

} // namespace SimdTTM::detail

//using detail::lower_bound;

} // namespace SimdTTM

