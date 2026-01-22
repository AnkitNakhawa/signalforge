#include "order_book.h"
#include <gtest/gtest.h>
#include <limits>

namespace signalforge {

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book;
};

// Test initial state
TEST_F(OrderBookTest, InitialState) {
    EXPECT_EQ(book.best_bid(), 0);
    EXPECT_EQ(book.best_ask(), 0);
}

// Test setting bid levels
TEST_F(OrderBookTest, SetBidLevel) {
    book.set_level(Side::BID, 100, 10);
    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(OrderBookTest, SetMultipleBidLevels) {
    book.set_level(Side::BID, 100, 10);
    book.set_level(Side::BID, 105, 5);
    book.set_level(Side::BID, 95, 15);

    // Best bid should be highest price (105)
    EXPECT_EQ(book.best_bid(), 105);
}

// Test setting ask levels
TEST_F(OrderBookTest, SetAskLevel) {
    book.set_level(Side::ASK, 110, 5);
    EXPECT_EQ(book.best_ask(), 110);
}

TEST_F(OrderBookTest, SetMultipleAskLevels) {
    book.set_level(Side::ASK, 110, 5);
    book.set_level(Side::ASK, 115, 10);
    book.set_level(Side::ASK, 105, 8);

    // Best ask should be lowest price (105)
    EXPECT_EQ(book.best_ask(), 105);
}

// Test removing levels (qty <= 0)
TEST_F(OrderBookTest, RemoveLevelWithZeroQuantity) {
    book.set_level(Side::BID, 100, 10);
    book.set_level(Side::BID, 95, 5);

    EXPECT_EQ(book.best_bid(), 100);

    // Remove the best bid
    book.set_level(Side::BID, 100, 0);

    // Best bid should now be 95
    EXPECT_EQ(book.best_bid(), 95);
}

TEST_F(OrderBookTest, RemoveAllLevels) {
    book.set_level(Side::BID, 100, 10);
    book.set_level(Side::ASK, 110, 5);

    book.set_level(Side::BID, 100, 0);
    book.set_level(Side::ASK, 110, 0);

    EXPECT_EQ(book.best_bid(), 0);
    EXPECT_EQ(book.best_ask(), 0);
}

// Test clear functionality
TEST_F(OrderBookTest, Clear) {
    book.set_level(Side::BID, 100, 10);
    book.set_level(Side::ASK, 110, 5);

    book.clear();

    EXPECT_EQ(book.best_bid(), 0);
    EXPECT_EQ(book.best_ask(), 0);
}

// Test add_level (delta semantics)
TEST_F(OrderBookTest, AddLevel) {
    book.add_level(Side::BID, 100, 10);
    book.add_level(Side::BID, 100, 5);

    // Quantity should accumulate
    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(OrderBookTest, AddLevelIgnoresNegative) {
    book.set_level(Side::BID, 100, 10);

    book.add_level(Side::BID, 100, -5);

    EXPECT_EQ(book.best_bid(), 100);
}

// Test remove_level (delta semantics)
TEST_F(OrderBookTest, RemoveLevel) {
    book.set_level(Side::BID, 100, 20);

    book.remove_level(Side::BID, 100, 5);

    // Should still have the level at 100
    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(OrderBookTest, RemoveLevelCompletely) {
    book.set_level(Side::BID, 100, 10);
    book.set_level(Side::BID, 95, 5);

    // Remove all quantity at 100
    book.remove_level(Side::BID, 100, 10);

    // Best bid should drop to 95
    EXPECT_EQ(book.best_bid(), 95);
}

TEST_F(OrderBookTest, RemoveLevelOverQuantity) {
    book.set_level(Side::BID, 100, 10);
    book.set_level(Side::BID, 95, 5);

    // Remove more than available
    book.remove_level(Side::BID, 100, 20);

    // Level should be removed, best bid should be 95
    EXPECT_EQ(book.best_bid(), 95);
}

// Test realistic order book scenario
TEST_F(OrderBookTest, RealisticScenario) {
    // Build a book with spread
    book.set_level(Side::BID, 100, 10);
    book.set_level(Side::BID, 99, 20);
    book.set_level(Side::BID, 98, 15);

    book.set_level(Side::ASK, 101, 5);
    book.set_level(Side::ASK, 102, 10);
    book.set_level(Side::ASK, 103, 8);

    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_EQ(book.best_ask(), 101);

    // Spread should be 1 tick
    EXPECT_EQ(book.best_ask() - book.best_bid(), 1);
}

}  // namespace signalforge
