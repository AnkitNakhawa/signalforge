#pragma once
#include "cpp/interfaces/execution_model.h"
#include "cpp/orderbook/order_book.h"

namespace signalforge {
    class PositionTracker {   
    public:
        //init position
        PositionTracker() : position_(0), avg_entry_price_(0), realized_pnl_(0) {}
        
        //update position based on fill order (buy -> ++, sell --> --)
        void on_fill(const Fill& fill);

        //return position
        Quantity position() const {return position_;}

        //calc prof/loss on OPEN pos (cur price vs mkt price)
        double unrealized_pnl(Price current_price) const;

        //total form closed positions
        double realized_pnl() const { return realized_pnl_; }


        double total_pnl(Price current_price) const {
            return realized_pnl_ + unrealized_pnl(current_price);
        }

        //avg price based on all positions
        Price avg_entry_price() const {
            return avg_entry_price_;
        }

        private:
            Quantity position_;
            Price avg_entry_price_;
            double realized_pnl_;

    };
}