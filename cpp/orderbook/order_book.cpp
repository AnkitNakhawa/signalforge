#include "order_book.h"
#include <limits>

namespace signalforge {

OrderBook::OrderBook()
    : best_bid_(std::numeric_limits<Price>::min()),
      best_ask_(std::numeric_limits<Price>::max()) {}

void OrderBook::set_level(Side side, Price price, Quantity qty) {
    if (qty <= 0) return;

    if (side == Side::BID && price > best_bid_) {
        best_bid_ = price;
    } else if (side == Side::ASK && price < best_ask_) {
        best_ask_ = price;
    }
}

Price OrderBook::best_bid() const {
    return best_bid_;
}

Price OrderBook::best_ask() const {
    return best_ask_;
}

} 
