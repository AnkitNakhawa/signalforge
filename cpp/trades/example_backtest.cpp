// Example: Complete backtest using DataManager

#include "data_manager.h"
#include "cpp/market/trade_only_market_view.h"
#include "cpp/execution/trade_through_execution.h"
#include <iostream>

namespace signalforge {

void example_simple_backtest() {
    std::cout << "=== Simple Backtest Example ===" << std::endl;

    // 1. Set up data manager
    DataManager dm("data");

    // 2. Load a day of data (sampled to 1 per minute)
    std::cout << "\nLoading BTCUSDT data for 2024-01-15..." << std::endl;

    try {
        auto trades = dm.load_day(
            "BTCUSDT",
            "2024-01-15",
            DataManager::Granularity::PER_MINUTE
        );

        // Show stats
        auto stats = dm.last_load_stats();
        std::cout << "✓ Loaded " << stats.sampled_trade_count << " trades" << std::endl;
        std::cout << "  Raw trades: " << stats.raw_trade_count << std::endl;
        std::cout << "  Sampling ratio: " << (stats.sampling_ratio * 100) << "%" << std::endl;

        // 3. Set up market and execution
        TradeOnlyMarketView market_view;
        TradeThroughExecution exec(market_view);

        // 4. Place some orders
        // Buy if price drops to $42,000
        Price buy_price = 4200000;  // $42,000.00 in ticks
        OrderId buy_order = exec.submit({Side::BID, OrderType::LIMIT, buy_price, 1});

        // Sell if price rises to $44,000
        Price sell_price = 4400000;  // $44,000.00 in ticks
        OrderId sell_order = exec.submit({Side::ASK, OrderType::LIMIT, sell_price, 1});

        std::cout << "\n📊 Orders placed:" << std::endl;
        std::cout << "  BUY  @ $" << (buy_price / 100.0) << " (ID: " << buy_order << ")" << std::endl;
        std::cout << "  SELL @ $" << (sell_price / 100.0) << " (ID: " << sell_order << ")" << std::endl;

        // 5. Run backtest
        std::cout << "\n🔄 Running backtest..." << std::endl;

        size_t fill_count = 0;
        Price min_price = trades[0].price;
        Price max_price = trades[0].price;

        for (const auto& trade : trades) {
            // Update market
            market_view.on_trade(trade.price);

            // Track price range
            if (trade.price < min_price) min_price = trade.price;
            if (trade.price > max_price) max_price = trade.price;

            // Execute orders
            exec.on_tick();

            // Process fills
            Fill fill;
            while (exec.poll_fill(fill)) {
                fill_count++;
                double fill_price_dollars = fill.price / 100.0;

                std::cout << "  ✅ FILL #" << fill_count << ": "
                          << (fill.side == Side::BID ? "BUY" : "SELL")
                          << " @ $" << fill_price_dollars
                          << " (Order #" << fill.order_id << ")"
                          << std::endl;
            }
        }

        // 6. Results
        std::cout << "\n📈 Backtest Results:" << std::endl;
        std::cout << "  Trades processed: " << trades.size() << std::endl;
        std::cout << "  Fills: " << fill_count << std::endl;
        std::cout << "  Price range: $" << (min_price / 100.0)
                  << " - $" << (max_price / 100.0) << std::endl;

        if (fill_count == 0) {
            std::cout << "\n💡 Tip: Price never reached order levels." << std::endl;
            std::cout << "   Try adjusting order prices based on the price range above." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "\n❌ Error: " << e.what() << std::endl;
        std::cerr << "\n💡 To download data, run:" << std::endl;
        std::cerr << "   ./scripts/download_binance_data.sh BTCUSDT 7" << std::endl;
    }
}

void example_multi_day_backtest() {
    std::cout << "\n=== Multi-Day Backtest Example ===" << std::endl;

    DataManager dm("data");

    // List of dates to test
    std::vector<std::string> dates = {
        "2024-01-15",
        "2024-01-16",
        "2024-01-17"
    };

    size_t total_trades = 0;

    for (const auto& date : dates) {
        if (dm.has_data("BTCUSDT", date)) {
            auto trades = dm.load_day("BTCUSDT", date, DataManager::Granularity::PER_MINUTE);
            total_trades += trades.size();
            std::cout << "  " << date << ": " << trades.size() << " trades" << std::endl;
        } else {
            std::cout << "  " << date << ": No data available" << std::endl;
        }
    }

    std::cout << "\nTotal trades across all days: " << total_trades << std::endl;
}

void example_granularity_comparison() {
    std::cout << "\n=== Granularity Comparison ===" << std::endl;

    DataManager dm("data");
    std::string date = "2024-01-15";

    if (!dm.has_data("BTCUSDT", date)) {
        std::cout << "Data not available for " << date << std::endl;
        return;
    }

    std::cout << "\nComparing different sampling rates for " << date << ":\n" << std::endl;

    std::vector<std::pair<std::string, DataManager::Granularity>> granularities = {
        {"RAW (all trades)", DataManager::Granularity::RAW},
        {"Per second", DataManager::Granularity::PER_SECOND},
        {"Per minute", DataManager::Granularity::PER_MINUTE},
        {"Per hour", DataManager::Granularity::PER_HOUR},
        {"Per day", DataManager::Granularity::PER_DAY}
    };

    for (const auto& [name, gran] : granularities) {
        auto trades = dm.load_day("BTCUSDT", date, gran);
        auto stats = dm.last_load_stats();

        std::cout << name << ":" << std::endl;
        std::cout << "  Trades: " << stats.sampled_trade_count << std::endl;
        std::cout << "  Ratio: " << (stats.sampling_ratio * 100) << "%" << std::endl;
        std::cout << std::endl;
    }
}

}  // namespace signalforge

// Uncomment to run:
// int main() {
//     signalforge::example_simple_backtest();
//     signalforge::example_multi_day_backtest();
//     signalforge::example_granularity_comparison();
//     return 0;
// }
