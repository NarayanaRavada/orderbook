#include "order_book.h"
#include <iostream>
#include <thread>

void runDemo(OrderBook& book) {
  std::cout << "=== Order Book Demo ===\n";

  // Add some sample orders
  std::vector<Order> demo_orders = {
    {1, 100.50, 10, true},   // Buy 10 @ $100.50
    {2, 100.25, 15, true},   // Buy 15 @ $100.25
    {3, 101.00, 8, false},   // Sell 8 @ $101.00
    {4, 101.25, 12, false},  // Sell 12 @ $101.25
    {5, 100.75, 20, true},   // Buy 20 @ $100.75
  };

  for (auto& order : demo_orders) {
    std::cout << "\nAdding order " << order.getId() << ": "
      << (order.isBuyOrder() ? "BUY" : "SELL") << " "
      << order.getInitialQuantity() << " @ $" << order.getPrice() << "\n";

    book.addOrder(order);
    book.printOrderBook(4);

    // Small delay for dramatic effect
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

int main() {
  OrderBook book;

  runDemo(book);

  return 0;
}

