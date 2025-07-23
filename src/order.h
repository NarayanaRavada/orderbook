#pragma once

#include <chrono>

struct Order {
private:
  int id_;
  double price_;
  int initialQuantity_;
  int remainingQuantity_;
  bool isBuy_;
  long timestamp_;

public:
  Order(int id, double price, int quantity, bool isBuy)
    : id_(id)
    , price_(price)
    , initialQuantity_(quantity)
    , remainingQuantity_(quantity)
    , isBuy_(isBuy)
  {
    timestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
    ).count();
  }

  int getId() const { return id_; }
  double getPrice() const { return price_; }
  int getInitialQuantity() const { return initialQuantity_; }
  int getRemainingQuantity() const { return remainingQuantity_; }
  bool isBuyOrder() const { return isBuy_; }
  long getTimestamp() const { return timestamp_; }

  bool isFilled() const { return remainingQuantity_ == 0; }
  void fill(int quantity) { remainingQuantity_ -= quantity; }
};

struct Trade {
private:
  int buyId_;
  int sellId_;
  double price_;
  int quantity_;
  long timestamp_;

public:
  Trade(int buyId, int sellId, double price, int quantity)
    : buyId_(buyId)
    , sellId_(sellId)
    , price_(price)
    , quantity_(quantity)
  {
    timestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
    ).count();
  }

  int getBuyId() const { return buyId_; }
  int getSellId() const { return sellId_; }
  double getPrice() const { return price_; }
  int getQuantity() const { return quantity_; }
  long getTimestamp() const { return timestamp_; }
};
