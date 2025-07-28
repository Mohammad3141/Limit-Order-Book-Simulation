#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <cstdint>
#include <stdexcept>

enum class Side
{
    BUY,
    SELL,
    UNKNOWN
};

enum class OrderType
{
    LIMIT,
    MARKET,
    CANCEL,
    MODIFY
};

struct Order
{
    uint64_t order_id;
    uint64_t time_ns;
    std::string raw_time;
    int token;
    Side side;
    int price;
    int quantity;
    OrderType type;
    bool is_active;
    uint64_t user_id; // agent/trader identifier

    // Robust explicit constructor
    Order(
        uint64_t order_id_,
        uint64_t time_ns_,
        const std::string &raw_time_,
        int token_,
        Side side_,
        int price_,
        int quantity_,
        OrderType type_ = OrderType::LIMIT,
        bool is_active_ = true,
        uint64_t user_id_ = 0) : order_id(order_id_), time_ns(time_ns_), raw_time(raw_time_), token(token_),
                                 side(side_), price(price_), quantity(quantity_), type(type_),
                                 is_active(is_active_), user_id(user_id_)
    {
        validate();
    }

    void validate() const
    {
        if (price <= 0)
            throw std::invalid_argument("Order price must be positive.");
        if (quantity <= 0)
            throw std::invalid_argument("Order quantity must be positive.");
        if (side == Side::UNKNOWN)
            throw std::invalid_argument("Order side must be BUY or SELL.");
    }
};

#endif
