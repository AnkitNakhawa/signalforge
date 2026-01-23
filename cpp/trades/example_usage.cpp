// Example: How to use TradeCsvLoader for BTCUSDT backtesting

#include "trade_csv_loader.h"
#include "cpp/market/trade_only_market_view.h"
#include "cpp/execution/trade_through_execution.h"
#include <iostream>

namespace signalforge {

void example_backtest() {
    // 1. Load historical trade data from Binance CSV
    TradeCsvLoader loader;
    auto trades = loader.load("BTCUSDT-trades-2024-01.csv");

    std::cout << "Loaded " << trades.size() << " trades" << std::endl;
    std::cout << "Skipped " << loader.skipped_rows() << " invalid rows" << std::endl;

    // 2. Set up market view and execution model
    TradeOnlyMarketView market_view;
    TradeThroughExecution exec(market_view);

    // 3. Submit some orders to backtest
    // Buy limit order at $42,500.00 (4250000 ticks)
    OrderId buy_id = exec.submit({Side::BID, OrderType::LIMIT, 4250000, 1});

    // Sell limit order at $42,550.00 (4255000 ticks)
    OrderId sell_id = exec.submit({Side::ASK, OrderType::LIMIT, 4255000, 1});

    std::cout << "Submitted orders: buy_id=" << buy_id << ", sell_id=" << sell_id << std::endl;

    // 4. Process each trade sequentially
    size_t fill_count = 0;
    for (const auto& trade : trades) {
        // Update market with new trade price
        market_view.on_trade(trade.price);

        // Execute any matching orders
        exec.on_tick();

        // Poll for fills
        Fill fill;
        while (exec.poll_fill(fill)) {
            fill_count++;

            // Convert price back to decimal for display
            double price_decimal = fill.price / 100.0;

            std::cout << "Fill #" << fill_count << ": "
                      << "OrderID=" << fill.order_id << ", "
                      << "Side=" << (fill.side == Side::BID ? "BUY" : "SELL") << ", "
                      << "Price=$" << price_decimal << ", "
                      << "Qty=" << fill.qty
                      << std::endl;
        }
    }

    std::cout << "\nBacktest complete!" << std::endl;
    std::cout << "Total fills: " << fill_count << std::endl;
}

}  // namespace signalforge

// Uncomment to run as standalone program:
// int main() {
//     signalforge::example_backtest();
//     return 0;
// }
