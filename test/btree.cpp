#include <VcAlgo/detail/btree.h>
#include "gtest/gtest.h"

void printRow( VcAlgo::detail::btree_row<int32_t, 16>& row, std::pair<bool, int32_t>& up )
{
    std::cout << std::hex << row;
    if( up.first )
    {
        std::cout << " push up: " << up.second;
    }
    std::cout << std::endl << std::endl;
}

TEST(BtreeRow, EmptyTest)
{
    VcAlgo::detail::btree_row<int32_t, 16> row;

    auto up = row.insert( 0, 256 ); printRow( row, up );
    up = row.insert( 0, 512 ); printRow( row, up );
    up = row.insert( 0, 768 ); printRow( row, up );
    up = row.insert( 0, 1024 ); printRow( row, up );

    up = row.insert( 0, 256 + 8 ); printRow( row, up );
    up = row.insert( 0, 512 + 8 ); printRow( row, up );
    up = row.insert( 0, 768 + 8 ); printRow( row, up );
    up = row.insert( 0, 1024 + 8 ); printRow( row, up );

    up = row.insert( 0, 256 + 4 ); printRow( row, up );
    up = row.insert( 0, 512 + 4 ); printRow( row, up );
    up = row.insert( 0, 768 + 4 ); printRow( row, up );
    up = row.insert( 0, 1024 + 4 ); printRow( row, up );

    up = row.insert( 0, 256 + 12 ); printRow( row, up );
    up = row.insert( 0, 512 + 12 ); printRow( row, up );
    up = row.insert( 0, 768 + 12 ); printRow( row, up );
    up = row.insert( 0, 1024 + 12 ); printRow( row, up );

    up = row.insert( 0, 256 + 2 ); printRow( row, up );
    up = row.insert( 0, 512 + 2 ); printRow( row, up );
    up = row.insert( 1, 768 + 2 ); printRow( row, up );
    up = row.insert( 1, 1024 + 2 ); printRow( row, up );

    up = row.insert( 0, 256 + 6 ); printRow( row, up );
    up = row.insert( 0, 512 + 6 ); printRow( row, up );
    up = row.insert( 1, 768 + 6 ); printRow( row, up );
    up = row.insert( 1, 1024 + 6 ); printRow( row, up );

    up = row.insert( 0, 256 + 10 ); printRow( row, up );
    up = row.insert( 0, 512 + 10 ); printRow( row, up );
    up = row.insert( 1, 768 + 10 ); printRow( row, up );
    up = row.insert( 1, 1024 + 10 ); printRow( row, up );

    up = row.insert( 0, 256 + 14 ); printRow( row, up );
    up = row.insert( 0, 512 + 14 ); printRow( row, up );
    up = row.insert( 1, 768 + 14 ); printRow( row, up );
    up = row.insert( 1, 1024 + 14 ); printRow( row, up );

    up = row.insert( 0, 256 + 1 ); printRow( row, up );
    up = row.insert( 0, 512 + 1 ); printRow( row, up );
    up = row.insert( 1, 768 + 1 ); printRow( row, up );
    up = row.insert( 3, 1024 + 1 ); printRow( row, up );

    up = row.insert( 0, 256 + 3 ); printRow( row, up );
    up = row.insert( 2, 512 + 3 ); printRow( row, up );
    up = row.insert( 1, 768 + 3 ); printRow( row, up );
    up = row.insert( 3, 1024 + 3 ); printRow( row, up );

    up = row.insert( 0, 256 + 5 ); printRow( row, up );
    up = row.insert( 2, 512 + 5 ); printRow( row, up );
    up = row.insert( 1, 768 + 5 ); printRow( row, up );
    up = row.insert( 3, 1024 + 5 ); printRow( row, up );

    up = row.insert( 0, 256 + 7 ); printRow( row, up );
    up = row.insert( 2, 512 + 7 ); printRow( row, up );
    up = row.insert( 1, 768 + 7 ); printRow( row, up );
    up = row.insert( 3, 1024 + 7 ); printRow( row, up );

    up = row.insert( 0, 256 + 9 ); printRow( row, up );
    up = row.insert( 2, 512 + 9 ); printRow( row, up );
    up = row.insert( 1, 768 + 9 ); printRow( row, up );
    up = row.insert( 3, 1024 + 9 ); printRow( row, up );

    up = row.insert( 0, 256 + 11 ); printRow( row, up );
    up = row.insert( 2, 512 + 11 ); printRow( row, up );
    up = row.insert( 1, 768 + 11 ); printRow( row, up );
    up = row.insert( 3, 1024 + 11 ); printRow( row, up );

    up = row.insert( 0, 256 + 13 ); printRow( row, up );
    up = row.insert( 2, 512 + 13 ); printRow( row, up );
    up = row.insert( 1, 768 + 13 ); printRow( row, up );
    up = row.insert( 3, 1024 + 13 ); printRow( row, up );

    up = row.insert( 0, 256 + 15 ); printRow( row, up );
    up = row.insert( 2, 512 + 15 ); printRow( row, up );
    up = row.insert( 1, 768 + 15 ); printRow( row, up );
    up = row.insert( 3, 1024 + 15 ); printRow( row, up );
}




