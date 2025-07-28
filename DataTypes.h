#ifndef DATATYPES_H
#define DATATYPES_H

#include <cstdint>
#include <string>

// Side of order or trade
enum class Side
{
    BUY,
    SELL
};

// Supported types of events
enum class EventType
{
    NEW,
    MODIFY,
    CANCEL,
    TRADE
};

// Minimal Order struct
struct Order
{
    uint64_t order_id;
    int price; // in minor unit
    int quantity;
    Side side;
    uint64_t user_id;

    Order(
        uint64_t oid = 0,
        int p = 0,
        int q = 0,
        Side s = Side::BUY,
        uint64_t uid = 0) : order_id(oid), price(p), quantity(q), side(s), user_id(uid) {}
};

// Unified Event struct
struct Event
{
    uint64_t timestamp;
    EventType type;
    uint64_t order_id;
    int price;
    int quantity;
    Side side;
    uint64_t token;
    uint64_t user_id;
    uint64_t buy_order_id;
    uint64_t sell_order_id;

    Event(
        uint64_t ts = 0,
        EventType t = EventType::NEW,
        uint64_t oid = 0,
        int p = 0,
        int q = 0,
        Side s = Side::BUY,
        uint64_t uid = 0,
        uint64_t boid = 0,
        uint64_t soid = 0) : timestamp(ts), type(t), order_id(oid), price(p), quantity(q), side(s),
                             user_id(uid), buy_order_id(boid), sell_order_id(soid) {}
};

#endif // DATATYPES_H
