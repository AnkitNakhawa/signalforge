#include "order_book.h"

namespace signalforge {

OrderBook::OrderBook()
    : best_bid_(0),
      best_ask_(0) {}


void OrderBook::clear() {
    bids_.clear();
    asks_.clear();
    best_bid_ = 0;
    best_ask_ = 0;
}

void OrderBook::set_level(Side side, Price price, Quantity qty) {
    if (side == Side::BID) {
        if (qty <= 0) {
            bids_.erase(price);
        } else {
            bids_[price] = qty;
        }
    } else {
        if (qty <= 0) {
            asks_.erase(price);
        } else {
            asks_[price] = qty;
        }
    }

    update_best_levels();
}

void OrderBook::add_level(Side side, Price price, Quantity delta) {
    if (delta <= 0) return;

    if (side == Side::BID) {
        bids_[price] += delta;
    } else {
        asks_[price] += delta;
    }

    update_best_levels();
}

void OrderBook::remove_level(Side side, Price price, Quantity delta) {
    if (delta <= 0) return;

    if (side == Side::BID) {
        auto it = bids_.find(price);
        if (it != bids_.end()) {
            it->second -= delta;
            if (it->second <= 0) {
                bids_.erase(it);
            }
        }
    } else {
        auto it = asks_.find(price);
        if (it != asks_.end()) {
            it->second -= delta;
            if (it->second <= 0) {
                asks_.erase(it);
            }
        }
    }

    update_best_levels();
}

void OrderBook::update_best_levels() {
    best_bid_ = bids_.empty() ? 0 : bids_.begin()->first;
    best_ask_ = asks_.empty() ? 0 : asks_.begin()->first;
}

Price OrderBook::best_bid() const {
    return best_bid_;
}

Price OrderBook::best_ask() const {
    return best_ask_;
}

} 
