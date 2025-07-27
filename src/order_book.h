#pragma once

#include "order.h"

#include <map>
#include <queue>
#include <unordered_map>
#include <vector>

struct OrderBook {
private:
  // highest bid first
  std::map<double, std::queue<Order>, std::greater<double>> bids;

  // least ask first
  std::map<double, std::queue<Order>> asks;

  // order lookup
  std::unordered_map<int, Order> orders;

  // trade history
  std::vector<Trade> tradeHistory;

  // if match possible
  std::vector<Trade> tryMatch(Order& order);
public:

  // add market order
  void addOrder(Order order);
  bool cancelOrder(int order_id);

  std::pair<double, double> getBestPrices() const;
  int getBidVolume(double price) const;
  int getAskVolume(double price) const;

  size_t getTotalOrders() const;
  size_t getTotalTrades() const;

  // printing
  void printOrderBook(int depth) const;
};
