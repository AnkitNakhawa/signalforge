#pragma once
#include <deque>
#include <vector>
#include "cpp/interfaces/execution_model.h"
#include "cpp/interfaces/market_view.h"
#include <iostream>

namespace signalforge
{
    class TradeThroughExecution final : public ExecutionModel
    {
    public:
        explicit TradeThroughExecution(const MarketView& mv) : mv_(mv) {}

        OrderId submit(const OrderIntent& intent) override {
            const OrderId id = ++next_id_;
            open_.push_back({id, intent});
            return id;
        }

        void on_tick() override {
            if (!mv_.has_last()) return;
            const Price last_price = mv_.last_price();

            std::vector<size_t> to_erase;
            to_erase.reserve(open_.size());

            for (size_t i =0; i < open_.size(); ++i) {
                const auto& o = open_[i];
                const auto& in = o.intent;

                if (in.type == OrderType::MARKET) {
                    fills_.push_back({o.id, in.side, last_price, in.qty});
                    to_erase.push_back(i);
                    continue;
                }

                //LIMIT order w/ trade through
                if (in.side == Side::BID && last_price <= in.limit_price) {
                    fills_.push_back({o.id, in.side, last_price, in.qty});
                    to_erase.push_back(i);
                } else if (in.side == Side::ASK && last_price >= in.limit_price) {
                    fills_.push_back({o.id, in.side, last_price, in.qty});
                    to_erase.push_back(i);
                }

            }

            for (size_t k = to_erase.size(); k-- > 0;) {
                open_.erase(open_.begin() + static_cast<long>(to_erase[k]));
            }
        }

        bool poll_fill(Fill& out) override {
            if (fills_.empty()) return false;
            out = fills_.front();
            fills_.pop_front();
            return true;
        }

    private:
        struct OpenOrder { OrderId id; OrderIntent intent; };
        const MarketView& mv_;
        OrderId next_id_ = 0;
        std::deque<OpenOrder> open_;
        std::deque<Fill> fills_;
    };

}  // namespace signalforge