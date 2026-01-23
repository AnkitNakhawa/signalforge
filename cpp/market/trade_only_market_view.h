#pragma once
#include "cpp/interfaces/market_view.h"

namespace signalforge {

class TradeOnlyMarketView final : public MarketView {
public:
    void on_trade(Price p) { last_ = p; has_last_ = true; }

    bool has_top() const override { return has_last_; }
    Price best_bid() const override { return last_; } // approximation
    Price best_ask() const override { return last_; } // approximation

    bool has_last() const override { return has_last_; }
    Price last_price() const override { return last_; }

private:
    bool has_last_ = false;
    Price last_ = 0;
};

}
