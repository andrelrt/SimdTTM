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

void printBtree( VcAlgo::detail::btree<int32_t, 16>& btree )
{
    std::cout << std::hex << btree << std::endl;
}

TEST(Btree, BigTest)
{
    VcAlgo::detail::btree<int32_t, 16> btree;
    for( int32_t i = 0; i <= 0x10000; ++i )
    {
        btree.insert( i );
    }
    printBtree( btree );
}

TEST(Btree, EmptyTest)
{
    VcAlgo::detail::btree<int32_t, 16> btree;
    {
    std::cout << "Inserting -------------------------------------------------------------------\n"
              << "-----------------------------------------------------------------------------" << std::endl;
    btree.insert( 0x100 ); printBtree( btree );
    btree.insert( 0x200 ); printBtree( btree );
    btree.insert( 0x300 ); printBtree( btree );
    btree.insert( 0x400 ); printBtree( btree );

    btree.insert( 0x100 + 8 ); printBtree( btree );
    btree.insert( 0x200 + 8 ); printBtree( btree );
    btree.insert( 0x300 + 8 ); printBtree( btree );
    btree.insert( 0x400 + 8 ); printBtree( btree );

    btree.insert( 0x100 + 4 ); printBtree( btree );
    btree.insert( 0x200 + 4 ); printBtree( btree );
    btree.insert( 0x300 + 4 ); printBtree( btree );
    btree.insert( 0x400 + 4 ); printBtree( btree );

    btree.insert( 0x100 + 12 ); printBtree( btree );
    btree.insert( 0x200 + 12 ); printBtree( btree );
    btree.insert( 0x300 + 12 ); printBtree( btree );
    btree.insert( 0x400 + 12 ); printBtree( btree );

    btree.insert( 0x2ff ); printBtree( btree );

    btree.insert( 0x100 + 2 ); printBtree( btree );
    btree.insert( 0x200 + 2 ); printBtree( btree );
    btree.insert( 0x300 + 2 ); printBtree( btree );
    btree.insert( 0x400 + 2 ); printBtree( btree );

    btree.insert( 0x100 + 6 ); printBtree( btree );
    btree.insert( 0x200 + 6 ); printBtree( btree );
    btree.insert( 0x300 + 6 ); printBtree( btree );
    btree.insert( 0x400 + 6 ); printBtree( btree );

    btree.insert( 0x100 + 10 ); printBtree( btree );
    btree.insert( 0x200 + 10 ); printBtree( btree );
    btree.insert( 0x300 + 10 ); printBtree( btree );
    btree.insert( 0x400 + 10 ); printBtree( btree );

    btree.insert( 0x100 + 14 ); printBtree( btree );
    btree.insert( 0x200 + 14 ); printBtree( btree );
    btree.insert( 0x300 + 14 ); printBtree( btree );
    btree.insert( 0x400 + 14 ); printBtree( btree );

    btree.insert( 0x100 + 1 ); printBtree( btree );
    btree.insert( 0x200 + 1 ); printBtree( btree );
    btree.insert( 0x300 + 1 ); printBtree( btree );
    btree.insert( 0x400 + 1 ); printBtree( btree );

    btree.insert( 0x100 + 3 ); printBtree( btree );
    btree.insert( 0x200 + 3 ); printBtree( btree );
    btree.insert( 0x300 + 3 ); printBtree( btree );
    btree.insert( 0x400 + 3 ); printBtree( btree );

    btree.insert( 0x100 + 5 ); printBtree( btree );
    btree.insert( 0x200 + 5 ); printBtree( btree );
    btree.insert( 0x300 + 5 ); printBtree( btree );
    btree.insert( 0x400 + 5 ); printBtree( btree );

    btree.insert( 0x100 + 7 ); printBtree( btree );
    btree.insert( 0x200 + 7 ); printBtree( btree );
    btree.insert( 0x300 + 7 ); printBtree( btree );
    btree.insert( 0x400 + 7 ); printBtree( btree );

    btree.insert( 0x100 + 9 ); printBtree( btree );
    btree.insert( 0x200 + 9 ); printBtree( btree );
    btree.insert( 0x300 + 9 ); printBtree( btree );
    btree.insert( 0x400 + 9 ); printBtree( btree );

    btree.insert( 0x100 + 11 ); printBtree( btree );
    btree.insert( 0x200 + 11 ); printBtree( btree );
    btree.insert( 0x300 + 11 ); printBtree( btree );
    btree.insert( 0x400 + 11 ); printBtree( btree );

    btree.insert( 0x100 + 13 ); printBtree( btree );
    btree.insert( 0x200 + 13 ); printBtree( btree );
    btree.insert( 0x300 + 13 ); printBtree( btree );
    btree.insert( 0x400 + 13 ); printBtree( btree );

    btree.insert( 0x100 + 15 ); printBtree( btree );
    btree.insert( 0x200 + 15 ); printBtree( btree );
    btree.insert( 0x300 + 15 ); printBtree( btree );
    btree.insert( 0x400 + 15 ); printBtree( btree );

    }
}

