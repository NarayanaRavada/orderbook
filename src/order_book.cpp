#include "order.h"
#include "order_book.h"

#include <algorithm>
#include <vector>

std::vector<Trade> OrderBook::tryMatch(Order& order) {
  std::vector<Trade> trades;

  auto matchOrderAgainstBook = [&order, &trades, this] (auto& priceLevels) {
    auto isBuy = order.isBuyOrder();
    auto priceCmp = isBuy
      ? [] (double a, double b) { return a >= b; }
      : [] (double a, double b) { return a <= b; };

    auto priceLevelIt = priceLevels.begin();

    while (priceLevelIt != priceLevels.end() && !order.isFilled() && priceCmp(priceLevelIt->first, order.getPrice())) {
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

void OrderBook::addOrder(Order& order) {
  std::vector<Trade> trades = tryMatch(order);

  tradeHistory.insert(tradeHistory.end(), trades.begin(), trades.end());

  if (!order.isFilled()) {
    orders[order.getId()] = order;

    if (order.isBuyOrder()) {
      bids[order.getPrice()].push(order);
    } else {
      asks[order.getPrice()].push(order);
    }
  }
}

bool OrderBook::cancelOrder(int orderId) {
  auto& orderToCancel = orders[orderId];

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
    deleteFromPriceLevel(bids);
  } else {
    deleteFromPriceLevel(asks);
  }

  return true;
}
