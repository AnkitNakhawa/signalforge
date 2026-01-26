#include "position_tracker.h"
#include <cmath>

namespace signalforge {

    void PositionTracker::on_fill(const Fill& fill) {
        Quantity fill_qty = fill.qty;

        //sign adjustment for pos
        if (fill.side == Side::ASK) {
            fill_qty = -fill_qty;
        }

        bool is_closing = (position_ > 0 && fill_qty < 0) || (position_ < 0 && fill_qty > 0);

        if (is_closing) {
            //calc closing
            Quantity close_qty = std::min(std::abs(position_), std::abs(fill_qty));

            // realized PnL on closed
            double entry_price_dollars = avg_entry_price_ / 100.0;
            double exit_price_dollars = fill.price / 100.0;

            if (position_ > 0) {
                //closing long: PnL = (exit - entry)  qty
                realized_pnl_ = (exit_price_dollars - entry_price_dollars) * close_qty;
            } else {
                //closing short
                realized_pnl_ = (entry_price_dollars - exit_price_dollars) * close_qty;
            }

            position_ += fill_qty;

            //If pos flipped, new entry price
            if ((position_ > 0 && fill_qty > 0) || (position_ < 0 && fill_qty < 0)) {
                avg_entry_price_ = fill.price;
            } else {
                if (position_ == 0) {
                    //open new pos
                    avg_entry_price_ = fill.price;
                } else {
                    Quantity old_qty = std::abs(position_);
                    Quantity new_qty = std::abs(fill_qty);
                    Quantity total_qty = old_qty + new_qty;

                    avg_entry_price_ = (avg_entry_price_ * old_qty + fill.price * new_qty) / total_qty;
                }

                position_ += fill_qty;
            }

            
        }

    }

}