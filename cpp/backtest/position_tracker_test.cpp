#include "position_tracker.h"
#include <gtest/gtest.h>

namespace signalforge {

TEST(PositionTrackerTest, InitialState) {
    PositionTracker pt;

    EXPECT_EQ(pt.position(), 0);
    EXPECT_DOUBLE_EQ(pt.realized_pnl(), 0.0);
    EXPECT_DOUBLE_EQ(pt.unrealized_pnl(4250000), 0.0);
}

TEST(PositionTrackerTest, OpenLongPosition) {
    PositionTracker pt;

    // Buy 1 BTC at $42,500
    Fill fill{1, Side::BID, 4250000, 1};
    pt.on_fill(fill);

    EXPECT_EQ(pt.position(), 1);
    EXPECT_EQ(pt.avg_entry_price(), 4250000);
    EXPECT_DOUBLE_EQ(pt.realized_pnl(), 0.0);
}

TEST(PositionTrackerTest, UnrealizedPnLLong) {
    PositionTracker pt;

    // Buy 1 BTC at $42,500
    Fill fill{1, Side::BID, 4250000, 1};
    pt.on_fill(fill);

    // Price goes to $43,000 (+$500 profit)
    EXPECT_DOUBLE_EQ(pt.unrealized_pnl(4300000), 500.0);

    // Price goes to $42,000 (-$500 loss)
    EXPECT_DOUBLE_EQ(pt.unrealized_pnl(4200000), -500.0);
}

TEST(PositionTrackerTest, CloseLongPosition) {
    PositionTracker pt;

    // Buy 1 BTC at $42,500
    pt.on_fill({1, Side::BID, 4250000, 1});

    // Sell 1 BTC at $43,000 (+$500 profit)
    pt.on_fill({2, Side::ASK, 4300000, 1});

    EXPECT_EQ(pt.position(), 0);
    EXPECT_DOUBLE_EQ(pt.realized_pnl(), 500.0);
    EXPECT_DOUBLE_EQ(pt.unrealized_pnl(4300000), 0.0);
}

TEST(PositionTrackerTest, PartialClose) {
    PositionTracker pt;

    // Buy 2 BTC at $42,500
    pt.on_fill({1, Side::BID, 4250000, 2});
    EXPECT_EQ(pt.position(), 2);

    // Sell 1 BTC at $43,000 (close half)
    pt.on_fill({2, Side::ASK, 4300000, 1});

    EXPECT_EQ(pt.position(), 1);  // 1 BTC left
    EXPECT_DOUBLE_EQ(pt.realized_pnl(), 500.0);  // $500 from closing 1 BTC
    EXPECT_DOUBLE_EQ(pt.unrealized_pnl(4300000), 500.0);  // $500 on remaining 1 BTC
}

TEST(PositionTrackerTest, AverageEntryPrice) {
    PositionTracker pt;

    // Buy 1 BTC at $42,000
    pt.on_fill({1, Side::BID, 4200000, 1});

    // Buy 1 BTC at $44,000
    pt.on_fill({2, Side::BID, 4400000, 1});

    // Average should be $43,000
    EXPECT_EQ(pt.avg_entry_price(), 4300000);
    EXPECT_EQ(pt.position(), 2);
}

}  // namespace signalforge
