#pragma once
#include <cstdint>
#include "cpp/orderbook/order_book.h"

namespace signalforge {

using OrderId = uint64_t;

enum class OrderType { MARKET, LIMIT };

struct OrderIntent {
    Side side;
    OrderType type;
    Price limit_price;      // ignored for market
    Quantity qty;
};

struct Fill {
    OrderId order_id;
    Side side;
    Price price;
    Quantity qty;
};

class ExecutionModel {
public:
    virtual ~ExecutionModel() = default;

    virtual OrderId submit(const OrderIntent& intent) = 0;

    // Called on each market update / trade tick depending on mode
    virtual void on_tick() = 0;

    // Pull fills deterministically (queue)
    virtual bool poll_fill(Fill& out) = 0;
};

}
