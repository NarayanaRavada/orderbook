#include "order.h"
#include "order_book.h"
#include <iostream>
#include <thread>

void runDemo(OrderBook& book) {
  std::cout << "=== Order Book Demo ===\n";

  // Add some sample orders
  std::vector<Order> demo_orders = {
    {1, 100.50, 10, Side::Buy},   // Buy 10 @ $100.50
    {2, 100.25, 15, Side::Buy},   // Buy 15 @ $100.25
    {3, 101.00, 8, Side::Sell},   // Sell 8 @ $101.00
    {4, 101.25, 12, Side::Sell},  // Sell 12 @ $101.25
    {5, 100.75, 20, Side::Buy},   // Buy 20 @ $100.75
  };

  for (auto& order : demo_orders) {
    std::cout << "\nAdding order " << order.getId() << ": "
      << (order.getSide() == Side::Buy ? "BUY" : "SELL") << " "
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

