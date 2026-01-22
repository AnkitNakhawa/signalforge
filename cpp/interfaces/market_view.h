#pragma once
#include "cpp/orderbook/order_book.h" // for Price, Quantity, Side

namespace signalforge {

class MarketView {
public:
    virtual ~MarketView() = default;

    virtual bool has_top() const = 0;      // best bid/ask available
    virtual Price best_bid() const = 0;
    virtual Price best_ask() const = 0;
    virtual bool has_last() const = 0;     // last trade available
    virtual Price last_price() const = 0;
};

}
