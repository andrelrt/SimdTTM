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

#include <SimdTTM/detail/simd/compatibility.h>

namespace SimdTTM {
namespace detail {

template< typename Type_T,        // Value type
          size_t NODE_SIZE = 256, // Node with 256 value_types
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
    using node_type = std::array<simd_type, simd_size>;
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
        if( pos < size -1 )
        {
            std::copy_backward( ptr + pos, ptr + size, ptr + size +1 );
        }
        ptr[pos] = val;
    }

    value_type remove( uint16_t pos, uint16_t size )
    {
        value_type* ptr = as_ptr();
        value_type ret = ptr[ pos ];
        if( pos < size -1 )
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

    value_type& operator[]( size_t i )
    {
        return as_ptr()[i];
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

enum class InsertMode
{
    NodeOnly,
    ShiftRight,
    ShiftLeft,
    SplitNode
};

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
        return node_list_[0] == list_end;
    }

    value_type& get( size_t extnode, size_t pos )
    {
        range_check( extnode );
        size_t node = translate_node( extnode ).first;
        return row_[node][pos];
    }

    std::pair<InsertMode, value_type>
    insert( bool shiftEnabled, size_t extnode, value_type val, value_type leftRoot, value_type rightRoot )
    {
        range_check( extnode );
        size_t node;
        size_t prev;
        std::tie( node, prev ) = translate_node( extnode );

        if( node_sizes_[node] < node_size )
        {
            // Insert an item on a node
            int32_t pos = row_[node].upper_bound( val );
            row_[node].insert( val, pos, node_sizes_[node] );
            ++node_sizes_[node];
            return std::make_pair( InsertMode::NodeOnly, value_type(0) );
        }

        if( shiftEnabled )
        {
            // Try shifts, left and right
            if( node != 0 &&
                node_sizes_[ prev ] < node_size )
            {
                int32_t pos = row_[node].upper_bound( val );
                return std::make_pair( InsertMode::ShiftLeft,
                                       insert_shift_left( node, prev, pos, val, leftRoot ) );
            }

            size_t next = node_list_[node];
            if( next != list_end &&
                node_sizes_[ next ] < node_size )
            {
                int32_t pos = row_[node].upper_bound( val );
                return std::make_pair( InsertMode::ShiftRight,
                                       insert_shift_right( node, pos, val, rightRoot ) );
            }
        }

        // The last effort is to split the node
        value_type ret = split_insert_node( node, val );

        fill_translation_map( extnode );

        return std::make_pair( InsertMode::SplitNode, ret );
    }

    std::pair<RemoveMode, value_type>
    remove( size_t extnode, value_type val, value_type leftRoot, value_type rightRoot )
    {
        range_check( extnode );
        size_t prev;
        size_t node;
        std::tie( node, prev ) = translate_node( extnode );

        // Find element on node
        int32_t pos = row_[node].upper_bound( val ) -1;

        if( pos == -1 || row_[node][pos] != val )
            return std::make_pair( RemoveMode::NotFound, value_type(0) );

        size_t next = node_list_[node];

        if( node_sizes_[node] > node_middle ||
            (next == list_end && node == 0) )
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
        if( next != list_end &&
            node_sizes_[ next ] > node_middle )
        {
            return std::make_pair( RemoveMode::ShiftRight,
                                   remove_shift_right( node, pos, rightRoot ) );
        }

        if( node != 0 &&
            node_sizes_[ prev ] > node_middle )
        {
            return std::make_pair( RemoveMode::ShiftLeft,
                                   remove_shift_left( node, prev, pos, leftRoot ) );
        }

        // The last effort is merge right and left
        if( next != list_end )
        {
            remove_merge_right( node, pos, rightRoot );
            fill_translation_map( extnode );
            return std::make_pair( RemoveMode::MergeRight, value_type(0) );
        }
        remove_merge_left( node, prev, pos, leftRoot );
        fill_translation_map( extnode -1 );
        return std::make_pair( RemoveMode::MergeLeft, value_type(0) );
    }

    std::pair<size_t, bool> upper_bound( size_t extnode, value_type val )
    {
        range_check( extnode );
        size_t node = translate_node( extnode ).first;
        int32_t pos = row_[node].upper_bound( val );
        return std::make_pair( pos, pos != 0 && row_[node][pos-1] == val );
    }

    size_t count_before_node( size_t extnode )
    {
        range_check( extnode );
        size_t node = translate_node( extnode ).first;

        size_t sum = 0;
        for( size_t i = 0; i < node; ++ i )
            sum += node_sizes_[i];

        return sum;
    }

private:
    row_type row_;
    std::vector<uint16_t> node_list_;
    std::vector<uint16_t> node_sizes_;
    std::vector<uint16_t> translation_map_;
    size_t last_ordered_node_;
    bool isLeaf_;

    template<typename T, size_t s>
    friend std::ostream& operator<<( std::ostream&, const btree_row<T,s>& );

    static constexpr uint16_t list_end = std::numeric_limits< uint16_t >::max();
    static constexpr value_type empty_value = std::numeric_limits< value_type >::max();

    value_type insert_shift_left( size_t node, size_t prev, int32_t pos, value_type val, value_type root )
    {
        value_type ret = row_[node].remove( 0, node_sizes_[node] );
        row_[node].insert( val, std::max(0, pos-1), node_sizes_[node] -1 );
        row_[prev].insert( root, node_sizes_[prev], node_sizes_[prev] );
        ++node_sizes_[prev];
        return ret;
    }

    value_type insert_shift_right( size_t node, int32_t pos, value_type val, value_type root )
    {
        value_type ret = row_[node].remove( node_sizes_[node] -1, node_sizes_[node] );
        row_[node].insert( val, pos, node_sizes_[node] -1 );

        size_t next = node_list_[node];
        row_[next].insert( root, 0, node_sizes_[next] );
        ++node_sizes_[next];
        return ret;
    }

    value_type split_insert_node( size_t node, value_type val )
    {
        // Split the node and strip the middle item
        size_t next = add_node();
        node_list_[next] = node_list_[node];
        node_list_[node] = next;

        int32_t pos = row_[node].upper_bound( val );
        value_type ret = row_[node].split( row_[next], val, pos );
        node_sizes_[node] = node_middle;
        node_sizes_[next] = node_middle;
        return ret;
    }

    value_type remove_shift_right( size_t node, int32_t pos, value_type root )
    {
        row_[node].remove( pos, node_sizes_[node] );
        row_[node].insert( root, node_sizes_[node]-1, node_sizes_[node] -1 );

        size_t next = node_list_[node];
        value_type ret = row_[next].remove( 0, node_sizes_[next] );
        --node_sizes_[next];
        return ret;
    }

    value_type remove_shift_left( size_t node, size_t prev, int32_t pos, value_type root )
    {
        row_[node].remove( pos, node_sizes_[node] );
        row_[node].insert( root, 0, node_sizes_[node] -1 );

        value_type ret = row_[prev].remove( node_sizes_[prev] -1, node_sizes_[prev] );
        --node_sizes_[prev];
        return ret;
    }

    void remove_merge_right( size_t node, int32_t pos, value_type root )
    {
        size_t next = node_list_[node];

        row_[node].remove( pos, node_sizes_[node] );
        row_[node].merge( row_[next], root, node_sizes_[node] -1, node_sizes_[next] );

        node_sizes_[node] += node_sizes_[next];
        node_list_[node] = node_list_[next];
        node_list_[next] = list_end;
        // XXX may introduce some block fragmentation here.
        //     possible solution: move last block to the empty space created here.
    }

    void remove_merge_left( size_t node, size_t prev, int32_t pos, value_type root )
    {
        row_[node].remove( pos, node_sizes_[node] );
        row_[prev].merge( row_[node], root, node_sizes_[prev], node_sizes_[node] -1 );

        node_sizes_[prev] += node_sizes_[node];
        node_list_[prev] = node_list_[node];
        node_list_[node] = list_end;
        // XXX may introduce some block fragmentation here.
        //     possible solution: move last block to the empty space created here.
    }

    std::pair<size_t, size_t> translate_node( size_t extnode )
    {
        if( extnode == 0 )
            return std::make_pair( 0, 0 );
        return std::make_pair( translation_map_[ extnode ], translation_map_[ extnode-1 ] );
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
        uint16_t val = list_end; // XXX Some weird bug on push_back and constexpr
        node_list_.push_back( val );
        translation_map_.push_back( 0 );
        return node_list_.size() -1;
    }

    void fill_translation_map( uint16_t extnode )
    {
        uint16_t node = translation_map_[ extnode ];
        do
        {
            translation_map_[ extnode ] = node;
            node = node_list_[ node ];

            ++extnode;

        } while( node != list_end );
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
    static_assert( node_size % simd::simd_size< value_type >() == 0,
                   "NODE_SIZE must be multiple of simd_size" );
public:
    template<typename Val_T> using allocator_template = Alloc_T<Val_T>;
    using node_type = std::array<simd_type, simd_size>; // 256 bytes node
    using row_type = btree_row< value_type, node_size, allocator_template >;
    using btree_type = std::vector< row_type, allocator_template< row_type > >;

    btree() : data_(1)
    {
        data_.front().setLeaf( true );
    }

    // TODO Should return iterator
    void insert( value_type val )
    {
        assert( data_.back().isRoot() );

        bool found;
        std::vector< std::pair< size_t, size_t > > nodes;
        std::tie( found, nodes ) = find_node( val );

        // same value is not inserted twice
        if( found )
            return;

        std::pair<InsertMode, value_type> ins = std::make_pair( InsertMode::NodeOnly, val );
        for( size_t i = 0; i < data_.size() -1; ++i )
        {
            auto& row = data_[i];
            auto& parentRow = data_[i+1];

            auto& prevNode = nodes[ nodes.size() -2 ];
            value_type& leftRoot = parentRow.get( prevNode.first, std::max( 1ul, prevNode.second ) -1 );
            value_type& rightRoot = parentRow.get( prevNode.first, prevNode.second );

            ins = row.insert( true, nodes.back().first, ins.second, leftRoot, rightRoot );
            switch( ins.first )
            {
                case InsertMode::NodeOnly:
                    return;

                case InsertMode::ShiftLeft:
                    leftRoot = ins.second;
                    return;

                case InsertMode::ShiftRight:
                    rightRoot = ins.second;
                    return;
            }

            nodes.pop_back();
        }
        ins = data_.back().insert( false, nodes.back().first, ins.second, 0, 0 );
        if( ins.first == InsertMode::NodeOnly )
            return;

        // New root
        data_.emplace_back();
        data_.back().insert( false, 0, ins.second, 0, 0 );
    }

    // TODO Should return iterator
    void erase( value_type val )
    {
        assert( data_.back().isRoot() );

        bool found;
        std::vector< size_t > nodes;
        std::tie( found, nodes ) = find_nodes( val );

        if( !found )
            return;
    }

private:
    btree_type data_;

    template<typename T, size_t s>
    friend std::ostream& operator<<( std::ostream&, const btree<T,s>& );

    std::pair<bool, std::vector< std::pair< size_t, size_t >>>
    find_node( value_type val )
    {
        std::vector< std::pair< size_t, size_t > > nodes;
        nodes.reserve( data_.size() );

        size_t curNode = 0;
        for( size_t i = data_.size()-1; i > 0; --i )
        {
            auto next = data_[i].upper_bound( curNode, val );
            nodes.emplace_back( curNode, next.first );

            if( next.second )
                return std::make_pair( true, nodes );

            size_t before = data_[i].count_before_node( curNode );
            curNode = before + next.first;
        }
        auto next = data_[0].upper_bound( curNode, val );
        nodes.emplace_back( curNode, next.first );

        return std::make_pair( next.second, nodes );
    }
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
        out << "(" << i << "->" << row.node_list_[i] << ":" << row.node_sizes_[i] << ")";
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

using detail::btree;

} // namespace SimdTTM

