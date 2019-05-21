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
#include <boost/align/aligned_allocator.hpp> 

#include <SimdTTM/algorithm.h>
#include <SimdTTM/detail/simd/compatibility.h>

template< typename Val_T >
using avector = std::vector< Val_T, boost::alignment::aligned_allocator<Val_T> >;

template< class Cont_T >
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

template< class Cont_T, size_t array_size >
struct SimdTTMLowerBound
{
	using container_type = Cont_T;
    using value_type     = typename container_type::value_type;
    using const_iterator = typename container_type::const_iterator;

    explicit SimdTTMLowerBound( const container_type& ref ) : ref_( ref ){}

    void build_index(){}

    const_iterator find( const value_type& key )
    {
        auto first = SimdTTM::lower_bound< const_iterator, value_type, array_size >
                        ( ref_.begin(), ref_.end(), key );

        return (first!=ref_.end() && !(key<*first)) ? first : ref_.end();
    }
private:
    const container_type& ref_;
};

template< class Cont_T >
struct SimdTTMPointerLowerBound
{
	using container_type = Cont_T;
    using value_type     = typename container_type::value_type;
    using const_iterator = typename container_type::const_iterator;

    explicit SimdTTMPointerLowerBound( const container_type& ref ) : ref_( ref ){}

    void build_index(){}

    const_iterator find( const value_type& key )
    {
        auto first = SimdTTM::lower_bound< const value_type*,
                                  value_type >( ref_.data(), ref_.data()+ref_.size(), key );

        auto it = ref_.begin();
        std::advance( it, first - ref_.data() );

        return (it!=ref_.end() && !(key<*it)) ? it : ref_.end();
    }
private:
    const container_type& ref_;
};

void do_nothing( int32_t );

template< typename TYPE_T > const char* getName() { return ""; }
#define GET_NAME(X) template<> const char* getName<X>(){ return #X; }

GET_NAME(int8_t)
GET_NAME(int16_t)
GET_NAME(int32_t)
GET_NAME(int64_t)
GET_NAME(uint8_t)
GET_NAME(uint16_t)
GET_NAME(uint32_t)
GET_NAME(uint64_t)
GET_NAME(float)
GET_NAME(double)

template< class Cont_T, class Index_T >
uint64_t bench( const std::string& name, size_t size, size_t loop, bool verbose )
{
	using container_type = Cont_T;
    using index_type = Index_T;
    using value_type = typename container_type::value_type;

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
    if( verbose )
        std::cout << "Find all " << getName<value_type>() << " " << name << ": " << timer.format();

    return timer.elapsed().wall;
}

float percent( uint64_t base, uint64_t compare )
{
    return 100.f * (static_cast<float>(base) / static_cast<float>(compare) -1.f);
}

template< typename NUM_T, size_t sz = SimdTTM::detail::simd::simd_size< NUM_T >() >
class benchSizes
{
public:
    std::vector< std::pair< size_t, uint64_t > >
    operator()( size_t runSize, size_t inLoop, bool verbose )
    {
        std::stringstream ss;
        ss << "SimdTTM::lower_bound (" << sz << ")";
        uint64_t ret = bench< avector< NUM_T >, SimdTTMLowerBound< avector< NUM_T >, sz > >
                        ( ss.str(), runSize, inLoop, verbose );

        std::vector< std::pair< size_t, uint64_t > > vec
           ;//= benchSizes< NUM_T, sz-1 >()( runSize, inLoop, verbose );

        vec.emplace_back( sz, ret );
        return vec;
    }
};

template< typename NUM_T >
class benchSizes< NUM_T, 1 >
{
public:
    std::vector< std::pair< size_t, uint64_t > >
    operator()( size_t runSize, size_t inLoop, bool verbose )
    {
        uint64_t ret = bench< avector< NUM_T >, SimdTTMLowerBound< avector< NUM_T >, 1 > >
                        ( "SimdTTM::lower_bound (1)", runSize, inLoop, verbose );

        std::vector< std::pair< size_t, uint64_t > > vec;
        vec.emplace_back( 1, ret );
        return vec;
    }
};


template< typename NUM_T, typename... NEXT_T >
class benchLoop
{
public:
    void operator()( bool verbose, size_t runSize, size_t inloop, size_t outloop )
    {
        benchLoop<NUM_T>()( verbose, runSize, inloop, outloop );
        benchLoop<NEXT_T...>()( verbose, runSize, inloop, outloop );
    }
};

template< typename NUM_T >
class benchLoop< NUM_T >
{
public:
    void operator()( bool verbose, size_t runSize, size_t inloop, size_t outloop )
    {
        if( verbose )
        {
            std::cout
                << std::endl << "size: 0x" << std::hex << std::setw(8) << std::setfill( '0') << runSize
                << std::endl << "type: " << getName<NUM_T>()
                << std::endl << std::endl;
        }
        else
        {
            std::cout
                << "size," << runSize << ",type," << getName<NUM_T>() << std::endl
                << "std::lower_bound,SimdTTM::lower_bound" << std::endl;
        }
        for( size_t i = 0; i < outloop; ++i )
        {
            uint64_t base = bench< avector< NUM_T >, StdLowerBound< avector< NUM_T > > >( "std::lower_bound ......", runSize, inloop, verbose );

            auto simd_times = benchSizes< NUM_T >()( runSize, inloop, verbose );

            if( verbose )
            {
                for( auto&& timer: simd_times )
                {
                    std::cout << std::dec
                        << "\nSimdTTM::lower_bound( " << timer.first << " ), "
                        << getName<NUM_T>() << " Speed up: "
                        << std::fixed << std::setprecision(2) << percent( base, timer.second ) << "%";
                }

                std::cout << std::endl << std::endl;
            }
            else
            {
                std::cout << base;
                for( auto&& timer: simd_times )
                    std::cout << "," << timer.second;
                std::cout << std::endl;
            }
        }
    }
};

int main(int argc, char* /*argv*/[])
{
    constexpr size_t runSize = 0x00400000;
    constexpr size_t loop = 5;
    bool verbose = (argc == 1);

#if defined(STTM_VC_ENABLED) // XXX Vc library bug on int8_t and int64_t
    benchLoop< int16_t, int32_t, float, double > benchmark;
#else
    benchLoop< int8_t, int16_t, int32_t, int64_t, float, double > benchmark;
#endif

    benchmark( verbose, runSize, loop, verbose? 1: 5 );

    return 0;
}
