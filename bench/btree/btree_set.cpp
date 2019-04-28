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
#include <set>
#include <vector>
#include <boost/timer/timer.hpp>
#include <boost/align/aligned_allocator.hpp> 

#include <btree/btree_set.h>
#include <SimdTTM/algorithm.h>

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

template< class Cont_T >
uint64_t bench( const std::string& name, size_t size, size_t loop, bool verbose )
{
	using container_type = Cont_T;
    using value_type = typename container_type::value_type;

    boost::timer::cpu_timer timer;

    std::vector< value_type > randData;
    srand( 1 );
    std::generate_n( std::back_inserter(randData), size, &rand );

    container_type testSet;

    timer.start();
    for( value_type val : randData )
        testSet.insert( val );
    timer.stop();

    if( verbose )
        std::cout << "Insert all " << getName<value_type>() << " " << name << ": " << timer.format();

    return timer.elapsed().wall;
}

float percent( uint64_t base, uint64_t compare )
{
    return 100.f * (static_cast<float>(base) / static_cast<float>(compare) -1.f);
}

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
                << "std::set,SimdTTM::btree" << std::endl;
        }
        for( size_t i = 0; i < outloop; ++i )
        {
            uint64_t base = bench< std::set< NUM_T > >( "std::set ............", runSize, inloop, verbose );
            uint64_t cppbtree = bench< btree::btree_set< NUM_T > >( "btree::btree_set ....", runSize, inloop, verbose );

            uint64_t btree128 = bench< SimdTTM::btree< NUM_T, 128 > >( "SimdTTM::btree< 128 >", runSize, inloop, verbose );
            uint64_t btree256 = bench< SimdTTM::btree< NUM_T, 256 > >( "SimdTTM::btree< 256 >", runSize, inloop, verbose );
            uint64_t btree512 = bench< SimdTTM::btree< NUM_T, 512 > >( "SimdTTM::btree< 512 >", runSize, inloop, verbose );
            uint64_t btree1024 = bench< SimdTTM::btree< NUM_T, 1024 > >( "SimdTTM::btree< 1024 >", runSize, inloop, verbose );
            uint64_t btree2048 = bench< SimdTTM::btree< NUM_T, 2048 > >( "SimdTTM::btree< 2048 >", runSize, inloop, verbose );

            if( verbose )
            {
                std::cout
                    << std::endl << "btree::btree_set,     " << getName<NUM_T>() << " Speed up: "
                    << std::fixed << std::setprecision(2) << percent( base, cppbtree ) << "%"

                    << std::endl << "SimdTTM::btree<128> , " << getName<NUM_T>() << " Speed up: "
                    << std::fixed << std::setprecision(2) << percent( base, btree128 ) << "%"

                    << std::endl << "SimdTTM::btree<256>,  " << getName<NUM_T>() << " Speed up: "
                    << std::fixed << std::setprecision(2) << percent( base, btree256 ) << "%"

                    << std::endl << "SimdTTM::btree<512>,  " << getName<NUM_T>() << " Speed up: "
                    << std::fixed << std::setprecision(2) << percent( base, btree512 ) << "%"

                    << std::endl << "SimdTTM::btree<1024>, " << getName<NUM_T>() << " Speed up: "
                    << std::fixed << std::setprecision(2) << percent( base, btree1024 ) << "%"

                    << std::endl << "SimdTTM::btree<2048>, " << getName<NUM_T>() << " Speed up: "
                    << std::fixed << std::setprecision(2) << percent( base, btree2048 ) << "%"

                    << std::endl << "SimdTTM vs cppbtree,  " << getName<NUM_T>() << " Speed up: "
                    << std::fixed << std::setprecision(2) << percent( cppbtree, btree256 ) << "%"

                    << std::endl << std::endl;
            }
            else
            {
                std::cout
                    << base
                    << "," << btree128
                    << std::endl;
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
