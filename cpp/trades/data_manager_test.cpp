#include "data_manager.h"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

namespace signalforge {

class DataManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test data directory
        test_dir_ = std::filesystem::temp_directory_path() / "data_manager_test";
        std::filesystem::create_directories(test_dir_ / "BTCUSDT");

        // Create sample CSV file with known data
        create_sample_csv();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }

    void create_sample_csv() {
        std::string csv_path = (test_dir_ / "BTCUSDT" / "trades-2024-01-15.csv").string();
        std::ofstream file(csv_path);

        // Header
        file << "trade_id,price,qty,quote_qty,time,is_buyer_maker\n";

        // 10 trades over 10 seconds (1 per second)
        for (int i = 0; i < 10; i++) {
            uint64_t timestamp = 1640000000000 + (i * 1000);  // 1 second apart
            file << (1000 + i) << ","
                 << (42500.0 + i) << ","
                 << "0.1,4250.0,"
                 << timestamp << ","
                 << "true\n";
        }

        file.close();
    }

    std::filesystem::path test_dir_;
};

TEST_F(DataManagerTest, GetFilePath) {
    DataManager dm(test_dir_.string());

    std::string path = dm.get_file_path("BTCUSDT", "2024-01-15");

    EXPECT_TRUE(path.find("BTCUSDT") != std::string::npos);
    EXPECT_TRUE(path.find("2024-01-15") != std::string::npos);
    EXPECT_TRUE(path.find(".csv") != std::string::npos);
}

TEST_F(DataManagerTest, HasData) {
    DataManager dm(test_dir_.string());

    EXPECT_TRUE(dm.has_data("BTCUSDT", "2024-01-15"));
    EXPECT_FALSE(dm.has_data("BTCUSDT", "2024-01-16"));
    EXPECT_FALSE(dm.has_data("ETHUSDT", "2024-01-15"));
}

TEST_F(DataManagerTest, LoadDayRaw) {
    DataManager dm(test_dir_.string());

    auto trades = dm.load_day("BTCUSDT", "2024-01-15", DataManager::Granularity::RAW);

    EXPECT_EQ(trades.size(), 10);

    // Check first trade
    EXPECT_EQ(trades[0].trade_id, 1000);
    EXPECT_EQ(trades[0].price, 4250000);  // 42500.00 * 100

    // Check stats
    auto stats = dm.last_load_stats();
    EXPECT_EQ(stats.raw_trade_count, 10);
    EXPECT_EQ(stats.sampled_trade_count, 10);
    EXPECT_DOUBLE_EQ(stats.sampling_ratio, 1.0);
}

TEST_F(DataManagerTest, LoadDayPerSecond) {
    DataManager dm(test_dir_.string());

    auto trades = dm.load_day("BTCUSDT", "2024-01-15", DataManager::Granularity::PER_SECOND);

    // We have 10 trades, 1 per second, so all should be included
    EXPECT_EQ(trades.size(), 10);

    auto stats = dm.last_load_stats();
    EXPECT_EQ(stats.raw_trade_count, 10);
    EXPECT_EQ(stats.sampled_trade_count, 10);
}

TEST_F(DataManagerTest, LoadDayPerMinute) {
    DataManager dm(test_dir_.string());

    auto trades = dm.load_day("BTCUSDT", "2024-01-15", DataManager::Granularity::PER_MINUTE);

    // 10 trades over 10 seconds = all in same minute bucket
    EXPECT_EQ(trades.size(), 1);

    auto stats = dm.last_load_stats();
    EXPECT_EQ(stats.raw_trade_count, 10);
    EXPECT_EQ(stats.sampled_trade_count, 1);
    EXPECT_DOUBLE_EQ(stats.sampling_ratio, 0.1);
}

TEST_F(DataManagerTest, LoadDayPerHour) {
    DataManager dm(test_dir_.string());

    auto trades = dm.load_day("BTCUSDT", "2024-01-15", DataManager::Granularity::PER_HOUR);

    // All trades within same hour
    EXPECT_EQ(trades.size(), 1);
}

TEST_F(DataManagerTest, LoadDayPerDay) {
    DataManager dm(test_dir_.string());

    auto trades = dm.load_day("BTCUSDT", "2024-01-15", DataManager::Granularity::PER_DAY);

    // All trades within same day
    EXPECT_EQ(trades.size(), 1);
}

TEST_F(DataManagerTest, FileNotFound) {
    DataManager dm(test_dir_.string());

    EXPECT_THROW(
        dm.load_day("BTCUSDT", "2024-01-16"),
        std::runtime_error
    );
}

TEST_F(DataManagerTest, SamplingPreservesFirstTradeInBucket) {
    // Create CSV with multiple trades per second
    std::string csv_path = (test_dir_ / "BTCUSDT" / "trades-2024-01-16.csv").string();
    std::ofstream file(csv_path);

    file << "trade_id,price,qty,quote_qty,time,is_buyer_maker\n";

    // Multiple trades in same second
    file << "1,42500.00,0.1,4250.0,1640000000000,true\n";  // First
    file << "2,42501.00,0.1,4250.1,1640000000500,true\n";  // Same second
    file << "3,42502.00,0.1,4250.2,1640000001000,true\n";  // Next second

    file.close();

    DataManager dm(test_dir_.string());
    auto trades = dm.load_day("BTCUSDT", "2024-01-16", DataManager::Granularity::PER_SECOND);

    EXPECT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].trade_id, 1);  // First trade in first second
    EXPECT_EQ(trades[1].trade_id, 3);  // First trade in second second
}

TEST_F(DataManagerTest, EmptyFile) {
    // Create empty CSV
    std::string csv_path = (test_dir_ / "BTCUSDT" / "trades-2024-01-17.csv").string();
    std::ofstream file(csv_path);
    file << "trade_id,price,qty,quote_qty,time,is_buyer_maker\n";
    file.close();

    DataManager dm(test_dir_.string());
    auto trades = dm.load_day("BTCUSDT", "2024-01-17");

    EXPECT_EQ(trades.size(), 0);

    auto stats = dm.last_load_stats();
    EXPECT_EQ(stats.raw_trade_count, 0);
    EXPECT_EQ(stats.sampled_trade_count, 0);
    EXPECT_DOUBLE_EQ(stats.sampling_ratio, 0.0);
}

}  // namespace signalforge
