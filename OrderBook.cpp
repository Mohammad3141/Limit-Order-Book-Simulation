#include "OrderBook.h"
#include <iostream>
#include <algorithm>

OrderBook::OrderBook() : bids_{}, asks_{}, order_map_{}, snapshots_{} {}

void OrderBook::process_event(const Event &event)
{
    if (event.type == EventType::NEW && would_self_trade(event))
    {
        std::cerr << "[OrderBook] Self-trade detected, order_id " << event.order_id << "; Ignored.\n";
        return;
    }

    switch (event.type)
    {
    case EventType::NEW:
        add_order(event);
        break;
    case EventType::MODIFY:
        modify_order(event);
        break;
    case EventType::CANCEL:
        cancel_order(event.order_id);
        break;
    case EventType::TRADE:
        process_trade(event);
        break;
    }
}

void OrderBook::add_order(const Event &event)
{
    if (event.quantity <= 0 || event.price <= 0)
    {
        std::cerr << "[OrderBook] Invalid order: price/quantity must be > 0 (order_id="
                  << event.order_id << ")\n";
        return;
    }
    int remaining_quantity = event.quantity;
    // BUY SIDE
    if (event.side == Side::BUY)
    {
        while (remaining_quantity > 0 && !asks_.empty() && event.price >= asks_.begin()->first)
        {
            auto &ask_list = asks_.begin()->second;
            auto &best_ask = ask_list.front();
            if (best_ask.user_id == event.user_id && event.user_id != 0)
            {
                std::cerr << "[OrderBook] Prevented BUY self-trade on match (order_id=" << event.order_id << ")\n";
                break;
            }
            if (remaining_quantity >= best_ask.quantity)
            {
                remaining_quantity -= best_ask.quantity;
                cancel_order(best_ask.order_id);
            }
            else
            {
                best_ask.quantity -= remaining_quantity;
                remaining_quantity = 0;
            }
        }
        if (remaining_quantity > 0)
        {
            Order new_order(event.order_id, event.price, remaining_quantity, event.side, event.user_id);
            bids_[event.price].push_back(new_order);
            auto it = bids_[event.price].end();
            --it;
            order_map_[event.order_id] = {event.price, Side::BUY, it};
        }
        // SELL SIDE
    }
    else if (event.side == Side::SELL)
    {
        while (remaining_quantity > 0 && !bids_.empty() && event.price <= bids_.begin()->first)
        {
            auto &bid_list = bids_.begin()->second;
            auto &best_bid = bid_list.front();
            if (best_bid.user_id == event.user_id && event.user_id != 0)
            {
                std::cerr << "[OrderBook] Prevented SELL self-trade on match (order_id=" << event.order_id << ")\n";
                break;
            }
            if (remaining_quantity >= best_bid.quantity)
            {
                remaining_quantity -= best_bid.quantity;
                cancel_order(best_bid.order_id);
            }
            else
            {
                best_bid.quantity -= remaining_quantity;
                remaining_quantity = 0;
            }
        }
        if (remaining_quantity > 0)
        {
            Order new_order(event.order_id, event.price, remaining_quantity, event.side, event.user_id);
            asks_[event.price].push_back(new_order);
            auto it = asks_[event.price].end();
            --it;
            order_map_[event.order_id] = {event.price, Side::SELL, it};
        }
    }
    else
    {
        std::cerr << "[OrderBook] Attempt to add order with unknown side (order_id="
                  << event.order_id << ")\n";
    }
}

void OrderBook::modify_order(const Event &event)
{
    cancel_order(event.order_id);
    add_order(event);
}

void OrderBook::cancel_order(uint64_t order_id)
{
    auto it = order_map_.find(order_id);
    if (it == order_map_.end())
        return;
    const auto &loc = it->second;
    if (loc.side == Side::BUY)
    {
        bids_[loc.price].erase(loc.it);
        cleanup_level(loc.price, Side::BUY);
    }
    else
    {
        asks_[loc.price].erase(loc.it);
        cleanup_level(loc.price, Side::SELL);
    }
    order_map_.erase(it);
}

