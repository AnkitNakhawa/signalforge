#pragma once
#include "cpp/interfaces/execution_model.h"
#include "cpp/orderbook/order_book.h"

namespace signalforge
{
    class Strategy {
        public:
            virtual ~Strategy() = default;

            virtual void intialize() {} // Called once at start of backtest

            virtual void on_trade(Price trade_price, uint64_t timestamp) = 0; // Called on each trade

            virtual void on_fill(const Fill& fill) = 0; // Called on each fill

            virtual void finalize() {} // Called once at end of backtest

            void set_execution_model(ExecutionModel* exec) { exec_ = exec; }

        protected:
            ExecutionModel* exec_ = nullptr;
    };
}