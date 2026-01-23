#include "trade_csv_loader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>

namespace signalforge {

std::vector<Trade> TradeCsvLoader::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    std::vector<Trade> trades;
    std::string line;
    skipped_rows_ = 0;
    bool first_line = true;

    while (std::getline(file, line)) {
        // Skip header row if it exists (check if first field is "trade_id")
        if (first_line) {
            first_line = false;
            if (line.find("trade_id") != std::string::npos) {
                continue;
            }
        }

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        try {
            // Parse CSV line: trade_id,price,qty,quote_qty,time,is_buyer_maker
            std::stringstream ss(line);
            std::string field;
            std::vector<std::string> fields;

            // Split by comma
            while (std::getline(ss, field, ',')) {
                fields.push_back(field);
            }

            // Need at least 5 fields (trade_id, price, qty, quote_qty, time)
            if (fields.size() < 5) {
                skipped_rows_++;
                continue;
            }

            // Extract and convert fields
            Trade trade;
            trade.trade_id = std::stoull(fields[0]);

            // Convert price to ticks (2 decimal precision)
            double price_float = std::stod(fields[1]);
            trade.price = static_cast<Price>(std::round(price_float * 100.0));

            trade.timestamp = std::stoull(fields[4]);

            trades.push_back(trade);

        } catch (const std::exception&) {
            // Skip malformed rows silently
            skipped_rows_++;
        }
    }

    return trades;
}

}  // namespace signalforge