void OrderBook::process_trade(const Event &) {}

void OrderBook::cleanup_level(int price, Side side)
{
    if (side == Side::BUY && bids_[price].empty())
        bids_.erase(price);
    if (side == Side::SELL && asks_[price].empty())
        asks_.erase(price);
}

std::pair<int, int> OrderBook::get_best_bid_ask() const
{
    int best_bid = 0, best_ask = 0;
    if (!bids_.empty())
        best_bid = bids_.begin()->first;
    if (!asks_.empty())
        best_ask = asks_.begin()->first;
    return std::make_pair(best_bid, best_ask);
}

int OrderBook::get_volume_at_price(int price, Side side) const
{
    int total = 0;
    if (side == Side::BUY)
    {
        auto it = bids_.find(price);
        if (it != bids_.end())
            for (const auto &o : it->second)
                total += o.quantity;
    }
    else
    {
        auto it = asks_.find(price);
        if (it != asks_.end())
            for (const auto &o : it->second)
                total += o.quantity;
    }
    return total;
}

std::vector<std::pair<int, int>> OrderBook::get_bids_depth(int levels) const
{
    std::vector<std::pair<int, int>> out;
    out.reserve(levels);
    auto it = bids_.begin();
    for (int lvl = 0; it != bids_.end() && lvl < levels; ++it, ++lvl)
    {
        int qty = 0;
        for (auto o = it->second.begin(); o != it->second.end(); ++o)
            qty += o->quantity;
        out.push_back({it->first, qty});
    }
    return out;
}

std::vector<std::pair<int, int>> OrderBook::get_asks_depth(int levels) const
{
    std::vector<std::pair<int, int>> out;
    out.reserve(levels);
    auto it = asks_.begin();
    for (int lvl = 0; it != asks_.end() && lvl < levels; ++it, ++lvl)
    {
        int qty = 0;
        for (auto o = it->second.begin(); o != it->second.end(); ++o)
            qty += o->quantity;
        out.push_back({it->first, qty});
    }
    return out;
}

size_t OrderBook::order_count(Side side) const
{
    size_t count = 0;
    if (side == Side::BUY)
    {
        for (auto it = bids_.begin(); it != bids_.end(); ++it)
            count += it->second.size();
    }
    else
    {
        for (auto it = asks_.begin(); it != asks_.end(); ++it)
            count += it->second.size();
    }
    return count;
}

void OrderBook::take_snapshot(uint64_t timestamp)
{
    OrderBookSnapshot snap;
    snap.timestamp = timestamp;
    snap.bid_levels = get_bids_depth(10);
    snap.ask_levels = get_asks_depth(10);
    snapshots_.push_back(snap);
    expire_old_snapshots(MAX_SNAPSHOTS);
}

void OrderBook::expire_old_snapshots(size_t max_snapshot_count)
{
    while (snapshots_.size() > max_snapshot_count)
    {
        snapshots_.pop_front();
    }
}
const std::deque<OrderBookSnapshot> &OrderBook::get_snapshots() const
{
    return snapshots_;
}

bool OrderBook::would_self_trade(const Event &event) const
{
    if (event.user_id == 0)
        return false;
    if (event.side == Side::BUY)
    {
        for (auto it = asks_.begin(); it != asks_.end(); ++it)
        {
            if (event.price >= it->first)
            {
                for (auto ot = it->second.begin(); ot != it->second.end(); ++ot)
                    if (ot->user_id == event.user_id)
                        return true;
            }
            else
                break;
        }
    }
    else if (event.side == Side::SELL)
    {
        for (auto it = bids_.begin(); it != bids_.end(); ++it)
        {
            if (event.price <= it->first)
            {
                for (auto ot = it->second.begin(); ot != it->second.end(); ++ot)
                    if (ot->user_id == event.user_id)
                        return true;
            }
            else
                break;
        }
    }
    return false;
}
