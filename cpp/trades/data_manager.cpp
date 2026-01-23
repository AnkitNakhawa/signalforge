#include "data_manager.h"
#include <filesystem>
#include <stdexcept>
#include <sstream>

namespace signalforge {

DataManager::DataManager(const std::string& data_dir)
    : data_dir_(data_dir), last_stats_{0, 0, 0.0} {}

std::string DataManager::get_file_path(const std::string& symbol, const std::string& date) const {
    // Build path: data_dir/SYMBOL/trades-YYYY-MM-DD.csv
    std::ostringstream path;
    path << data_dir_ << "/" << symbol << "/trades-" << date << ".csv";
    return path.str();
}

bool DataManager::has_data(const std::string& symbol, const std::string& date) const {
    return std::filesystem::exists(get_file_path(symbol, date));
}

std::vector<Trade> DataManager::sample_trades(
    const std::vector<Trade>& raw_trades,
    Granularity granularity
) {
    if (granularity == Granularity::RAW || raw_trades.empty()) {
        return raw_trades;
    }

    std::vector<Trade> sampled;
    sampled.reserve(raw_trades.size() / 10);  // Estimate

    uint64_t time_interval_ms;
    switch (granularity) {
        case Granularity::PER_SECOND:
            time_interval_ms = 1000;  // 1 second
            break;
        case Granularity::PER_MINUTE:
            time_interval_ms = 60 * 1000;  // 60 seconds
            break;
        case Granularity::PER_HOUR:
            time_interval_ms = 60 * 60 * 1000;  // 3600 seconds
            break;
        case Granularity::PER_DAY:
            time_interval_ms = 24 * 60 * 60 * 1000;  // 86400 seconds
            break;
        default:
            return raw_trades;
    }

    uint64_t last_bucket = 0;
    for (const auto& trade : raw_trades) {
        uint64_t current_bucket = trade.timestamp / time_interval_ms;

        if (current_bucket != last_bucket) {
            sampled.push_back(trade);
            last_bucket = current_bucket;
        }
    }

    return sampled;
}

std::vector<Trade> DataManager::load_day(
    const std::string& symbol,
    const std::string& date,
    Granularity granularity
) {
    std::string file_path = get_file_path(symbol, date);

    // Check if file exists
    if (!std::filesystem::exists(file_path)) {
        throw std::runtime_error(
            "Data file not found: " + file_path +
            "\n\nTo download: visit https://data.binance.vision/?prefix=data/spot/daily/trades/" + symbol + "/"
            "\nOr run: wget https://data.binance.vision/data/spot/daily/trades/" + symbol + "/" + symbol + "-trades-" + date + ".zip"
        );
    }

    // Load raw trades
    auto raw_trades = csv_loader_.load(file_path);

    // Sample if needed
    auto sampled_trades = sample_trades(raw_trades, granularity);

    // Update stats
    last_stats_.raw_trade_count = raw_trades.size();
    last_stats_.sampled_trade_count = sampled_trades.size();
    last_stats_.sampling_ratio = raw_trades.empty() ? 0.0 :
        static_cast<double>(sampled_trades.size()) / raw_trades.size();

    return sampled_trades;
}

}  // namespace signalforge
