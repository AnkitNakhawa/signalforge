#pragma once
#include <cstdint>

namespace signalforge {

enum class Side : uint8_t {
    BID,
    ASK
};

using Price = int64_t;     // fixed-point ticks
using Quantity = int64_t;  // fixed-point units

class OrderBook {
public:
    OrderBook();

    void set_level(Side side, Price price, Quantity qty);

    Price best_bid() const;
    Price best_ask() const;

private:
    Price best_bid_;
    Price best_ask_;
};

} // namespace signalforge
