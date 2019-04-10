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

void printRow( VcAlgo::detail::btree_row<int32_t, 16>& row,
               std::pair<VcAlgo::detail::RemoveMode, int32_t>& up )
{
    std::cout << std::hex << row;

    switch( up.first )
    {
        case VcAlgo::detail::RemoveMode::NotFound:
            std::cout << " - not found";
            break;

        case VcAlgo::detail::RemoveMode::NodeOnly:
            break;

        case VcAlgo::detail::RemoveMode::NodeOnlyLessThanMin:
            std::cout << " - less than min (should be root)";
            break;

        case VcAlgo::detail::RemoveMode::ShiftRight:
            std::cout << " - shift right, push up: " << up.second;
            break;

        case VcAlgo::detail::RemoveMode::ShiftLeft:
            std::cout << " - shift left, push up: " << up.second;
            break;

        case VcAlgo::detail::RemoveMode::MergeRight:
            std::cout << " - merge right";
            break;

        case VcAlgo::detail::RemoveMode::MergeLeft:
            std::cout << " - merge left";
            break;
    }
    std::cout << std::endl << std::endl;
}

TEST(BtreeRow, EmptyTest)
{
    VcAlgo::detail::btree_row<int32_t, 16> row;

    {
    std::cout << "Inserting -------------------------------------------------------------------" << std::endl;
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

    up = row.insert( 0, 0x2ff ); printRow( row, up );

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
    up = row.insert( 2, 1024 + 1 ); printRow( row, up );

    up = row.insert( 0, 256 + 3 ); printRow( row, up );
    up = row.insert( 1, 512 + 3 ); printRow( row, up );
    up = row.insert( 2, 768 + 3 ); printRow( row, up );
    up = row.insert( 3, 1024 + 3 ); printRow( row, up );

    up = row.insert( 0, 256 + 5 ); printRow( row, up );
    up = row.insert( 1, 512 + 5 ); printRow( row, up );
    up = row.insert( 2, 768 + 5 ); printRow( row, up );
    up = row.insert( 3, 1024 + 5 ); printRow( row, up );

    up = row.insert( 0, 256 + 7 ); printRow( row, up );
    up = row.insert( 1, 512 + 7 ); printRow( row, up );
    up = row.insert( 2, 768 + 7 ); printRow( row, up );
    up = row.insert( 3, 1024 + 7 ); printRow( row, up );

    up = row.insert( 0, 256 + 9 ); printRow( row, up );
    up = row.insert( 1, 512 + 9 ); printRow( row, up );
    up = row.insert( 2, 768 + 9 ); printRow( row, up );
    up = row.insert( 3, 1024 + 9 ); printRow( row, up );

    up = row.insert( 0, 256 + 11 ); printRow( row, up );
    up = row.insert( 1, 512 + 11 ); printRow( row, up );
    up = row.insert( 2, 768 + 11 ); printRow( row, up );
    up = row.insert( 3, 1024 + 11 ); printRow( row, up );

    up = row.insert( 0, 256 + 13 ); printRow( row, up );
    up = row.insert( 1, 512 + 13 ); printRow( row, up );
    up = row.insert( 2, 768 + 13 ); printRow( row, up );
    up = row.insert( 3, 1024 + 13 ); printRow( row, up );

    up = row.insert( 0, 256 + 15 ); printRow( row, up );
    up = row.insert( 1, 512 + 15 ); printRow( row, up );
    up = row.insert( 2, 768 + 15 ); printRow( row, up );
    up = row.insert( 3, 1024 + 15 ); printRow( row, up );
    }


    {
    std::cout << "Removing -------------------------------------------------------------------" << std::endl;
    // Remove
    auto up = row.remove( 3, 1024 + 15, 0, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 15, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 15, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 15, 0, 0 ); printRow( row, up );

    up = row.remove( 3, 1024 + 13, 0, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 13, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 13, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 13, 0, 0 ); printRow( row, up );

    up = row.remove( 3, 1024 + 11, 0, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 11, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 11, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 11, 0, 0 ); printRow( row, up );

    up = row.remove( 3, 1024 + 9, 0, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 9, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 9, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 9, 0, 0 ); printRow( row, up );

    up = row.remove( 3, 1024 + 7, 0, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 7, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 7, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 7, 0, 0 ); printRow( row, up );

    up = row.remove( 3, 1024 + 5, 0, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 5, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 5, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 5, 0, 0 ); printRow( row, up );

    up = row.remove( 3, 1024 + 3, 0, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 3, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 3, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 3, 0, 0 ); printRow( row, up );

    up = row.remove( 2, 1024 + 1, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 768 + 1, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 1, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 1, 0, 0 ); printRow( row, up );

    up = row.remove( 1, 1024 + 14, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 768 + 14, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 14, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 14, 0, 0 ); printRow( row, up );

    up = row.remove( 1, 1024 + 10, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 768 + 10, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 10, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 10, 0, 0 ); printRow( row, up );

    up = row.remove( 1, 1024 + 6, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 768 + 6, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 6, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 6, 0, 0 ); printRow( row, up );

    up = row.remove( 1, 1024 + 2, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 768 + 2, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 2, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 2, 0, 0 ); printRow( row, up );

    up = row.remove( 0, 0x2ff, 0, 0 ); printRow( row, up );

    up = row.remove( 0, 1024 + 12, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 768 + 12, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 12, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 12, 0, 0 ); printRow( row, up );

    up = row.remove( 0, 1024 + 4, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 768 + 4, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 4, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 4, 0, 0 ); printRow( row, up );

    up = row.remove( 0, 1024 + 8, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 768 + 8, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 8, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 8, 0, 0 ); printRow( row, up );

    up = row.remove( 0, 1024, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 768, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256, 0, 0 ); printRow( row, up );
    }
}




