#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "cpp/orderbook/order_book.h"

namespace signalforge {

struct Trade {
    uint64_t trade_id;
    Price price;        // Price in ticks (2 decimal precision = price * 100)
    uint64_t timestamp; // Unix time in milliseconds
};

class TradeCsvLoader {
public:
    // Load all trades from a Binance CSV file
    // Expected format: trade_id,price,qty,quote_qty,time,is_buyer_maker
    // Throws std::runtime_error if file cannot be opened
    // Silently skips malformed rows
    std::vector<Trade> load(const std::string& filepath);

    // Get the number of rows that were skipped during the last load
    size_t skipped_rows() const { return skipped_rows_; }

private:
    size_t skipped_rows_ = 0;
};

}  // namespace signalforge
