#include "trade_csv_loader.h"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

namespace signalforge {

class TradeCsvLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test files
        test_dir_ = std::filesystem::temp_directory_path() / "trade_csv_test";
        std::filesystem::create_directories(test_dir_);
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(test_dir_);
    }

    std::string create_test_file(const std::string& filename, const std::string& content) {
        std::filesystem::path filepath = test_dir_ / filename;
        std::ofstream file(filepath);
        file << content;
        file.close();
        return filepath.string();
    }

    std::filesystem::path test_dir_;
    TradeCsvLoader loader;
};

// Test basic CSV loading with valid data
TEST_F(TradeCsvLoaderTest, LoadValidCsv) {
    std::string csv_content =
        "trade_id,price,qty,quote_qty,time,is_buyer_maker\n"
        "1234567,42500.50,0.025,1062.5125,1640000000000,true\n"
        "1234568,42501.00,0.100,4250.1000,1640000001000,false\n"
        "1234569,42499.75,0.050,2124.9875,1640000002000,true\n";

    std::string filepath = create_test_file("valid.csv", csv_content);
    auto trades = loader.load(filepath);

    ASSERT_EQ(trades.size(), 3);

    // Check first trade
    EXPECT_EQ(trades[0].trade_id, 1234567);
    EXPECT_EQ(trades[0].price, 4250050);  // 42500.50 * 100
    EXPECT_EQ(trades[0].timestamp, 1640000000000);

    // Check second trade
    EXPECT_EQ(trades[1].trade_id, 1234568);
    EXPECT_EQ(trades[1].price, 4250100);  // 42501.00 * 100
    EXPECT_EQ(trades[1].timestamp, 1640000001000);

    // Check third trade
    EXPECT_EQ(trades[2].trade_id, 1234569);
    EXPECT_EQ(trades[2].price, 4249975);  // 42499.75 * 100
    EXPECT_EQ(trades[2].timestamp, 1640000002000);
}

// Test CSV without header
TEST_F(TradeCsvLoaderTest, LoadCsvWithoutHeader) {
    std::string csv_content =
        "1234567,42500.50,0.025,1062.5125,1640000000000,true\n"
        "1234568,42501.00,0.100,4250.1000,1640000001000,false\n";

    std::string filepath = create_test_file("no_header.csv", csv_content);
    auto trades = loader.load(filepath);

    ASSERT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].price, 4250050);
}

// Test skipping malformed rows
TEST_F(TradeCsvLoaderTest, SkipMalformedRows) {
    std::string csv_content =
        "trade_id,price,qty,quote_qty,time,is_buyer_maker\n"
        "1234567,42500.50,0.025,1062.5125,1640000000000,true\n"
        "invalid,data,here\n"  // Malformed row
        "1234568,42501.00,0.100,4250.1000,1640000001000,false\n"
        "1234569,not_a_price,0.050,2124.9875,1640000002000,true\n"  // Invalid price
        "1234570,42502.00,0.075,3187.65,1640000003000,false\n";

    std::string filepath = create_test_file("malformed.csv", csv_content);
    auto trades = loader.load(filepath);

    EXPECT_EQ(trades.size(), 3);  // Only 3 valid rows
    EXPECT_EQ(loader.skipped_rows(), 2);  // 2 malformed rows

    // Verify the valid trades were loaded
    EXPECT_EQ(trades[0].trade_id, 1234567);
    EXPECT_EQ(trades[1].trade_id, 1234568);
    EXPECT_EQ(trades[2].trade_id, 1234570);
}

// Test empty file
TEST_F(TradeCsvLoaderTest, LoadEmptyFile) {
    std::string filepath = create_test_file("empty.csv", "");
    auto trades = loader.load(filepath);

    EXPECT_EQ(trades.size(), 0);
}

// Test file with only header
TEST_F(TradeCsvLoaderTest, LoadOnlyHeader) {
    std::string csv_content = "trade_id,price,qty,quote_qty,time,is_buyer_maker\n";
    std::string filepath = create_test_file("header_only.csv", csv_content);
    auto trades = loader.load(filepath);

    EXPECT_EQ(trades.size(), 0);
}

// Test file not found
TEST_F(TradeCsvLoaderTest, FileNotFound) {
    EXPECT_THROW(
        loader.load("/nonexistent/path/file.csv"),
        std::runtime_error
    );
}

// Test price conversion precision
TEST_F(TradeCsvLoaderTest, PriceConversionPrecision) {
    std::string csv_content =
        "1,100.00,1,100,1640000000000,true\n"
        "2,100.01,1,100.01,1640000001000,true\n"
        "3,100.99,1,100.99,1640000002000,true\n"
        "4,0.01,1,0.01,1640000003000,true\n"
        "5,99999.99,1,99999.99,1640000004000,true\n";

    std::string filepath = create_test_file("precision.csv", csv_content);
    auto trades = loader.load(filepath);

    EXPECT_EQ(trades[0].price, 10000);      // 100.00 * 100
    EXPECT_EQ(trades[1].price, 10001);      // 100.01 * 100
    EXPECT_EQ(trades[2].price, 10099);      // 100.99 * 100
    EXPECT_EQ(trades[3].price, 1);          // 0.01 * 100
    EXPECT_EQ(trades[4].price, 9999999);    // 99999.99 * 100
}

// Test large realistic BTCUSDT prices
TEST_F(TradeCsvLoaderTest, RealisticBtcusdtPrices) {
    std::string csv_content =
        "1,43256.78,0.001,43.25678,1640000000000,true\n"
        "2,43257.00,0.005,216.285,1640000001000,false\n"
        "3,43256.50,0.002,86.513,1640000002000,true\n";

    std::string filepath = create_test_file("btcusdt.csv", csv_content);
    auto trades = loader.load(filepath);

    EXPECT_EQ(trades[0].price, 4325678);    // 43256.78 * 100
    EXPECT_EQ(trades[1].price, 4325700);    // 43257.00 * 100
    EXPECT_EQ(trades[2].price, 4325650);    // 43256.50 * 100
}

// Test integration with TradeOnlyMarketView
TEST_F(TradeCsvLoaderTest, IntegrationWithMarketView) {
    std::string csv_content =
        "1,42500.00,0.1,4250,1640000000000,true\n"
        "2,42505.50,0.2,8501.1,1640000001000,false\n"
        "3,42510.25,0.15,6376.5375,1640000002000,true\n";

    std::string filepath = create_test_file("integration.csv", csv_content);
    auto trades = loader.load(filepath);

    // Simulate feeding trades into market view
    ASSERT_EQ(trades.size(), 3);

    // Each trade should have valid price data
    for (const auto& trade : trades) {
        EXPECT_GT(trade.price, 0);
        EXPECT_GT(trade.timestamp, 0);
        EXPECT_GT(trade.trade_id, 0);
    }
}

}  // namespace signalforge
