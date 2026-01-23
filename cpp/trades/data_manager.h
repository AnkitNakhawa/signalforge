#pragma once
#include <string>
#include <vector>
#include "trade_csv_loader.h"

namespace signalforge {

// Manages loading and sampling of historical trade data
class DataManager {
public:
    enum class Granularity {
        RAW,         // All trades (no sampling)
        PER_SECOND,  // 1 trade per second
        PER_MINUTE,  // 1 trade per minute (recommended)
        PER_HOUR,    // 1 trade per hour
        PER_DAY      // 1 trade per day (OHLC equivalent)
    };

    // Constructor with data directory path
    // Default: "data" (relative to working directory)
    explicit DataManager(const std::string& data_dir = "data");

    // Load trades for a specific day with optional sampling
    // date: Format "YYYY-MM-DD" (e.g., "2024-01-15")
    // symbol: Trading pair (e.g., "BTCUSDT")
    // granularity: Sampling rate
    // Returns: Vector of trades (sampled if granularity != RAW)
    std::vector<Trade> load_day(
        const std::string& symbol,
        const std::string& date,
        Granularity granularity = Granularity::PER_MINUTE
    );

    // Get the file path for a specific day's data
    // Returns: Full path to CSV file (e.g., "data/BTCUSDT/trades-2024-01-15.csv")
    std::string get_file_path(const std::string& symbol, const std::string& date) const;

    // Check if data file exists for a given day
    bool has_data(const std::string& symbol, const std::string& date) const;

    // Get statistics about loaded data
    struct Stats {
        size_t raw_trade_count;
        size_t sampled_trade_count;
        double sampling_ratio;  // sampled / raw
    };
    Stats last_load_stats() const { return last_stats_; }

private:
    std::string data_dir_;
    TradeCsvLoader csv_loader_;
    Stats last_stats_;

    // Sample trades according to granularity
    std::vector<Trade> sample_trades(
        const std::vector<Trade>& raw_trades,
        Granularity granularity
    );
};

}  // namespace signalforge
