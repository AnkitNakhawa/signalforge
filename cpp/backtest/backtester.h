#pragma once

#include "strategy.h"
#include "results.h"
#include "position_tracker.h"
#include "../trades/data_manager.h"
#include "../market/trade_only_market_view.h"
#include "../execution/trade_through_execution.h"
#include <string>
#include <memory>

namespace signalforge {

class Backtester {
public:
    // Constructor takes ownership of the strategy
    explicit Backtester(std::unique_ptr<Strategy> strategy);

    // Run backtest for a single day
    BacktestResults run_single_day(
        const std::string& symbol,
        const std::string& date,
        DataManager::Granularity granularity = DataManager::Granularity::RAW
    );

private:
    std::unique_ptr<Strategy> strategy_;
    PositionTracker position_tracker_;
    DataManager data_manager_;
};

}  // namespace signalforge

