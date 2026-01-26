#pragma once
#include <cstdint>
#include <iostream>

namespace signalforge {

struct BacktestResults {
    // Basic metrics
    double total_pnl = 0.0;
    double realized_pnl = 0.0;
    double unrealized_pnl = 0.0;

    // Trade statistics
    size_t total_trades = 0;
    size_t winning_trades = 0;
    size_t losing_trades = 0;
    double win_rate = 0.0;  // Percentage

    // Performance
    double max_drawdown = 0.0;
    double max_position = 0.0;

    // Time
    uint64_t start_timestamp = 0;
    uint64_t end_timestamp = 0;

    void print() const {
        std::cout << "=== Backtest Results ===" << std::endl;
        std::cout << "Total PnL: $" << total_pnl << std::endl;
        std::cout << "Realized PnL: $" << realized_pnl << std::endl;
        std::cout << "Unrealized PnL: $" << unrealized_pnl << std::endl;
        std::cout << "Total Trades: " << total_trades << std::endl;
        std::cout << "Win Rate: " << win_rate << "%" << std::endl;
        std::cout << "Max Drawdown: $" << max_drawdown << std::endl;
    }
};

}  // namespace signalforge