TEST(BtreeRow, EmptyTest)
{
    VcAlgo::detail::btree_row<int32_t, 16> row;

    {
    std::cout << "Inserting -------------------------------------------------------------------\n"
              << "-----------------------------------------------------------------------------" << std::endl;
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
    up = row.insert( 1, 512 + 1 ); printRow( row, up );
    up = row.insert( 2, 768 + 1 ); printRow( row, up );
    up = row.insert( 3, 1024 + 1 ); printRow( row, up );

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

    up = row.insert( 1, 256 + 15 ); printRow( row, up );
    up = row.insert( 1, 512 + 15 ); printRow( row, up );
    up = row.insert( 4, 768 + 15 ); printRow( row, up );
    up = row.insert( 4, 1024 + 15 ); printRow( row, up );
    }


    {
    std::cout << "Removing -------------------------------------------------------------------\n"
              << "----------------------------------------------------------------------------" << std::endl;
    // Remove
    auto up = row.remove( 5, 1024 + 15, 0x407, 0 ); printRow( row, up );
    up = row.remove( 4, 768 + 15, 0, 0 ); printRow( row, up );
    up = row.remove( 2, 512 + 15, 0, 0 ); printRow( row, up );
    up = row.remove( 1, 256 + 15, 0, 0 ); printRow( row, up );

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

    up = row.remove( 3, 1024 + 1, 1024, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 1, 0x2ff, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 1, 0, 0x20e ); printRow( row, up );
    up = row.remove( 0, 256 + 1, 0, 0x10e ); printRow( row, up );

    up = row.remove( 2, 1024 + 14, 0x30e, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 14, 0x30c, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 14, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 14, 0, 0x200 ); printRow( row, up );

    up = row.remove( 2, 1024 + 10, 0x30a, 0 ); printRow( row, up );
    up = row.remove( 2, 768 + 10, 0x308, 0 ); printRow( row, up );
    up = row.remove( 1, 512 + 10, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 10, 0, 0x202 ); printRow( row, up );

    up = row.remove( 1, 1024 + 6, 0x306, 0 ); printRow( row, up );
    up = row.remove( 1, 768 + 6, 0x304, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 6, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 6, 0, 0 ); printRow( row, up );

    up = row.remove( 1, 1024 + 2, 0x302, 0 ); printRow( row, up );
    up = row.remove( 1, 768 + 2, 0x300, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 2, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 2, 0, 0 ); printRow( row, up );

    up = row.remove( 1, 1024 + 12, 0x2ff, 0 ); printRow( row, up );
    up = row.remove( 0, 768 + 12, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 512 + 12, 0, 0 ); printRow( row, up );
    up = row.remove( 0, 256 + 12, 0, 0 ); printRow( row, up );

    up = row.remove( 0, 0x2ff, 0, 0 ); printRow( row, up );

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




