#include "trade_through_execution.h"
#include "cpp/market/trade_only_market_view.h"
#include <gtest/gtest.h>
#include <iostream>

namespace signalforge {

class TradeOnlyMarketViewTest : public ::testing::Test {
protected:
    TradeOnlyMarketView view;
};

// Test initial state
TEST_F(TradeOnlyMarketViewTest, InitialState) {
    EXPECT_FALSE(view.has_top());
    EXPECT_FALSE(view.has_last());
}

// Test trade updates
TEST_F(TradeOnlyMarketViewTest, OnTrade) {
    view.on_trade(100);

    EXPECT_TRUE(view.has_top());
    EXPECT_TRUE(view.has_last());
    EXPECT_EQ(view.last_price(), 100);
    EXPECT_EQ(view.best_bid(), 100);
    EXPECT_EQ(view.best_ask(), 100);
}

// Test multiple trades
TEST_F(TradeOnlyMarketViewTest, MultipleTrades) {
    view.on_trade(100);
    view.on_trade(105);
    view.on_trade(95);

    // Should have the last trade price
    EXPECT_EQ(view.last_price(), 95);
    EXPECT_EQ(view.best_bid(), 95);
    EXPECT_EQ(view.best_ask(), 95);
}

// Test trade price updates
TEST_F(TradeOnlyMarketViewTest, TradeUpdatesPrice) {
    view.on_trade(100);
    EXPECT_EQ(view.last_price(), 100);

    view.on_trade(200);
    EXPECT_EQ(view.last_price(), 200);
    EXPECT_EQ(view.best_bid(), 200);
    EXPECT_EQ(view.best_ask(), 200);
}


class TradeThroughExecutionTest : public ::testing::Test {
protected:
    TradeOnlyMarketView view;
    TradeThroughExecution exec{view};
};

// Test initial state
TEST_F(TradeThroughExecutionTest, InitialState) {
    Fill fill;
    EXPECT_FALSE(exec.poll_fill(fill));
}

// Test market order execution
TEST_F(TradeThroughExecutionTest, MarketOrderBuy) {
    // Submit market buy order
    OrderIntent intent{Side::BID, OrderType::MARKET, 0, 10};
    OrderId id = exec.submit(intent);

    EXPECT_EQ(id, 1);

    // No fill yet - no trade has occurred
    Fill fill;
    EXPECT_FALSE(exec.poll_fill(fill));

    // Trade occurs at 100
    view.on_trade(100);
    exec.on_tick();

    // Should have a fill now
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id);
    EXPECT_EQ(fill.side, Side::BID);
    EXPECT_EQ(fill.price, 100);
    EXPECT_EQ(fill.qty, 10);

    // No more fills
    EXPECT_FALSE(exec.poll_fill(fill));
}

TEST_F(TradeThroughExecutionTest, MarketOrderSell) {
    // Submit market sell order
    OrderIntent intent{Side::ASK, OrderType::MARKET, 0, 5};
    OrderId id = exec.submit(intent);

    // Trade occurs at 200
    view.on_trade(200);
    exec.on_tick();

    // Should have a fill
    Fill fill;
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id);
    EXPECT_EQ(fill.side, Side::ASK);
    EXPECT_EQ(fill.price, 200);
    EXPECT_EQ(fill.qty, 5);
}

// Test limit order execution - buy side
TEST_F(TradeThroughExecutionTest, LimitOrderBuyFills) {
    // Submit limit buy at 100
    OrderIntent intent{Side::BID, OrderType::LIMIT, 100, 10};
    OrderId id = exec.submit(intent);

    // Trade occurs at 100 - should fill
    view.on_trade(100);
    exec.on_tick();

    Fill fill;
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id);
    EXPECT_EQ(fill.price, 100);
}

TEST_F(TradeThroughExecutionTest, LimitOrderBuyTradeThrough) {
    // Submit limit buy at 100
    OrderIntent intent{Side::BID, OrderType::LIMIT, 100, 10};
    OrderId id = exec.submit(intent);

    // Trade occurs at 95 - should fill (trade through)
    view.on_trade(95);
    exec.on_tick();

    Fill fill;
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id);
    EXPECT_EQ(fill.price, 95);
}

TEST_F(TradeThroughExecutionTest, LimitOrderBuyNoFill) {
    // Submit limit buy at 100
    OrderIntent intent{Side::BID, OrderType::LIMIT, 100, 10};
    exec.submit(intent);

    // Trade occurs at 105 - should NOT fill
    view.on_trade(105);
    exec.on_tick();

    Fill fill;
    EXPECT_FALSE(exec.poll_fill(fill));
}

