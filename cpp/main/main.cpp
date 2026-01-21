#include <iostream>
#include "cpp/orderbook/order_book.h"

int main() {
    signalforge::OrderBook book;

    book.set_level(signalforge::Side::BID, 100, 10);
    book.set_level(signalforge::Side::ASK, 105, 5);

    std::cout << "Best bid: " << book.best_bid() << "\n";
    std::cout << "Best ask: " << book.best_ask() << "\n";
    return 0;
}
