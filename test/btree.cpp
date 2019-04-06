#include <VcAlgo/detail/btree.h>
#include "gtest/gtest.h"

void printRow( VcAlgo::detail::btree_row<int32_t>& row, std::pair<bool, int32_t>& up )
{
    std::cout << std::hex << row;
    if( up.first )
    {
        std::cout << " push up: " << up.second;
    }
    std::cout << std::endl;
}

TEST(BtreeRow, EmptyTest)
{
    VcAlgo::detail::btree_row<int32_t> row;

    auto up = row.insert( 0, 16 );
    printRow( row, up );

    up = row.insert( 0, 32 );
    printRow( row, up );

    up = row.insert( 0, 48 );
    printRow( row, up );

    up = row.insert( 0, 64 );
    printRow( row, up );
}