// Test limit order execution - sell side
TEST_F(TradeThroughExecutionTest, LimitOrderSellFills) {
    // Submit limit sell at 100
    OrderIntent intent{Side::ASK, OrderType::LIMIT, 100, 10};
    OrderId id = exec.submit(intent);

    // Trade occurs at 100 - should fill
    view.on_trade(100);
    exec.on_tick();

    Fill fill;
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id);
    EXPECT_EQ(fill.price, 100);
}

TEST_F(TradeThroughExecutionTest, LimitOrderSellTradeThrough) {
    // Submit limit sell at 100
    OrderIntent intent{Side::ASK, OrderType::LIMIT, 100, 10};
    OrderId id = exec.submit(intent);

    // Trade occurs at 105 - should fill (trade through)
    view.on_trade(105);
    exec.on_tick();

    Fill fill;
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id);
    EXPECT_EQ(fill.price, 105);
}

TEST_F(TradeThroughExecutionTest, LimitOrderSellNoFill) {
    // Submit limit sell at 100
    OrderIntent intent{Side::ASK, OrderType::LIMIT, 100, 10};
    exec.submit(intent);

    // Trade occurs at 95 - should NOT fill
    view.on_trade(95);
    exec.on_tick();

    Fill fill;
    EXPECT_FALSE(exec.poll_fill(fill));
}

// Test multiple orders
TEST_F(TradeThroughExecutionTest, MultipleOrders) {
    OrderId id1 = exec.submit({Side::BID, OrderType::LIMIT, 100, 10});
    OrderId id2 = exec.submit({Side::BID, OrderType::LIMIT, 105, 5});
    OrderId id3 = exec.submit({Side::ASK, OrderType::LIMIT, 105, 8});

    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);

    // Trade at 100 - should fill first and second order
    view.on_trade(100);
    exec.on_tick();

    Fill fill;
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id1);

    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id2);

    // Third order should not fill (sell at 105, trade at 100)
    EXPECT_FALSE(exec.poll_fill(fill));
}

// Test fill queue ordering
TEST_F(TradeThroughExecutionTest, FillQueueOrdering) {
    OrderId id1 = exec.submit({Side::BID, OrderType::MARKET, 0, 10});
    OrderId id2 = exec.submit({Side::BID, OrderType::MARKET, 0, 20});
    OrderId id3 = exec.submit({Side::BID, OrderType::MARKET, 0, 30});

    view.on_trade(100);
    exec.on_tick();

    // Fills should come in order
    Fill fill;
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id1);
    EXPECT_EQ(fill.qty, 10);

    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id2);
    EXPECT_EQ(fill.qty, 20);

    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, id3);
    EXPECT_EQ(fill.qty, 30);

    EXPECT_FALSE(exec.poll_fill(fill));
}

// Test no execution without trade
TEST_F(TradeThroughExecutionTest, NoExecutionWithoutTrade) {
    exec.submit({Side::BID, OrderType::MARKET, 0, 10});
    exec.submit({Side::BID, OrderType::LIMIT, 100, 5});

    // Call on_tick without any trade
    exec.on_tick();

    Fill fill;
    EXPECT_FALSE(exec.poll_fill(fill));
}

// Test realistic scenario
TEST_F(TradeThroughExecutionTest, RealisticScenario) {
    // Submit various orders
    OrderId market_buy = exec.submit({Side::BID, OrderType::MARKET, 0, 10});
    OrderId limit_buy = exec.submit({Side::BID, OrderType::LIMIT, 99, 5});
    OrderId limit_sell = exec.submit({Side::ASK, OrderType::LIMIT, 101, 8});

    // Trade at 100
    view.on_trade(100);
    exec.on_tick();

    // Market buy should fill
    Fill fill;
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, market_buy);
    EXPECT_EQ(fill.price, 100);

    // Limit buy should fill (100 <= 99 is false, so no fill)
    // Actually, limit buy at 99 should NOT fill at 100
    EXPECT_FALSE(exec.poll_fill(fill));

    // Trade at 98
    view.on_trade(98);
    exec.on_tick();

    // Now limit buy should fill
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, limit_buy);
    EXPECT_EQ(fill.price, 98);

    // Trade at 102
    view.on_trade(102);
    exec.on_tick();

    // Now limit sell should fill
    EXPECT_TRUE(exec.poll_fill(fill));
    EXPECT_EQ(fill.order_id, limit_sell);
    EXPECT_EQ(fill.price, 102);

    EXPECT_FALSE(exec.poll_fill(fill));
}

}  // namespace signalforge
