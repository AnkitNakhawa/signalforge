#include "backtester.h"
#include "simple_strategy.h"
#include <memory>
#include <iostream>

using namespace signalforge;

int main() {
    std::cout << "=== Backtester Example ===" << std::endl;

    // Create a simple strategy
    auto strategy = std::make_unique<SimpleStrategy>();

    // Create backtester
    Backtester backtester(std::move(strategy));

    // Run backtest for a single day
    // NOTE: Make sure you have data downloaded first!
    // Run: ./scripts/download_binance_data.sh BTCUSDT -s 2024-01-01 -e 2024-01-01
    BacktestResults results = backtester.run_single_day(
        "BTCUSDT",
        "2024-01-01",
        DataManager::Granularity::PER_MINUTE  // Sample to 1 trade per minute
    );

    std::cout << "\n=== Final Results ===" << std::endl;
    std::cout << "Final Position: " << results.final_position << std::endl;
    std::cout << "Realized PnL: $" << results.realized_pnl << std::endl;
    std::cout << "Unrealized PnL: $" << results.unrealized_pnl << std::endl;
    std::cout << "Total PnL: $" << results.total_pnl << std::endl;

    return 0;
}
