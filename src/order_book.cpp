#include "order.h"
#include "order_book.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

std::vector<Trade> OrderBook::tryMatch(Order& order) {
  std::vector<Trade> trades;

  auto matchOrderAgainstBook = [&order, &trades, this] (auto& priceLevels) {
    auto isBuy = order.isBuyOrder();
    auto priceCmp = isBuy
      ? [] (double a, double b) { return a >= b; }
      : [] (double a, double b) { return a <= b; };

    auto priceLevelIt = priceLevels.begin();

    while (priceLevelIt != priceLevels.end() && !order.isFilled() && priceCmp(order.getPrice(), priceLevelIt->first)) {
      auto price = priceLevelIt->first;
      auto& orderQueue = priceLevelIt->second;

      while (!orderQueue.empty() && !order.isFilled()) {
        Order& bookOrder = orderQueue.front();

        int tradeQuantity = std::min(order.getRemainingQuantity(), bookOrder.getRemainingQuantity());

        if (isBuy) {
          trades.emplace_back(order.getId(), bookOrder.getId(), price, tradeQuantity);
        } else {
          trades.emplace_back(bookOrder.getId(), order.getId(), price, tradeQuantity);
        }

        order.fill(tradeQuantity);
        bookOrder.fill(tradeQuantity);

        if (bookOrder.isFilled()) {
          orders.erase(bookOrder.getId());
          orderQueue.pop();
        }
      }

      if (orderQueue.empty()) {
        priceLevelIt = priceLevels.erase(priceLevelIt);
      } else {
        priceLevelIt++;
      }
    }
  };


  if (order.isBuyOrder()) {
    matchOrderAgainstBook(asks);
  } else {
    matchOrderAgainstBook(bids);
  }

  return trades;
}

void OrderBook::addOrder(Order order) {
  std::vector<Trade> trades = tryMatch(order);

  tradeHistory.insert(tradeHistory.end(), trades.begin(), trades.end());

  if (!order.isFilled()) {
    orders.emplace(order.getId(), order);

    if (order.isBuyOrder()) {
      bids[order.getPrice()].push(order);
    } else {
      asks[order.getPrice()].push(order);
    }
  }
}

bool OrderBook::cancelOrder(int orderId) {
  auto it = orders.find(orderId);
  if (it == orders.end()) return false;
  auto& orderToCancel = it->second;

  auto deleteFromPriceLevel = [&orderToCancel, this] (auto& priceLevels) {
    auto priceLevelIt = priceLevels.find(orderToCancel.getPrice());

    if (priceLevelIt == priceLevels.end()) return false;

    auto& oldOrderQueue = priceLevelIt->second;
    std::queue<Order> orderQueue;

    bool found = false;

    while (!oldOrderQueue.empty()) {
      Order currentOrder = oldOrderQueue.front();
      oldOrderQueue.pop();

      if (currentOrder.getId() == orderToCancel.getId())  {
        found = true;
        continue;
      }

      orderQueue.push(currentOrder);
    }

    if (!found) { return false; }

    priceLevelIt->second = std::move(orderQueue);
    orders.erase(orderToCancel.getId());

    if (priceLevelIt->second.empty()) {
      priceLevels.erase(priceLevelIt);
    }

    return true;
  };

  if (orderToCancel.isBuyOrder()) {
    return deleteFromPriceLevel(bids);
  } else {
    return deleteFromPriceLevel(asks);
  }
}

std::pair<double, double> OrderBook::getBestPrices() const {
  double bestAsk = 0.0;
  double bestBid = 0.0;

  if (!asks.empty()) {
    bestAsk = asks.begin()->first;
  }

  if (!bids.empty()) {
    bestBid = bids.begin()->first;
  }

  return { bestBid, bestAsk };
}

int OrderBook::getBidVolume(double price) const {

  auto it = bids.find(price);
  if (it == bids.end()) return 0;

  int volume = 0;

  // expensive copy
  auto queueCopy = it->second;
  while (!queueCopy.empty()) {
    auto order = queueCopy.front();
    queueCopy.pop();

    volume += order.getRemainingQuantity();
  }

  return volume;
}

int OrderBook::getAskVolume(double price) const {

  auto it = asks.find(price);
  if (it == asks.end()) return 0;

  int volume = 0;

  // expensive copy
  auto queueCopy = it->second;
  while (!queueCopy.empty()) {
    auto order = queueCopy.front();
    queueCopy.pop();

    volume += order.getRemainingQuantity();
  }

  return volume;
}

size_t OrderBook::getTotalOrders() const {
  return orders.size();
}

// storage problems for large volume of trades
size_t OrderBook::getTotalTrades() const {
  return tradeHistory.size();
}

// printing
//
// Helper to calculate how many lines the order book printout uses, based on depth
int orderBookPrintLines(int depth) {
  // Lines printed by your function:
  // 1 line: "=== ORDER BOOK ==="
  // 1 line: best bid/ask/spread line
  // 1 empty line
  // 1 line header: BIDS / ASKS
  // 1 line header: Price Qty Price Qty
  // 1 separator line
  // depth lines for bids/asks rows
  // 1 empty line
  return 7 + depth + 1;
}

void OrderBook::printOrderBook(int depth) const {
  static int last_print_lines = 0;

  if (last_print_lines > 0) {
    // Move cursor up to start of previous output block
    std::cout << "\033[" << last_print_lines << "A";

    // Clear all lines in the block one by one
    for (int i = 0; i < last_print_lines - 2; ++i) {
      std::cout << "\033[2K";    // Clear entire line
      std::cout << "\033[1E";    // Move cursor down one line (to next line)
    }

    // Move cursor up again to the start of block before printing new output
    std::cout << "\033[" << last_print_lines << "F";
  }

  auto [best_bid, best_ask] = getBestPrices();

  std::cout << "\n=== ORDER BOOK ===\n";
  std::cout << "Best Bid: $" << std::fixed << std::setprecision(2) << best_bid
            << " | Best Ask: $" << best_ask;

  if (best_bid > 0 && best_ask > 0) {
    std::cout << " | Spread: $" << (best_ask - best_bid);
  }
  std::cout << "\n\n";

  std::cout << std::setw(15) << "BIDS" << std::setw(20) << "ASKS" << "\n";
  std::cout << std::setw(8) << "Price" << std::setw(8) << "Qty"
            << std::setw(8) << "Price" << std::setw(8) << "Qty" << "\n";
  std::cout << std::string(32, '-') << "\n";

  auto bid_it = bids.begin();
  auto ask_it = asks.begin();

  for (int i = 0; i < depth; ++i) {
    // Print bid side
    if (bid_it != bids.end()) {
      int total_qty = 0;
      std::queue<Order> temp_queue = bid_it->second;
      while (!temp_queue.empty()) {
        total_qty += temp_queue.front().getRemainingQuantity();
        temp_queue.pop();
      }
      std::cout << std::setw(8) << std::fixed << std::setprecision(2)
                << bid_it->first << std::setw(8) << total_qty;
      ++bid_it;
    } else {
      std::cout << std::setw(16) << "";
    }

    // Print ask side
    if (ask_it != asks.end()) {
      int total_qty = 0;
      std::queue<Order> temp_queue = ask_it->second;
      while (!temp_queue.empty()) {
        total_qty += temp_queue.front().getRemainingQuantity();
        temp_queue.pop();
      }
      std::cout << std::setw(8) << std::fixed << std::setprecision(2)
                << ask_it->first << std::setw(8) << total_qty;
      ++ask_it;
    }

    std::cout << "\n";
  }
  std::cout << "\n";

  last_print_lines = orderBookPrintLines(depth);
}
