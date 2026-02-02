#include "backtester.h"
#include <iostream>

namespace signalforge {

Backtester::Backtester(std::unique_ptr<Strategy> strategy)
    : strategy_(std::move(strategy)), position_tracker_(), data_manager_() {}

BacktestResults Backtester::run_single_day(
    const std::string& symbol,
    const std::string& date,
    DataManager::Granularity granularity
) {
    // Load data
    std::vector<Trade> trades = data_manager_.load_day(symbol, date, granularity);

    if (trades.empty()) {
        std::cerr << "Warning: No trades loaded for " << symbol << " on " << date << std::endl;
        return BacktestResults{};
    }

    std::cout << "Loaded " << trades.size() << " trades for " << symbol << " on " << date << std::endl;

    // Create market view and execution
    TradeOnlyMarketView market_view;
    TradeThroughExecution execution(market_view);

    // Give strategy access to execution model
    strategy_->set_execution_model(&execution);
    strategy_->intialize();  // Note: typo in strategy.h - it's "intialize" not "initialize"

    // Main backtest loop
    for (const auto& trade : trades) {
        // Update market (note: on_trade only takes price, not timestamp)
        market_view.on_trade(trade.price);

        // Let strategy react to trade
        strategy_->on_trade(trade.price, trade.timestamp);

        // Execute pending orders
        execution.on_tick();

        // Process fills
        Fill fill;
        while (execution.poll_fill(fill)) {
            strategy_->on_fill(fill);
            position_tracker_.on_fill(fill);
        }
    }

    // Finalize strategy
    strategy_->finalize();

    // Calculate results
    BacktestResults results;
    results.realized_pnl = position_tracker_.realized_pnl();

    // Get final price for unrealized PnL
    Price final_price = trades.back().price;
    results.unrealized_pnl = position_tracker_.unrealized_pnl(final_price);
    results.total_pnl = results.realized_pnl + results.unrealized_pnl;

    results.final_position = position_tracker_.position();

    std::cout << "Backtest complete!" << std::endl;
    std::cout << "Final position: " << results.final_position << std::endl;
    std::cout << "Realized PnL: " << results.realized_pnl << std::endl;
    std::cout << "Unrealized PnL: " << results.unrealized_pnl << std::endl;
    std::cout << "Total PnL: " << results.total_pnl << std::endl;

    return results;
}

}  // namespace signalforge
