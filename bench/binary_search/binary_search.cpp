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

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <array>
#include <vector>
#include <boost/timer/timer.hpp>

#include <Vc/Vc>
#include <VcAlgo/algorithm>

bool g_verbose = true;

template< typename Val_T >
using avector = std::vector< Val_T, Vc::Allocator<Val_T> >;

template< class Cont_T, typename TAG_T >
struct StdLowerBound
{
	using container_type = Cont_T;
    using value_type     = typename container_type::value_type;
    using const_iterator = typename container_type::const_iterator;

    StdLowerBound( const container_type& ref ) : ref_( ref ){}

    void build_index(){}

    const_iterator find( const value_type& key )
    {
        auto first = std::lower_bound( ref_.begin(), ref_.end(), key );
        return (first!=ref_.end() && !(key<*first)) ? first : ref_.end();
    }
private:
    const container_type& ref_;
};

template< class Cont_T, typename TAG_T >
struct VcAlgoLowerBound
{
	using container_type = Cont_T;
    using value_type     = typename container_type::value_type;
    using const_iterator = typename container_type::const_iterator;

    explicit VcAlgoLowerBound( const container_type& ref ) : ref_( ref ){}

    void build_index(){}

    const_iterator find( const value_type& key )
    {
        auto first = VcAlgo::lower_bound< const_iterator,
                                  value_type,
                                  TAG_T>( ref_.begin(), ref_.end(), key );

        return (first!=ref_.end() && !(key<*first)) ? first : ref_.end();
    }
private:
    const container_type& ref_;
};

void do_nothing( int32_t );

template< class Cont_T, template < typename... > class Index_T, typename TAG_T >
uint64_t bench( const std::string& name, size_t size, size_t loop )
{
	using container_type = Cont_T;
    using index_type = Index_T< container_type, TAG_T >;

    boost::timer::cpu_timer timer;
    container_type org;

    srand( 1 );
    std::generate_n( std::back_inserter(org), size, &rand );
    container_type sorted( org );
    std::sort( sorted.begin(), sorted.end() );
    index_type index( sorted );

    index.build_index();

    timer.start();
    for( size_t j = 0; j < loop; ++j )
    {
//        size_t cnt = 0;
        for( auto i : org )
        {
            auto ret = index.find( i );
            do_nothing( *ret );

//            // Test if the retun value is correct
//            if( ret == sorted.end() )
//            {
//                std::cout << "end: 0x" << std::hex << i << ", i: 0x" << cnt << std::endl;
//                break;
//            }
//            else
//            {
//                if( *ret != i )
//                {
//                    std::cout << *ret << "," << i << "-";
//                }
//            }
//            ++cnt;
        }
    }
    timer.stop();
    if( g_verbose )
        std::cout << "Find all " << name << ": " << timer.format();

    return timer.elapsed().wall;
}

int main(int argc, char* /*argv*/[])
{
    constexpr size_t runSize = 0x00400000;
    constexpr size_t loop = 10;
    if( argc > 1 )
    {
        g_verbose = false;
        std::cout << "std::lower_bound,VcAlgo::lower_bound SSE,VcAlgo::lower_bound AVX" << std::endl << std::endl << std::endl << std::endl;
    }
    else
    {
        std::cout << "\nsize: 0x" << std::hex << std::setw(8) << std::setfill( '0') << runSize << std::endl << std::endl;
    }
    while( 1 )
    {
        uint64_t base = bench< avector< int32_t >, StdLowerBound, void >( "lower_bound ...........", runSize, loop );
        uint64_t sse = bench< avector< int32_t >, VcAlgoLowerBound, Vc::VectorAbi::Sse >( "VcAlgo::lower_bound SSE", runSize, loop );
        uint64_t avx = bench< avector< int32_t >, VcAlgoLowerBound, Vc::VectorAbi::Avx >( "VcAlgo::lower_bound AVX", runSize, loop );


        if( g_verbose )
        {
            std::cout
                      << std::endl << "VcAlgo::lower_bound Speed up SSE.: " << std::fixed << std::setprecision(2)
                      << static_cast<float>(base)/static_cast<float>(sse) << "x"

                      << std::endl << "VcAlgo::lower_bound Speed up AVX.: " << std::fixed << std::setprecision(2)
                      << static_cast<float>(base)/static_cast<float>(avx) << "x"

                      << std::endl << std::endl;
        }
        else
        {
            std::cout
                << base
                << "," << sse
                << "," << avx
                << std::endl;
        }
    }
    return 0;
}
