#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "DataTypes.h"
#include <map>
#include <list>
#include <vector>
#include <unordered_map>
#include <deque>
#include <utility>
#include <cstdint>

// Pointer to an order's location for fast cancellation
struct OrderLocation
{
    int price;
    Side side;
    std::list<Order>::iterator it;
};

// Book snapshot for rolling buffer analytics
struct OrderBookSnapshot
{
    uint64_t timestamp;
    std::vector<std::pair<int, int>> bid_levels; // price, total qty
    std::vector<std::pair<int, int>> ask_levels;
};

class OrderBook
{
public:
    OrderBook();

    void process_event(const Event &event);

    std::pair<int, int> get_best_bid_ask() const;
    int get_volume_at_price(int price, Side side) const;
    std::vector<std::pair<int, int>> get_bids_depth(int levels) const;
    std::vector<std::pair<int, int>> get_asks_depth(int levels) const;
    size_t order_count(Side side) const;

    void take_snapshot(uint64_t timestamp);
    void expire_old_snapshots(size_t max_snapshot_count);
    const std::deque<OrderBookSnapshot> &get_snapshots() const;

    bool would_self_trade(const Event &event) const;

private:
    void add_order(const Event &event);
    void modify_order(const Event &event);
    void cancel_order(uint64_t order_id);
    void process_trade(const Event &event);

    std::map<int, std::list<Order>, std::greater<int>> bids_;
    std::map<int, std::list<Order>> asks_;
    std::unordered_map<uint64_t, OrderLocation> order_map_;

    std::deque<OrderBookSnapshot> snapshots_;
    const size_t MAX_SNAPSHOTS = 1000;

    void cleanup_level(int price, Side side);
};

#endif // ORDERBOOK_H
