#include "Utils.h"
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <iostream>

std::vector<std::string> Utils::split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

Event Utils::parse_line(const std::string &line, bool is_trade_file)
{
    Event event;
    std::vector<std::string> tokens = split(line, ',');

    try
    {
        // Minimal size check (12 fields for both formats)
        if (tokens.size() < 12)
        {
            std::cerr << "[parse_line] Corrupt line: " << line << std::endl;
            event.timestamp = 0;
            return event;
        }
        uint64_t time_micro = std::stoull(tokens[1]);
        uint64_t time_nano = std::stoull(tokens[6]);
        event.timestamp = time_micro * 1000 + time_nano;

        if (is_trade_file)
        {
            // TRADE FILE columns
            // [7]=Order_No_B, [8]=Order_No_S, [9]=Token, [10]=Price, [11]=Quantity
            event.type = EventType::TRADE;
            event.buy_order_id = std::stoull(tokens[7]);
            event.sell_order_id = std::stoull(tokens[8]);
            event.token = std::stoull(tokens[9]);
            event.price = std::stoi(tokens[10]);
            event.quantity = std::stoi(tokens[11]);
            event.side = Side::BUY; // Not meaningful for trade itself
            event.order_id = 0;     // No single relevant order in trade event
        }
        else
        {
            // ORDER FILE columns
            // [5]=Order_Type
            char order_type_char = tokens[5][0];
            if (order_type_char == 'N')
                event.type = EventType::NEW;
            else if (order_type_char == 'M')
                event.type = EventType::MODIFY;
            else if (order_type_char == 'X')
                event.type = EventType::CANCEL;
            else if (order_type_char == 'T')
                event.type = EventType::TRADE; // Sometimes present!
            else
            {
                std::cerr << "[parse_line] Unknown order type '" << order_type_char << "' in line: " << line << std::endl;
                event.timestamp = 0;
                return event;
            }
            event.order_id = std::stoull(tokens[7]);
            event.token = std::stoull(tokens[8]);
            event.side = (tokens[9] == "B") ? Side::BUY : Side::SELL;
            event.price = std::stoi(tokens[10]);
            event.quantity = std::stoi(tokens[11]);

            // For an in-order-file 'T' (TRADE) row, you may ALSO want to fill in buy_order_id, sell_order_id if you can.
            if (order_type_char == 'T' && tokens.size() >= 9)
            {
                // Sometimes, you may want: event.buy_order_id = event.order_id;
                // But without more info, can leave as is.
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[parse_line] parse error: " << e.what() << " for line: " << line << std::endl;
        event.timestamp = 0;
    }
    return event;
}

std::string Utils::format_timestamp_ist(uint64_t total_nanos)
{
    auto duration_ns = std::chrono::nanoseconds(total_nanos);
    using clock = std::chrono::system_clock;
    auto tp = clock::time_point(std::chrono::duration_cast<clock::duration>(duration_ns));
    tp += std::chrono::hours(5) + std::chrono::minutes(30);
    std::time_t time = clock::to_time_t(tp);
    auto since_epoch = tp.time_since_epoch();
    auto s = std::chrono::duration_cast<std::chrono::seconds>(since_epoch);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(since_epoch - s);

    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%H:%M:%S");
    ss << '.' << std::setw(6) << std::setfill('0') << us.count();

    return ss.str();
}
