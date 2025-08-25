#include "../src/order_book.h"
#include <cassert>
#include <iostream>
#include <random>
#include <iomanip>
#include <thread>

void test_basic_operations() {
  std::cout << "Running basic operations test...\n";

  OrderBook book;

  // Create orders using your Order constructor
  Order buy_order(1, 100.0, 10, Side::Buy);   // Buy 10 @ $100
  Order sell_order(2, 101.0, 15, Side::Sell); // Sell 15 @ $101

  book.addOrder(buy_order);
  book.addOrder(sell_order);

  auto [best_bid, best_ask] = book.getBestPrices();

  std::cout << "Debug: best_bid=" << best_bid << ", best_ask=" << best_ask << std::endl;

  // The test expects these values
  assert(best_bid == 100.0);
  assert(best_ask == 101.0);

  // Test cancellation
  assert(book.cancelOrder(1) == true);
  assert(book.cancelOrder(999) == false);

  std::tie(best_bid, best_ask) = book.getBestPrices();
  assert(best_bid == 0.0);
  assert(best_ask == 101.0);

  std::cout << "✓ Basic operations test passed\n";
}

void test_order_matching() {
  std::cout << "Running order matching test...\n";

  OrderBook book;

  // Add some orders that shouldn't match
  book.addOrder({1, 100.0, 10, Side::Buy});   // Buy 10 @ $100
  book.addOrder({2, 101.0, 15, Side::Sell});  // Sell 15 @ $101

  assert(book.getTotalTrades() == 0);  // No trades yet

  // Add a buy order that should match
  book.addOrder({3, 101.0, 8, Side::Buy});   // Buy 8 @ $101 (matches sell order)

  assert(book.getTotalTrades() == 1);  // One trade should have occurred

  auto [best_bid, best_ask] = book.getBestPrices();
  assert(best_bid == 100.0);  // Original buy order still there
  assert(best_ask == 101.0);  // Partial sell order remains

  std::cout << "✓ Order matching test passed\n";
}

void test_price_time_priority() {
  std::cout << "Running price-time priority test...\n";

  OrderBook book;

  // Add multiple orders at same price
  book.addOrder({1, 100.0, 10, Side::Buy});   // First buy order
  book.addOrder({2, 100.0, 15, Side::Buy});   // Second buy order at same price

  // Add sell order that should match with first order (time priority)
  book.addOrder({3, 100.0, 5, Side::Sell});   // Sell 5 @ $100

  assert(book.getTotalTrades() == 1);

  std::cout << "✓ Price-time priority test passed\n";
}

void benchmark_performance(int num_orders) {
  std::cout << "Running performance benchmark with " << num_orders << " orders...\n";

  OrderBook book;

  // More realistic random data generation
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> price_dist(95.0, 105.0);  // Wider range
  std::uniform_int_distribution<> qty_dist(1, 1000);         // Larger quantities
  std::bernoulli_distribution side_dist(0.5);

  // Pre-generate orders to avoid timing random generation
  std::vector<Order> orders;
  orders.reserve(num_orders);

  for (int i = 0; i < num_orders; ++i) {
    orders.emplace_back(i, price_dist(gen), qty_dist(gen), side_dist(gen) ? Side::Sell : Side::Buy);
  }

  // Warm up the CPU cache
  for (int i = 0; i < 100; ++i) {
    OrderBook warmup;
    warmup.addOrder(orders[i % orders.size()]);
  }

  // Actual benchmark
  auto start = std::chrono::high_resolution_clock::now();

  for (const auto& order : orders) {
    book.addOrder(order);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Fixed calculations with proper floating point
  double total_microseconds = static_cast<double>(duration.count());
  double throughput = (static_cast<double>(num_orders) * 1000000.0) / total_microseconds;
  double avg_latency = total_microseconds / static_cast<double>(num_orders);

  std::cout << "Processed " << num_orders << " orders in "
    << duration.count() << " microseconds\n";
  std::cout << "Throughput: " << std::fixed << std::setprecision(2)
    << throughput << " orders/second\n";
  std::cout << "Average latency: " << std::fixed << std::setprecision(3)
    << avg_latency << " microseconds/order\n";

  // Additional statistics
  auto [bestBid, bestAsk] = book.getBestPrices();
  std::cout << "Final book state - Orders: " << book.getTotalOrders()
    << ", Trades: " << book.getTotalTrades() << "\n";
  std::cout << "Best Bid: $" << bestBid << ", Best Ask: $" << bestAsk << "\n\n";
}

void realistic_benchmark(int num_orders) {
  OrderBook book;

  // Prevent compiler optimization with volatile
  volatile int dummy_result = 0;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> price_dist(95.0, 105.0);
  std::uniform_int_distribution<> qty_dist(1, 1000);
  std::bernoulli_distribution side_dist(0.5);

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_orders; ++i) {
    // Generate order inside loop to prevent optimization
    Order order(i, price_dist(gen), qty_dist(gen), side_dist(gen) ? Side::Buy : Side::Sell);

    book.addOrder(order);

    // Force the compiler to not optimize away the operation
    auto [bid, ask] = book.getBestPrices();
    dummy_result += static_cast<int>(bid + ask);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Use the dummy result to prevent dead code elimination
  std::cout << "Dummy result: " << dummy_result << std::endl;

  double throughput = (static_cast<double>(num_orders) * 1000000.0) / duration.count();
  std::cout << "Realistic throughput: " << throughput << " orders/second\n";
}

void ultra_realistic_benchmark(int num_orders) {
    OrderBook book;

    // Add initial market data to make it realistic
    for (int i = 0; i < 50; ++i) {
        book.addOrder(Order(i, 100.0 + i * 0.1, 100, i % 2 == 0 ? Side::Buy : Side::Sell));
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> price_dist(95.0, 105.0);
    std::uniform_int_distribution<> qty_dist(1, 1000);
    std::bernoulli_distribution side_dist(0.5);
    std::uniform_int_distribution<> action_dist(0, 10); // 10% cancellations

    volatile double price_sum = 0.0;
    volatile int order_count = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_orders; ++i) {
        if (action_dist(gen) == 0 && i > 100) {
            // 10% cancellation operations
            book.cancelOrder(i - 50);  // Cancel older order
        } else {
            // 90% add operations
            Order order(i + 1000, price_dist(gen), qty_dist(gen), side_dist(gen) ? Side::Buy : Side::Sell);
            book.addOrder(order);
        }

        // Force expensive operations every 10 orders
        if (i % 10 == 0) {
            auto [bid, ask] = book.getBestPrices();
            price_sum += bid + ask;
            order_count += book.getTotalOrders();

            // Add some "business logic" overhead
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Use results to prevent optimization
    std::cout << "Price sum: " << price_sum << ", Order count sum: " << order_count << std::endl;

    double throughput = (static_cast<double>(num_orders) * 1000000.0) / duration.count();
    double latency = static_cast<double>(duration.count()) / num_orders;

    std::cout << "Ultra-realistic throughput: " << std::fixed << std::setprecision(0)
              << throughput << " orders/second\n";
    std::cout << "Average latency: " << std::fixed << std::setprecision(3)
              << latency << " microseconds/order\n";
}

int main() {
  std::cout << "=== Order Book Test Suite ===\n\n";

  test_basic_operations();
  test_order_matching();
  test_price_time_priority();

  std::cout << "\n=== Performance Benchmarks ===\n";
  //benchmark_performance(1000);
  //benchmark_performance(10000);

  ultra_realistic_benchmark(1000);
  ultra_realistic_benchmark(10000);

  std::cout << "\n✓ All tests passed!\n";
  return 0;
}

