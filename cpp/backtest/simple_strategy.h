#pragma once

#include "strategy.h"
#include <iostream>

namespace signalforge {

// Simplest possible strategy: buy once at the beginning
class SimpleStrategy : public Strategy {
public:
    void on_trade(Price trade_price, uint64_t timestamp) override {
        // Buy 1 BTC on the first trade only
        if (!has_bought_) {
            OrderIntent intent;
            intent.side = Side::BID;  // BUY
            intent.type = OrderType::MARKET;
            intent.qty = 1;  // 1 BTC
            intent.limit_price = 0;  // ignored for market orders

            OrderId id = exec_->submit(intent);
            has_bought_ = true;

            std::cout << "Submitted BUY order (ID=" << id << ") at price " << trade_price << std::endl;
        }
    }

    void on_fill(const Fill& fill) override {
        std::cout << "Fill received: "
                  << "ID=" << fill.order_id << " "
                  << (fill.side == Side::BID ? "BUY" : "SELL")
                  << " qty=" << fill.qty
                  << " price=" << fill.price
                  << std::endl;
    }

private:
    bool has_bought_ = false;
};

}  // namespace signalforge
