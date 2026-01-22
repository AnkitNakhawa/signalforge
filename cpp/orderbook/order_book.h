#pragma once
#include <cstdint>
#include <map>

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

    // snapshot semantics
    void clear();
    void set_level(Side side, Price price, Quantity qty);

    // delta semantics
    void add_level(Side side, Price price, Quantity qty);
    void remove_level(Side side, Price price, Quantity qty);

    // query methods
    Price best_bid() const;
    Price best_ask() const;

    Quantity level_qty(Side side, Price price) const;

private:
    void update_best_levels();

    std::map<Price, Quantity, std::greater<Price>> bids_;
    std::map<Price, Quantity, std::less<Price>> asks_;

    Price best_bid_;
    Price best_ask_;
};

} // namespace signalforge
